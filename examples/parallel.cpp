
#include <atema/atema.hpp>

#include <iostream>
#include <cstdio>

using namespace std;
using namespace at;

#define PI 3.14159265359f


#define STRINGIFY(A) #A



int main() {

	try
	{

		Window window;
		Keyboard keyboard;

		Context::gl_version version;
		version.major = 4;
		version.minor = 5;

		window.create(512, 512, "Test", Window::options::visible | Window::options::frame | Window::options::resizable , version);
		window.set_viewport(Rect(0, 0, window.get_width(), window.get_height()));

		keyboard.set_window(window);

		//Texture created after window opening (context stuff...)
		Texture tex;
		tex.create(512, 512);
		tex.set_viewport(Rect(0, 0, tex.get_width(), tex.get_height()));

		cout << "GL_VENDOR " << (char const*)glGetString(GL_VENDOR) << endl;
		cout << "GL_RENDERER " << (char const*)glGetString(GL_RENDERER) << endl;
		cout << "==================================================\n";


		string code = STRINGIFY(
				//buffer Pos { float Position[]; };
				uniform writeonly image2D destTex;
				uniform uint Reso;
				uniform uint time;
				uniform vec2 os;


				void main() {
					const float PI = 3.1415926f;
					ivec2 p = ivec2(gl_GlobalInvocationID.xy);
					vec4 data = vec4(0,0,0,1);
					float i = 1;

					float cx = (p.x-256.0f)/180.0f-0.6-os.x;
					float cy = (p.y-256.0f)/180.0f-os.y;
					float x = cx;
					float y = cy;
					float tmp;

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
							data.x = (X != 0) ? data.x : 0.0f;
							data.y = (X != 0) ? data.y : 0.0f;
							data.z = (X != 0) ? data.z : 0.0f;

							imageStore(destTex, p, data);
							return ;
						}
					}
					data.x = 0.25*(2+cos(cx*50)+sin(cy*50));
					data.y = (p.x+p.y)/1024.0f;
					data.z = 1-sqrt(pow((p.x-256.)/256., 2) + pow((p.y-256.)/256., 2));

					uint X = 511-p.x;
					uint Y = 511-p.y;
					X &= Y;
					data.x = (X != 0) ? data.x : 0.0f;
					data.y = (X != 0) ? data.y : 0.0f;
					data.z = (X != 0) ? data.z : 0.0f;

					imageStore(destTex, p, data);
				}
		);

		Parallel<Parogl> cpt;
		cpt.add_src(code);
		cpt.build("destTex", "Reso", "time", "os");
		cpt.set_range(ComputeSize(512 / 16, 512 / 16), ComputeSize(16, 16));

		Vector2f os;
		os.x = 0.5;
		os.y = 0.5;

		unsigned frame = 1;
		while (window && !keyboard.is_pressed(Keyboard::key::escape))
		{
			window.clear();

			cpt(tex, (unsigned)18, frame, os);

			window.draw(tex);

			window.update();

			frame++;
		}
	}
	catch (Error& e)
	{
		printf("ERROR\n");
		printf("%s", e.what());
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


