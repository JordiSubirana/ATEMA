
#include <atema/atema.hpp>

#include <iostream>
#include <cstdio>
#include <cstdlib>

using namespace std;
using namespace at;



const char *vertex_shader = "#version 330 core\n" ATEMA_STRINGIFY(
		layout(location = 0) in vec3 position;
		layout(location = 1) in vec3 inf;
		out vec3 frag_color;

		void main()
		{
			frag_color = inf;
			gl_Position = vec4(position.x, position.y, position.z, 1.0);

			if (inf.x == 0 && inf.y == 0 && inf.z == 0)
				gl_PointSize = 8.0;
			else
				gl_PointSize = 1.0;
		}
);

const char *fragment_shader = "#version 330 core\n" ATEMA_STRINGIFY(
		in vec3 frag_color;
		out vec4 outcolor;

		void main()
		{
			outcolor = vec4(frag_color, 1.0);
		}
);

static string code = ATEMA_STRINGIFY(

		uniform uint time;
		buffer Pos {
				float pos_dst[];
		};
		buffer Vel {
				float vel_dst[];
		};
		buffer Inf {
				float inf_dst[];
		};

		void main() {
			uint i = gl_GlobalInvocationID.x;
			float t = time;
			float j = i;

			vec3 pos;
			vec3 vel;
			vec3 inf;

			pos.x = pos_dst[3*i+0];
			pos.y = pos_dst[3*i+1];
			pos.z = pos_dst[3*i+2];
			vel.x = vel_dst[3*i+0];
			vel.y = vel_dst[3*i+1];
			vel.z = vel_dst[3*i+2];
			inf.x = inf_dst[3*i+0];
			inf.y = inf_dst[3*i+1];
			inf.z = inf_dst[3*i+2];

			float dx = pos.x;
			float dy = pos.y;

			float d = sqrt(dx*dx + dy*dy)+0.01;
			float h = 1/(d*d);

			vel += -0.001*vec3(dx,dy,0)*h-0.0*vel;
			pos += vel;

			inf.x = d;
			inf.y = 1-d;
			inf.z = 0.5+0.5*cos(t/10.0);

			if (i == 0)
				inf *= 0;


			pos_dst[3*i+0] = pos.x;
			pos_dst[3*i+1] = pos.y;
			pos_dst[3*i+2] = pos.z;
			vel_dst[3*i+0] = vel.x;
			vel_dst[3*i+1] = vel.y;
			vel_dst[3*i+2] = vel.z;
			inf_dst[3*i+0] = inf.x;
			inf_dst[3*i+1] = inf.y;
			inf_dst[3*i+2] = inf.z;
		}
);

float rand01() {
	return (rand()%1000000)/1000000.0f;
}


int main() {

	try	{
		tic();
		srand(3543);

		const float fps=60;
		const unsigned N = 1024*512;

		Context::gl_version version;
		version.major = 4;
		version.minor = 3;

		Window window;
		window.create(512, 512, "Particle", Window::options::visible | Window::options::frame | Window::options::resizable , version);
		window.set_viewport(Rect(0, 0, window.get_width(), window.get_height()));

		Keyboard keyboard;
		keyboard.set_window(window);

		cout << "==================================================\n";
		cout << "GL_VENDOR   : " << (char const*)glGetString(GL_VENDOR) << endl;
		cout << "GL_RENDERER : " << (char const*)glGetString(GL_RENDERER) << endl;
		cout << "==================================================\n";
		cout << "  [arrows] :   navigate\n";
		cout << "==================================================\n";


		//Mesh creation : triangle position
		Vector3f pos_d[N];
		for (unsigned i=0 ; i<N ; i++) {
			pos_d[i].x = cos(float(i));
			pos_d[i].y = sin(float(i));
			//pos_d[i].x = i*cos(2*3.141592f*float(i)/N)/N;
			//pos_d[i].y = i*sin(2*3.141592f*float(i)/N)/N;
			pos_d[i].z = 0;
		}
		Mesh pos_m(Mesh::draw_mode::points, pos_d, sizeof(pos_d)/sizeof(Vector3f));
		Buffer<Vector3f> &pos = pos_m.elements;

		//Buffer creation : triangle color
		Buffer<Vector3f> vel;
		Vector3f *vel_d = vel.createVRAM_map(N);
		for (unsigned i=0 ; i<N ; i++) {
			vel_d[i].x = 0.01f*(rand01()-0.5f);
			vel_d[i].y = 0.01f*(rand01()-0.5f);
			vel_d[i].z = 0;
		}
		vel.unmap();


		//Buffer creation : triangle color
		Buffer<Vector3f> inf;
		Vector3f *inf_d = inf.createVRAM_map(N);
		for (unsigned i=0 ; i<N ; i++) {
			inf_d[i] *= 0;
		}
		inf.unmap();



		//Shader creation
		Shader shader;
		shader.create_from_memory("position", vertex_shader, fragment_shader);
		shader.set_varying("inf", inf);

		//Renderer creation
		Renderer renderer;
		renderer.set_target(&window);
		renderer.set_shader(&shader);

		// Parallel kernel creation
		Parallel<Parogl> cpt;
		cpt.add_src(code);
		cpt.build("time", "Pos", "Vel", "Inf");
		cpt.set_range(ComputeSize(N / 64), ComputeSize(64));

		// parametres controle
		unsigned frame = 1;

		cout << toc() << "s" << endl;

		bool pause = false;
		bool toggler = false;
		while (window && !keyboard.is_pressed(Keyboard::key::escape))
		{
			tic();
			window.clear();

			/*
			if (keyboard.is_pressed(Keyboard::key::left)) 		os.x -= 0.05*zoom;
			if (keyboard.is_pressed(Keyboard::key::right)) 		os.x += 0.05*zoom;
			if (keyboard.is_pressed(Keyboard::key::down)) 		os.y -= 0.05*zoom;
			if (keyboard.is_pressed(Keyboard::key::up)) 		os.y += 0.05*zoom;
			if (keyboard.is_pressed(Keyboard::key::page_down)) 	zoom *= 1.1;
			if (keyboard.is_pressed(Keyboard::key::page_up)) 	zoom *= 0.9;//*/
			if (keyboard.is_pressed(Keyboard::key::space)) {
				if (toggler)
					pause = !pause;
				toggler = false;
			}
			else {
				toggler = true;
			}

			if (!pause) {
				cpt(frame, pos, vel, inf);
				frame++;
			}
			/*
			pos_m.to_cpu();
			for (int i=0 ; i<N ; i++) {
				Vector3f &elt = pos_m.elements[i];
				cout << elt << endl;
				elt.x = (float) cos(i*1.0);
				elt.y = (float) sin(i*1.0);
				elt.z = 0;
				inf[i].x = 0;
				inf[i].y = 1;
				inf[i].z = 0.5;
			}
			pos_m.to_gpu();
			inf.to_gpu();
			return 0; //*/


			window.clear();
			renderer.draw(pos_m);


			float dt = 1/fps - toc();
			if (dt > 0.0f)
				sleep(second(dt));
			//*/

			window.update();
		}
	}
	catch (Error& e) {
		printf("error: %s", e.what());
	}

    return 0;
}



/*
 * z b.t
 * HLM
 * f?  ; repeat
 * D  suppr la fin
 *
 * :s/old/new    sur la ligne
 * :l1,l2s/old/new    sur le range
 * :%s/old/new      fichier
 * I    debut ligne + insert
 */
