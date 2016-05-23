#include <atema/atema.hpp>

#include <iostream>

using namespace std;
using namespace at;

int main()
{
	try
	{
		//Setup OpenGL version
		Context::gl_version version;
		version.major = 3;
		version.minor = 3;
		
		//Window creation
		Window window;
		window.create(512, 512, "Test", at::Window::options::visible | at::Window::options::frame, version);
		window.set_viewport(Rect(0, 0, window.get_width(), window.get_height()));		
		window.set_clear_color(Color(0.3,0.3,0.3,1.0));
		
		//Keyboard creation
		Keyboard keyboard;
		keyboard.set_window(window);
		
		//Texture creation
		Texture tex;
		tex.create("images/lena.jpg");
		tex.set_viewport(Rect(0, 0, tex.get_width(), tex.get_height()));
		tex.to_cpu(); //get clear color to all pixels in cpu
		for (size_t i = 0; i < std::min(tex.get_width(), tex.get_height()); i++)
		{
			Color c = tex[i+i*tex.get_width()];
			
			tex[i+i*tex.get_width()] = Color(1.0f, 0.0f, 0.0f, 1.0f);
		}
		tex.to_gpu(); //upload white line to gpu
		
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


