
#include <atema/atema.hpp>

#include <iostream>
#include <cstdio>
#include <cstdlib>

using namespace std;
using namespace at;


#define POINTS_X 128
#define POINTS_Y 128

const char *vertex_shader = "#version 330 core\n" ATEMA_STRINGIFY(
		layout(location = 0) in vec3 position;
		layout(location = 1) in vec3 color;
		out vec3 frag_color;

		void main() {
			frag_color = color;
			gl_Position = vec4(position.x, position.y, position.z, 4.0)/4;
			gl_PointSize = 2.0;
		}
);

const char *fragment_shader = "#version 330 core\n" ATEMA_STRINGIFY(
		in vec3 frag_color;
		out vec4 outcolor;

		void main()
		{
			outcolor = vec4(frag_color, 1.0);
		}
);

static string code = ATEMA_STRINGIFY(
		struct vec3_fix {
			float x;
			float y;
			float z;
		};
		struct Color {
			float r;
			float g;
			float b;
			float a;
		};


		uniform float t;
		buffer Mesh {
				vec3_fix mesh[];
		};
		buffer Colors {
				Color c[];
		};
		uniform uint cmd;

		void main() {
			uint id = gl_GlobalInvocationID.x;
			c[id].r *= 1;

			float k = 0.5+0.5*cos(3.141592+t);
			float r = mesh[id].x;
			float i = mesh[id].y;
			float m = sqrt(r*r + i*i);
			float a = atan(i, r);

			if (cmd == 0) { // power
				k = k+1;
				m = pow(m, k);
				a = a*k;
				r = m*cos(a);
				i = m*sin(a);
			}
			else if (cmd == 1) { // cos
				float cr = cos(r)*cosh(i);
				float ci = -sin(r)*sinh(i);
				r = (k)*cr + (1-k)*r;
				i = (k)*ci + (1-k)*i;
			}
			else if (cmd == 2) { // sin
				float cr = sin(r)*cosh(i);
				float ci = cos(r)*sinh(i);
				r = (k)*cr + (1-k)*r;
				i = (k)*ci + (1-k)*i;
			}
			else if (cmd == 3) { // cosh
				float cr = cosh(r)*cos(i);
				float ci = sinh(r)*sin(i);
				r = (k)*cr + (1-k)*r;
				i = (k)*ci + (1-k)*i;
			}
			else if (cmd == 4) { // sinh
				float cr = sinh(r)*cos(i);
				float ci = cosh(r)*sin(i);
				r = (k)*cr + (1-k)*r;
				i = (k)*ci + (1-k)*i;
			}
			else if (cmd == 5) { // ln
				float cr = r*cos(i);
				float ci = r*sin(i);
				r = (k)*cr + (1-k)*r;
				i = (k)*ci + (1-k)*i;
			}
			else if (cmd == 6) { // exp
				float cr = log(m);
				float ci = a;
				r = (k)*cr + (1-k)*r;
				i = (k)*ci + (1-k)*i;
			}


			mesh[id].x = r;
			mesh[id].y = i;
		}
);

float rand01() {
	return (rand()%1000000)/1000000.0f;
}


int main() {

	try	{
		tic();
		srand(3543);

		const float fps=60;
		const unsigned N = 1024*512;

		Context::gl_version version;
		version.major = 4;
		version.minor = 3;

		Window window;
		window.create(512, 512, "complex function exploration", Window::options::visible | Window::options::frame | Window::options::resizable , version);
		window.set_viewport(Rect(0, 0, window.get_width(), window.get_height()));
		window.set_clear_color(Color(0.3,0.3,0.3,1.0));

		Keyboard keyboard;
		keyboard.set_window(window);


		//Shader creation
		Shader shader;
		shader.create_from_memory("position", vertex_shader, fragment_shader);

		//Renderer creation
		Renderer renderer;
		renderer.set_target(&window);
		renderer.set_shader(&shader);
		renderer.set_polygon_mode(Renderer::polygon_mode::points);


		cout << "==================================================\n";
		cout << "GL_VENDOR   : " << (char const*)glGetString(GL_VENDOR) << endl;
		cout << "GL_RENDERER : " << (char const*)glGetString(GL_RENDERER) << endl;
		cout << "==================================================\n";
		cout << "  [0] :   z^[1-2]\n";
		cout << "  [1] :   cos(z)\n";
		cout << "  [2] :   sin(z)\n";
		cout << "  [3] :   cosh(z)\n";
		cout << "  [4] :   sinh(z)\n";
		cout << "  [5] :   exp(z)\n";
		cout << "  [6] :   ln(z)\n";
		cout << "==================================================\n";


		//Mesh creation : grid of (cell_x * cell_y) cells --> (cell_x + 1) * (cell_y + 1) points
		Mesh mesh = Shape::create_grid_mesh(POINTS_X-1, POINTS_Y-1, Vector3f(-2.0f, 2.0f, 0.0f), Vector3f(-2.0f, -2.0f, 0.0f), Vector3f(2.0f, 2.0f, 0.0f));
		//Mesh mesh = Shape::create_grid_mesh(POINTS_X-1, POINTS_Y-1);


		//Color buffer
		std::vector<Color> colors_vector;
		colors_vector.resize(mesh.elements.get_size());
		try
		{
			Texture texture;
			texture.create("images/big-lena.png");
			size_t dx = texture.get_width() / POINTS_X;
			size_t dy = texture.get_height() / POINTS_Y;

			for (size_t y = 0; y < POINTS_Y; y++) {
				for (size_t x = 0; x < POINTS_X; x++) {
					size_t xi = x*dx;
					size_t yi = y*dy;

					colors_vector[x + y*POINTS_X] = texture[xi + yi*texture.get_width()];
				}
			}
		}
		catch (...)
		{
			cerr << "image not found" << endl;
			//Put random colors
			for (size_t i = 0; i < colors_vector.size(); i++) {
				float i_norm = static_cast<float>(i)/static_cast<float>(colors_vector.size());
				colors_vector[i] = Color(i_norm, 1.0f-i_norm, (i_norm/2.0f)+0.25f, 1.0f);
			}
		}

		Buffer<Color> colors;
		colors.create(colors_vector.data(), colors_vector.size());
		shader.set_varying("color", colors);




		// Parallel kernel creation
		Parallel<Parogl> cpt;
		cpt.add_src(code);
		cpt.build("t", "Mesh", "Colors", "cmd");
		cpt.set_range(ComputeSize(POINTS_X * POINTS_Y / 64), ComputeSize(64));//*/

		// parametres controle
		unsigned cmd = 6;

		cout << toc() << "s" << endl;

		float t = 0;
		bool pause = false;
		bool toggler = false;
		while (window && !keyboard.is_pressed(Keyboard::key::escape))
		{
			tic();
			window.clear();


			if (keyboard.is_pressed(Keyboard::key::kp_0)) 		cmd = 0;
			if (keyboard.is_pressed(Keyboard::key::kp_1)) 		cmd = 1;
			if (keyboard.is_pressed(Keyboard::key::kp_2)) 		cmd = 2;
			if (keyboard.is_pressed(Keyboard::key::kp_3)) 		cmd = 3;
			if (keyboard.is_pressed(Keyboard::key::kp_4)) 		cmd = 4;
			if (keyboard.is_pressed(Keyboard::key::kp_5)) 		cmd = 5;
			if (keyboard.is_pressed(Keyboard::key::kp_6)) 		cmd = 6;
			if (keyboard.is_pressed(Keyboard::key::r))  {
				colors.to_gpu();
				mesh.elements.to_gpu();
			}
			if (keyboard.is_pressed(Keyboard::key::space)) {
				if (toggler)
					pause = !pause;
				toggler = false;
			}
			else {
				toggler = true;
			}

			if (!pause) {
				t += 1.0f/fps;
				if (cmd == 0) mesh.elements.to_gpu();
				if (cmd == 1) mesh.elements.to_gpu();
				if (cmd == 2) mesh.elements.to_gpu();
				if (cmd == 3) mesh.elements.to_gpu();
				if (cmd == 4) mesh.elements.to_gpu();
				if (cmd == 5) mesh.elements.to_gpu();
				if (cmd == 6) mesh.elements.to_gpu();
				cpt(t, mesh.elements, colors, cmd);
			}


			window.clear();
			renderer.draw(mesh);


			float dt = 1/fps - toc();
			if (dt > 0.0f)
				sleep(second(dt));
			//*/

			window.update();
		}
	}
	catch (Error& e) {
		printf("error: %s", e.what());
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
