#include <atema/atema.hpp>

#include <iostream>

using namespace std;
using namespace at;

//----------
//Main function
//----------
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
		window.create(640, 480, "Test", at::Window::options::visible | at::Window::options::frame, version);
		window.set_viewport(Rect(0, 0, window.get_width(), window.get_height()));		
		window.set_clear_color(Color(0.3,0.3,0.3,1.0));
		
		//Keyboard creation
		Keyboard keyboard;
		keyboard.set_window(window);
		
		//Shader creation
		Shader shader;
		shader.create_from_file("position", "shaders/unicolor_mesh.vert", "shaders/unicolor_mesh.frag");

		//Renderer creation
		Renderer renderer;
		renderer.set_target(&window);
		renderer.set_shader(&shader);
		
		//Mesh creation
		Mesh mesh = Shape::create_sphere_mesh(0.8f, 35, 35);
		
		//Shader Variable
		shader.set_uniform("color", Color(1.0f, 1.0f, 0.0f, 1.0f));
		
		bool toggle_polygone_mode = false;
		
		//Main loop
		while (window && !keyboard.is_pressed(Keyboard::key::escape))
		{
			tic();
			
			//Toggle between surfaces & lines when pressing spacebar
			if (keyboard.is_pressed(Keyboard::key::space))
			{
				if (!toggle_polygone_mode)
				{
					if (renderer.get_polygon_mode() == Renderer::polygon_mode::lines)
						renderer.set_polygon_mode(Renderer::polygon_mode::surfaces);
					else
						renderer.set_polygon_mode(Renderer::polygon_mode::lines);
					toggle_polygone_mode = true;
				}
			}
			else if (toggle_polygone_mode)
			{
				toggle_polygone_mode = false;
			}
			
			//Clear the window, then draw the colored triangle
			window.clear();
			
			renderer.draw(mesh);
			
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