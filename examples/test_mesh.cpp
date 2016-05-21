#include <atema/atema.hpp>

#include <iostream>
#include <cstdio>
#include <cmath>

using namespace std;
using namespace at;

#define STRINGIFY(A) #A

const char *vertex_shader = "#version 330 core\n" STRINGIFY(
	layout(location = 0) in vec3 pos;
	out vec3 frag_color;
	
	void main()
	{
		frag_color = vec3((pos.x+1)/2, (pos.y+1)/2, (pos.x+1+pos.y+1)/4);
		gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
		// gl_Position = vec4(pos.x, 0, 0, 1.0);
	}
);

const char *vertex_shader_colors = "#version 330 core\n" STRINGIFY(
	layout(location = 0) in vec3 pos;
	layout(location = 1)in vec3 custom_color;
	out vec3 frag_color;
	
	void main()
	{
		frag_color = custom_color;
		gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
	}
);

const char *fragment_shader = "#version 330 core\n" STRINGIFY(
	in vec3 frag_color;
	out vec4 color;
	
	void main()
	{
		// gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
		color = vec4(frag_color, 1.0);
	}
);

unsigned int gl_indices[] = {0, 1, 2};

// /*
Vector3f vertices[] =
{
	Vector3f(-0.5f, -0.5f, 0.0f),
	Vector3f(-0.5f, +0.5f, 0.0f),
	Vector3f(+0.5f, -0.5f, 0.0f)
};

Vector3f vec_colors1[] = {
   Vector3f(1.0f, 0.0f, 0.0f),
   Vector3f(0.0f, 1.0f, 0.0f),
   Vector3f(0.0f, 0.0f, 1.0f)
};

int main()
{
	cout << vertices[0].x << vertices[0].y << vertices[0].z << endl;
	cout << vertices[1].x << vertices[1].y << vertices[1].z << endl;
	cout << vertices[2].x << vertices[2].y << vertices[2].z << endl;
	
	try
	{
		Window window;
		Keyboard keyboard;
		
		Context::gl_version version;
		version.major = 3;
		version.minor = 3;
		
		window.create(640, 480, "Test", at::Window::options::visible | at::Window::options::frame, version);
		window.set_viewport(Rect(0, 0, window.get_width(), window.get_height()));
		
		window.set_clear_color(Color(0,0,1,1));
		
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
		
		Shader shader;
		shader.create_from_memory("pos", vertex_shader_colors, fragment_shader);
		cout << "Shader entry location : " << shader.get_gl_entry_location() << endl;
		
		Renderer renderer;
		renderer.set_target(&window);
		renderer.set_shader(&shader);
		
		Mesh mesh(Mesh::draw_mode::triangles, vertices, sizeof(vertices)/sizeof(Vector3f));
		// cout << "MAX GL STUFF " << GL_MAX_VERTEX_ATTRIBS << endl;
		cout << "Mesh : size " << mesh.get_size() << ", id " << mesh.get_gl_id() << endl;
		
		BufferArray<Vector3f> colors(vec_colors1, sizeof(vec_colors1)/sizeof(Vector3f));
		
		shader.set_varying("custom_color", colors);
		
		bool toggle_polygone_mode = false;
		
		int i = 0;
		while (window && !keyboard.is_pressed(Keyboard::key::escape))
		{
			// cout << i++ << endl;
			
			// window.draw(tex);
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
//*/

/*

GLfloat vertices1[] = {
   -1.0f, -1.0f, 0.0f,
   1.0f, -1.0f, 0.0f,
   0.0f,  1.0f, 0.0f
};

GLfloat vertices2[] = {
   1.0f, 1.0f, 0.0f,
   -1.0f, 1.0f, 0.0f,
   0.0f,  -1.0f, 0.0f
};

GLfloat colors1[] = {
   1.0f, 0.0f, 0.0f,
   0.0f, 1.0f, 0.0f,
   0.0f, 0.0f, 1.0f
};

int main()
{
	cout << vertices[0].x << vertices[0].y << vertices[0].z << endl;
	cout << vertices[1].x << vertices[1].y << vertices[1].z << endl;
	cout << vertices[2].x << vertices[2].y << vertices[2].z << endl;
	
	try
	{
		Window window;
		
		Context::gl_version version;
		version.major = 3;
		version.minor = 3;
		
		window.create(640, 480, "Test", at::Window::options::visible | at::Window::options::frame, version);
		
		window.make_current(true);
		
		window.set_viewport(Rect(0, 0, window.get_width(), window.get_height()));
		
		window.set_clear_color(Color(0,0,1,1));
		
		window.clear();
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, 640, 480);
		glClearColor(1, 1, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		Shader shader;
		shader.create_from_memory("pos", vertex_shader_colors, fragment_shader);
		
		//-----------------------------------------------------------------------------------------------------
		
		//-----VAO-BEGIN-----//
		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		GLuint ibo1;
		glGenBuffers(1, &ibo1);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo1);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gl_indices), gl_indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		
		GLuint vbo1;
		glGenBuffers(1, &vbo1);
		glBindBuffer(GL_ARRAY_BUFFER, vbo1);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		glBindVertexArray(0);
		//------VAO-END------//
		
		GLuint vbo2;
		glGenBuffers(1, &vbo2);
		glBindBuffer(GL_ARRAY_BUFFER, vbo2);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		GLuint vbo_colors;
		glGenBuffers(1, &vbo_colors);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_colors);
		glBufferData(GL_ARRAY_BUFFER, sizeof(colors1), colors1, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		//---
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo2);
		glBindVertexArray(0);
		//---
		
		shader.bind();
		
		// GLint mesh_loc = 0;
		GLint mesh_loc = shader.get_gl_entry_location();
		
		cout << "Mesh loc : " << mesh_loc << endl;
		
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_colors);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindVertexArray(0);
		
		// glBindVertexArray(vao);
		// glDisableVertexAttribArray(1);
		// glBindVertexArray(0);
		
		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo1);
		glBindBuffer(GL_ARRAY_BUFFER, vbo1);
		glEnableVertexAttribArray(mesh_loc);
		glVertexAttribPointer(mesh_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
		
		cout << "GL ERROR " << glGetError() << endl;
		
		glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);
		// glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
		// glDrawArrays(GL_POINTS, 0, 1); // Starting from vertex 0; 3 vertices total -> 1 triangle
		// glDisableVertexAttribArray(mesh_loc);
		
		shader.unbind();
		
		cout << "GL ERROR " << glGetError() << endl;
		
		window.update();
		
		int i = 0;
		while (window)
		{
			// cout << i++ << endl;
			
			// window.make_current(true);
			
			// window.clear();
			
			// shader.bind();
			// mesh.draw(shader);
			// shader.unbind();
			
			// window.update();
			
			// glfwSwapBuffers(window.get_glfw_window());
			
			glfwPollEvents();
		}
	}
	catch (Error& e)
	{
		printf("ERROR\n");
		printf("%s", e.what());
	}
	
	return 0;
}
//*/