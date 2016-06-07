#include <atema/atema.hpp>

#include <iostream>

using namespace std;
using namespace at;

#define POINTS_X 1024
#define POINTS_Y 1024

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
		window.create(512, 512, "Test", at::Window::options::visible | at::Window::options::frame, version);
		window.set_viewport(Rect(0, 0, window.get_width(), window.get_height()));		
		window.set_clear_color(Color(0.3,0.3,0.3,1.0));
		
		//Keyboard creation
		Keyboard keyboard;
		keyboard.set_window(window);
		
		//Shader creation
		Shader shader;
		shader.create_from_file("position", "shaders/colored_mesh.vert", "shaders/colored_mesh.frag");

		//Renderer creation
		Renderer renderer;
		renderer.set_target(&window);
		renderer.set_shader(&shader);
		
		//Mesh creation : grid of (cell_x * cell_y) cells --> (cell_x + 1) * (cell_y + 1) points
		// Mesh mesh = Shape::create_grid_mesh(POINTS_X, POINTS_Y, Vector3f(-0.9f, 0.9f, 0.0f), Vector3f(-0.9f, -0.9f, 0.0f), Vector3f(0.9f, 0.9f, 0.0f));
		Mesh mesh = Shape::create_grid_mesh(POINTS_X-1, POINTS_Y-1);
		
		//Texture creation
		Texture texture;
		bool texture_valid = true;
		
		try
		{
			texture.create("images/big-lena.png");
		}
		catch (...)
		{
			texture_valid = false;
		}
		
		//Color buffer
		std::vector<Color> colors_vector;
		Buffer<Color> colors;
		
		colors_vector.resize(mesh.elements.get_size());
		
		if (texture_valid)
		{
			size_t dx = texture.get_width() / POINTS_X;
			size_t dy = texture.get_height() / POINTS_Y;
			
			for (size_t y = 0; y < POINTS_Y; y++)
			{
				for (size_t x = 0; x < POINTS_X; x++)
				{
					size_t xi = x*dx;
					size_t yi = y*dy;
					
					colors_vector[x + y*POINTS_X] = texture[xi + yi*texture.get_width()];
				}
			}
		}
		else
		{
			//Put random colors
			for (size_t i = 0; i < colors_vector.size(); i++)
			{
				float i_norm = static_cast<float>(i)/static_cast<float>(colors_vector.size());
				
				colors_vector[i] = Color(i_norm, 1.0f-i_norm, (i_norm/2.0f)+0.25f, 1.0f);
			}
		}
		
		colors.create(colors_vector.data(), colors_vector.size());
		
		shader.set_varying("color", colors);
		
		bool toggle_polygone_mode = false;
		
		//Default : draw points
		renderer.set_polygon_mode(Renderer::polygon_mode::points);
		
		//Main loop
		while (window && !keyboard.is_pressed(Keyboard::key::escape))
		{
			tic();
			
			//Toggle between surfaces & points when pressing spacebar
			if (keyboard.is_pressed(Keyboard::key::space))
			{
				if (!toggle_polygone_mode)
				{
					if (renderer.get_polygon_mode() == Renderer::polygon_mode::surfaces)
						renderer.set_polygon_mode(Renderer::polygon_mode::points);
					else
						renderer.set_polygon_mode(Renderer::polygon_mode::surfaces);
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
		printf("%s\n", e.what());
	}
	
	return 0;
}