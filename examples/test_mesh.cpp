#include <atema/atema.hpp>

#include <iostream>

using namespace std;
using namespace at;

//----------
//Shaders
//----------
const char *vertex_shader = "#version 330 core\n" ATEMA_STRINGIFY(
	layout(location = 0) in vec3 position;
	layout(location = 1) in vec3 color;
	out vec3 frag_color;
	
	void main()
	{
		frag_color = color;
		gl_Position = vec4(position.x, position.y, position.z, 1.0);
	}
);

const char *fragment_shader = "#version 330 core\n" ATEMA_STRINGIFY(
	in vec3 frag_color;
	
	void main()
	{
		gl_FragColor = vec4(frag_color, 1.0);
	}
);

//----------
//Data (triangle position + color + indices)
//----------
Vector3f vertices_data[] =
{
	Vector3f(-0.8f, -0.8f, 0.0f),
	Vector3f(-0.8f, +0.8f, 0.0f),
	Vector3f(+0.8f, -0.8f, 0.0f)
};

Vector3f colors_data[] = {
   Vector3f(1.0f, 0.0f, 0.0f),
   Vector3f(0.0f, 1.0f, 0.0f),
   Vector3f(0.0f, 0.0f, 1.0f)
};

unsigned int indices_data[] = {0, 1, 2};

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
		shader.create_from_memory("position", vertex_shader, fragment_shader);
		
		//Renderer creation
		Renderer renderer;
		renderer.set_target(&window);
		renderer.set_shader(&shader);
		
		//Mesh creation : triangle position
		Mesh mesh(Mesh::draw_mode::triangles, vertices_data, sizeof(vertices_data)/sizeof(Vector3f));
		
		//BufferArray creation : triangle color
		BufferArray<Vector3f> colors(colors_data, sizeof(colors_data)/sizeof(Vector3f));
		
		//Update shader internal stuff
		shader.set_varying("color", colors);
		
		bool toggle_polygone_mode = false;
		
		//Main loop
		while (window && !keyboard.is_pressed(Keyboard::key::escape))
		{
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