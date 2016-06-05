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
		shader.create_from_file("position", "shaders/mesh_perlin_texture.vert", "shaders/mesh_perlin_texture.frag");

		//Renderer creation
		Renderer renderer;
		renderer.set_target(&window);
		renderer.set_shader(&shader);
		
		//Mesh creation : grid with 30*30 cells
		Mesh mesh = Shape::create_grid_mesh(30, 30, Vector3f(-0.8f, 0.8f, 0.0f), Vector3f(-0.8f, -0.8f, 0.0f), Vector3f(0.8f, 0.8f, 0.0f));
		
		//Grid texture coordinates
		Buffer<Vector2f> tex_coords = Shape::create_grid_texture_coordinates(30, 30);
		
		//Texture creation
		Texture texture;
		texture.create("images/lena.png");
		
		//Update shader internal stuff
		shader.set_uniform("custom_texture", texture);
		shader.set_varying("tex_coords", tex_coords);
		
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
			
			shader.set_uniform("time", Timer::elapsed_from_start().s());
			
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