
#include <atema/atema.hpp>

#include <iostream>
#include <cstdio>

using namespace std;
using namespace at;


#define STRINGIFY(A) #A




int main() {

	try
	{

		Window window;
		Keyboard keyboard;

		Context::gl_version version;
		version.major = 4;
		version.minor = 5;

		window.create(512, 512, "Test", at::Window::options::visible | at::Window::options::frame | Window::options::resizable , version);
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


				void main() {
					ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
					vec4 data = vec4(Reso,0,0,1);
					float i = 1;

					float cx = (storePos.x-256.0f)/180.0f-0.6;
					float cy = (storePos.y-256.0f)/180.0f;
					float x = cx;
					float y = cy;
					float tmp;

					for ( ; i<Reso ; i+=1) {
						tmp = x*x - y*y;
						y = 2*x*y + cy;
						x = tmp + cx;
						if (x*x + y*y > 6+6*cos(time/1200.0f)) {
							data.xyz = vec3(i/Reso,(mod(i+Reso/3, Reso))/Reso,(mod(i+2*Reso/3, Reso))/Reso);
							imageStore(destTex, storePos, data);
							return ;
						}
					}
					data.y = 1;
					//data.z = 2*(1+cos(time));

					imageStore(destTex, storePos, data);
				}
		);

		Parallel<Parogl> cpt;
		cpt.add_src(code);
		cpt.build("destTex", "Reso", "time");
		cpt.setRange(ComputeSize(512/16, 512/16), ComputeSize(16,16));



		unsigned frame = 1;
		while (window && !keyboard.is_pressed(Keyboard::key::escape))
		{
			window.clear();

			cpt(tex, (unsigned)18, frame);

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


