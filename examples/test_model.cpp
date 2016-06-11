#include <atema/atema.hpp>

#include <iostream>
#include <cmath>

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
		shader.create_from_file("position", "shaders/basic_mesh_mvp.vert", "shaders/basic_mesh_mvp.frag");

		//Renderer creation
		Renderer renderer;
		renderer.set_target(&window);
		renderer.set_shader(&shader);
		
		//MeshElement creation
		// MeshElement mesh = Shape::create_sphere_mesh(0.8f, 35, 35);
		Model model;
		model.create("3d/Spider-Man_Symbiote.obj");
		
		//Transform matrices
		Matrix4f camera, perspective, transform;
		camera = Transform::look_at(Vector3f(5.0f, 2.0f, 5.0f), Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f));
		perspective = Transform::perspective(static_cast<float>(ATEMA_DEG_TO_RAD(70.0f)), 640.0f/480.0f, 0.1f, 1000.0f);
		transform.identity();
		
		// camera.identity();
		// perspective.identity();
		// transform = Transform::scale(Vector3f(0.5f, 0.5f, 0.5f));
		
		shader.set_uniform("mat_M", transform); //Model
		shader.set_uniform("mat_V", camera); //View
		shader.set_uniform("mat_P", perspective); //Projection
		
		bool toggle_polygone_mode = false;
		float angle = 0.0f;
		
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
			
			angle += 0.001f;
			
			if (angle >= 2.0f*ATEMA_PI)
				angle = 0.0f;
			
			camera = Transform::look_at(Vector3f(2.5f*std::cos(angle), 2.0f, 2.5f*std::sin(angle)), Vector3f(0.0f, 2.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f));
			shader.set_uniform("mat_V", camera); //View
			
			//Clear the window, then draw the colored triangle
			window.clear();
			
			renderer.draw(model.get_mesh());
			
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