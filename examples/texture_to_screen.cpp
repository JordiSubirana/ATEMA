#include <atema/atema.hpp>

#include <iostream>
#include <cstdio>
#include <cmath>

using namespace std;
using namespace at;

int main()
{
	try
	{
		Window window;
		Keyboard keyboard;
		
		Context::gl_version version;
		version.major = 3;
		version.minor = 3;
		
		window.create(640, 480, "Test", at::Window::options::visible | at::Window::options::frame, version);
		window.set_viewport(Rect(0, 0, window.get_width(), window.get_height()));
		
		keyboard.set_window(window);
		
		//Texture created after window opening (context stuff...)
		Texture tex;
		tex.create(640, 480);
		tex.set_viewport(Rect(0, 0, tex.get_width(), tex.get_height()));
		
		//get opengl texture id : tex.get_gl_id();
		
		tex.set_clear_color(Color(0.2f,0,0,1));
		tex.clear();
		tex.to_cpu(); //get clear color to all pixels in cpu
		for (size_t i = 0; i < std::min(tex.get_width(), tex.get_height()); i++)
		{
			Color c = tex[i+i*tex.get_width()];
			
			tex[i+i*tex.get_width()] = Color(1.0f, 1.0f, 1.0f, 1.0f);
		}
		
		tex.to_gpu(); //upload white line to gpu
		
		window.set_clear_color(Color(0,0,1,1));
		
		while (window && !keyboard.is_pressed(Keyboard::key::escape))
		{
			window.clear();

			window.blit(tex);
			
			window.update();
		}
	}
	catch (Error& e)
	{
		printf("ERROR\n");
		printf("%s", e.what());
	}
	
	return 0;
}


