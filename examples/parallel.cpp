
#include <atema/atema.hpp>

#include <iostream>
#include <cstdio>
#include <chrono>

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
			gl_PointSize = 3.0;
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
		buffer Inf {
				float inf_dst[];
		};

		void main() {
			uint i = gl_GlobalInvocationID.x;
			float t = time;
			float j = i;

			vec3 pos;
			vec3 inf;

			pos.x = pos_dst[3*i+0];
			pos.y = pos_dst[3*i+1];
			pos.z = pos_dst[3*i+2];
			inf.x = inf_dst[3*i+0];
			inf.y = inf_dst[3*i+1];
			inf.z = inf_dst[3*i+2];


			pos.x = cos(j);
			pos.y = sin(j);
			pos.z = 0;

			inf.x = 1;
			inf.y = 0;
			inf.z = 0.5+0.5*cos(t);

			pos_dst[3*i+0] = pos.x;
			pos_dst[3*i+1] = pos.y;
			pos_dst[3*i+2] = pos.z;
			inf_dst[3*i+0] = inf.x;
			inf_dst[3*i+1] = inf.y;
			inf_dst[3*i+2] = inf.z;
		}
);


int main() {

	try	{
		tic();

		const float fps=12;
		const unsigned N = 1024;

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
		Mesh pos_m(Mesh::draw_mode::points, pos_d, sizeof(pos_d)/sizeof(Vector3f));
		Buffer<Vector3f> &pos = pos_m.elements;

		//Buffer creation : triangle color
		Vector3f inf_d[N];
		Buffer<Vector3f> inf(inf_d, sizeof(inf_d) / sizeof(Vector3f));

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
		cpt.build("time", "Pos", "Inf");
		cpt.set_range(ComputeSize(N / 64), ComputeSize(64));

		// parametres controle
		unsigned frame = 1;

		cout << toc() << "s" << endl;

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
			if (keyboard.is_pressed(Keyboard::key::page_up)) 	zoom *= 0.9;
			if (keyboard.is_pressed(Keyboard::key::space)) {
				os *= 0;
				zoom = 1.5;
			}//*/

			cpt(frame, pos, inf);

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

			glFinish();

			window.clear();
			renderer.draw(pos_m);

			frame++;

			sleep( msecond(10) );

			float dt = 1/fps - toc();
			if (dt > 0.0f)
				sleep(second(dt));

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
