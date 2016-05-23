
#include <atema/atema.hpp>

#include <iostream>
#include <cstdio>
#include <chrono>

using namespace std;
using namespace at;




static string code = ATEMA_STRINGIFY(
		// soit c'est writeonly, soit on precise le format...
	 	image2Dwr(rgba32f) destTex;
		uniform uint Reso;
		uniform uint time;
		uniform dvec2 os;
		uniform dvec2 c;
		uniform double zoom;

		void main() {
			const float PI = 3.1415926f;
			ivec2 p = ivec2(gl_GlobalInvocationID.xy);
			vec4 data = vec4(0,0,0,0);
			float i = 0;

			double x = ((p.x-256.0f)/256.0f);
			double y = ((p.y-256.0f)/256.0f);
			double tmp;
			x = x*zoom+os.x;
			y = y*zoom+os.y;


			if (zoom == 0) {
				i = pos[0];
				i *= time;
			}

			for ( ; i<Reso ; i+=1) {
				tmp = x*x - y*y;
				y = 2*x*y + c.y;
				x = tmp + c.x;
				if (x*x + y*y > 4) {
					float k = i/Reso;
					data.x = 0.5f*(1.0f+cos(2.0f*PI*(k+1.0f/3.0f)));
					data.y = 0.5f*(1.0f+cos(2.0f*PI*(2*k+2.0f/3.0f)));
					data.z = 0.5f*(1.0f+cos(2.0f*PI*(3*k+0.0f/3.0f)));

					imageStore(destTex, p, data);
					return ;
				}
			}
			data.x = ((511-p.x)/512.0f)*((511-p.x)/512.0f);
			data.y = (p.x+p.y)/1024.0;
			data.z = 1-sqrt(pow((p.x-256.)/256., 2) + pow((p.y-256.)/256., 2));

			imageStore(destTex, p, data);
		}
);


static string code2 = ATEMA_STRINGIFY(
		// soit c'est writeonly, soit on precise le format...
	 	image2Dwr(rgba32f) destTex;
		uniform uint Reso;
		uniform uint time;
		uniform vec2 os;
		uniform vec2 c;
		uniform float zoom;
		buffer Pos {
				float pos[];
		};

		void main() {
			const float PI = 3.1415926f;
			ivec2 p = ivec2(gl_GlobalInvocationID.xy);
			vec4 data = vec4(0,0,0,0);
			float i = 0;

			float x = ((p.x-256.0f)/256.0f);
			float y = ((p.y-256.0f)/256.0f);
			x = x*zoom+os.x;
			y = y*zoom+os.y;

			float lnr;
			float a;
			float A;
			float B;
			float den;

			for ( ; i<Reso ; i+=1) {

				a = atan(y, x);
				lnr = 0.5f*log(x*x+y*y);
				den = lnr*lnr+a*a;
				A = ((x+1)*x - y*y)/den;
				B = ((2*x+1)*y)/den;

				x = lnr*A+a*B + c.x;
				y = lnr*B-a*A + c.y;

				if (x*x + y*y > 4) {
					float k = i/Reso;
					data.x = 0.5f*(1.0f+cos(2.0f*PI*(k+1.0f/3.0f)));
					data.y = 0.5f*(1.0f+cos(2.0f*PI*(2*k+2.0f/3.0f)));
					data.z = 0.5f*(1.0f+cos(2.0f*PI*(3*k+0.0f/3.0f)));

					imageStore(destTex, p, data);
					return ;
				}
			}
			data.x = ((511-p.x)/512.0f)*((511-p.x)/512.0f);
			data.y = pos[0] + 0.01*cos(time); //(p.x+p.y)/1024.0;
			data.z = 1-sqrt(pow((p.x-256.)/256., 2) + pow((p.y-256.)/256., 2));

			imageStore(destTex, p, data);
		}
);

int main() {

	try	{
		tic();

		const float fps=12;

		Context::gl_version version;
		version.major = 4;
		version.minor = 3;

		Window window;
		window.create(512, 512, "Julia set 0.285+i0.01 - regulate at 10fps", Window::options::visible | Window::options::frame | Window::options::resizable , version);
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
		cout << "  [arrows] :   navigate\n";
		cout << "  [pageUp] :   zoom in\n";
		cout << "  [pageDown] : zoom out\n";
		cout << "  [space] :    reset\n";
		cout << "  [escape] :   exit\n";
		cout << "==================================================\n";



		Parallel<Parogl> cpt;
		cpt.add_src(code);
		cpt.build("destTex", "Reso", "time", "os", "zoom", "c");
		cpt.set_range(ComputeSize(512 / 16, 512 / 16), ComputeSize(16, 16));

		Vector2d c;
		c.x = 0.285;
		c.y = 0.01;
		Vector2d os;
		os *= 0;
		double zoom = 1.5;

		unsigned frame = 1;

		cout << toc() << "s" << endl;

		while (window && !keyboard.is_pressed(Keyboard::key::escape))
		{
			tic();
			window.clear();

			if (keyboard.is_pressed(Keyboard::key::left)) 		os.x -= 0.05*zoom;
			if (keyboard.is_pressed(Keyboard::key::right)) 		os.x += 0.05*zoom;
			if (keyboard.is_pressed(Keyboard::key::down)) 		os.y -= 0.05*zoom;
			if (keyboard.is_pressed(Keyboard::key::up)) 		os.y += 0.05*zoom;
			if (keyboard.is_pressed(Keyboard::key::page_down)) 	zoom *= 1.1;
			if (keyboard.is_pressed(Keyboard::key::page_up)) 	zoom *= 0.9;
			if (keyboard.is_pressed(Keyboard::key::space)) {
				os *= 0;
				zoom = 1.5;
			}

			cpt(tex, (unsigned)300, frame, os, zoom, c);

			window.blit(tex);

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
