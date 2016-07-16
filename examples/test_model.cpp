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
		window.create(1024, 720, "Test", at::Window::options::visible | at::Window::options::frame, version);
		window.set_viewport(Rect(0, 0, window.get_width(), window.get_height()));		
		window.set_clear_color(Color(0.3,0.3,0.3,1.0));
		
		//Keyboard creation
		Keyboard keyboard;
		keyboard.set_window(window);
		
		//Shader creation
		Shader shader;
		shader.create_from_file("model", "shaders/mesh_material_mvp.vert", "shaders/mesh_material_mvp.frag");

		//Renderer creation
		Renderer renderer;
		renderer.set_target(&window);
		renderer.set_shader(&shader);
		
		//Mesh creation
		Model model;
		model.create("3d/Spidey.obj");
		
		//Light
		PointLight light01;
		light01.position = Vector3f(30.0f, -10.0f, 0.0f);
		light01.color = Color(0.5f, 0.5f, 0.5f, 1.0f);
		light01.radius = 75.0f;
		light01.intensity = 0.02f;
		
		//Transform matrices
		Matrix4f camera, perspective, transform;
		camera = Transform::look_at(Vector3f(5.0f, 2.0f, 5.0f), Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f)); //the camera will change later, those values aren't important
		perspective = Transform::perspective(static_cast<float>(ATEMA_DEG_TO_RAD(70.0f)), 1024.0f/720.0f, 0.1f, 1000.0f);
		transform.identity();
		
		shader.set_uniform("mat_M", transform); //Model
		shader.set_uniform("mat_V", camera); //View
		shader.set_uniform("mat_P", perspective); //Projection
		shader.set_uniform("light01", light01); //Light
		
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
			
			angle += 0.0025f;
			
			if (angle >= 2.0f*ATEMA_PI)
				angle = 0.0f;
			
			Vector3f camera_pos = Vector3f(1.2f*std::cos(angle), 2.0f, 1.2f*std::sin(angle));
			
			camera = Transform::look_at(camera_pos, Vector3f(0.0f, 1.5f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f));
			shader.set_uniform("mat_V", camera); //View
			
			shader.set_uniform("camera_pos", camera_pos);
			
			//Clear the window, then draw the object
			window.clear();
			
			renderer.draw(model);
			
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