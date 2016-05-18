
#include <atema/atema.hpp>

#include <iostream>
#include <cstdio>

using namespace std;
using namespace at;

#define PI 3.14159265359f


#define STRINGIFY(A) #A

static string code = STRINGIFY(
		uniform writeonly image2D destTex;
		uniform uint Reso;
		uniform uint time;
		uniform vec2 os;
		uniform float zoom;
		buffer Pos {
				float pos[];
		};

		void main() {
			const float PI = 3.1415926f;
			ivec2 p = ivec2(gl_GlobalInvocationID.xy);
			vec4 data = vec4(0,0,0,1);
			float i = 1;

			float cx = ((p.x-256.0f)/180.0f-0.6);
			float cy = ((p.y-256.0f)/180.0f);
			cx = cx*zoom+os.x;
			cy = cy*zoom+os.y;

			float x = cx;
			float y = cy;
			float tmp;

			if (zoom == 0) {
				i = pos[0];
			}

			for ( ; i<Reso ; i+=1) {
				tmp = x*x - y*y;
				y = 2*x*y + cy;
				x = tmp + cx;
				if (x*x + y*y > 4+1.8*cos(time/1200.0f)) {
					//data.xyz = vec3(i/Reso,(mod(i+Reso/3, Reso))/Reso,(mod(i+2*Reso/3, Reso))/Reso);

					data.x = 0.5f*(1.0f+cos(2.0f*PI*(i/Reso+1.0f/3.0f)));
					data.y = 0.5f*(1.0f+cos(2.0f*PI*(i/Reso+2.0f/3.0f)));
					data.z = 0.5f*(1.0f+cos(2.0f*PI*(i/Reso+0.0f/3.0f)));

					uint X = p.x;
					uint Y = p.y;
					X &= Y;
					data.x *= (X != 0) ? 1 : 1-zoom;
					data.y *= (X != 0) ? 1 : 1-zoom;
					data.z *= (X != 0) ? 1 : 1-zoom;

					imageStore(destTex, p, data);
					return ;
				}
			}
			data.x = 0.25*(2+cos(cx*50)+sin(cy*50));
			data.y = (p.x+p.y)/1024.0f;
			data.z = 1-sqrt(pow((p.x-256.)/256., 2) + pow((p.y-256.)/256., 2));

			imageStore(destTex, p, data);
		}
);


int main() {

	try	{
		Context::gl_version version;
		version.major = 4;
		version.minor = 3;

		Window window;
		window.create(512, 512, "Test", Window::options::visible | Window::options::frame | Window::options::resizable , version);
		window.set_viewport(Rect(0, 0, window.get_width(), window.get_height()));

		Keyboard keyboard;
		keyboard.set_window(window);

		Texture tex;
		tex.create(512, 512);
		tex.set_viewport(Rect(0, 0, tex.get_width(), tex.get_height()));

		cout << "==================================================\n";
		cout << "GL_VENDOR   : " << (char const*)glGetString(GL_VENDOR) << endl;
		cout << "GL_RENDERER : " << (char const*)glGetString(GL_RENDERER) << endl;
		cout << "==================================================\n";
		cout << "  [arrows] : navigate\n";
		cout << "  [pageUp] : zoom in\n";
		cout << "  [pageDown] : zoom out\n";
		cout << "  [space] : reset\n";
		cout << "  [escape] : exit\n";
		cout << "==================================================\n";


		Buffer buffer(0);

		Parallel<Parogl> cpt;
		cpt.add_src(code);
		cpt.build("destTex", "Reso", "time", "os", "zoom", "Pos");
		cpt.set_range(ComputeSize(512 / 16, 512 / 16), ComputeSize(16, 16));

		Vector2f os;
		os *= 0;
		float zoom = 1;

		unsigned frame = 1;
		while (window && !keyboard.is_pressed(Keyboard::key::escape))
		{
			window.clear();

			if (keyboard.is_pressed(Keyboard::key::left)) 		os.x -= 0.01*zoom;
			if (keyboard.is_pressed(Keyboard::key::right)) 		os.x += 0.01*zoom;
			if (keyboard.is_pressed(Keyboard::key::down)) 		os.y -= 0.01*zoom;
			if (keyboard.is_pressed(Keyboard::key::up)) 		os.y += 0.01*zoom;
			if (keyboard.is_pressed(Keyboard::key::page_down)) 	zoom *= 1.01;
			if (keyboard.is_pressed(Keyboard::key::page_up)) 	zoom *= 0.99;
			if (keyboard.is_pressed(Keyboard::key::space)) {
				zoom = 1;
				os *= 0;
			}

			cpt(tex, (unsigned)100, frame, os, zoom, buffer);

			window.draw(tex);
			window.update();

			frame++;
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
