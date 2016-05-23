

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
		uniform double zoom;

		void main() {
			const float PI = 3.1415926f;
			ivec2 p = ivec2(gl_GlobalInvocationID.xy);
			vec4 data = vec4(0,0,0,0);
			float i = 1;

			double cx = ((p.x-256.0f)/256.0f);
			double cy = ((p.y-256.0f)/256.0f);
			cx = cx*zoom+os.x;
			cy = cy*zoom+os.y;

			double x = cx;
			double y = cy;
			double tmp;

			if (zoom == 0) {
				i *= time;
			}

			for ( ; i<Reso ; i+=1) {
				tmp = x*x - y*y;
				y = 2*x*y + cy;
				x = tmp + cx;
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


int main() {

	try	{
		tic();

		const float fps=12;

		Context::gl_version version;
		version.major = 4;
		version.minor = 3;

		Window window;
		window.create(512, 512, "MandleBrot explorer - regulate at 10fps", Window::options::visible | Window::options::frame | Window::options::resizable , version);
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
		cpt.build("destTex", "Reso", "time", "os", "zoom");
		cpt.set_range(ComputeSize(512 / 16, 512 / 16), ComputeSize(16, 16));

		Vector2d os;
		os *= 0;
		os.x = -0.7;
		double zoom = 1.5;

		unsigned frame = 1;

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
				os.x = -0.7;
				zoom = 1.5;
			}

			cpt(tex, (unsigned)300, frame, os, zoom);

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