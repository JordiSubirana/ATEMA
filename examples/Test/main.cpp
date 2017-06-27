#include <Atema/Atema.hpp>

#include <iostream>
#include <chrono>
#include <random>
#include <thread>

#include <Atema/Graphics/Model.hpp>

using namespace std;
using namespace at;

static const string vertexShader =
"#version 330 core\n" \
"in vec3 pos;\n" \
"in vec4 offset;\n" \
"in vec4 color;\n" \
"out vec4 fcolor;\n" \
"void main() {\n" \
"fcolor = color;\n" \
"gl_Position = vec4(pos.xyz+offset.xyz, 1);}";

static const string fragmentShader =
"#version 330 core\n" \
"in vec4 fcolor;\n" \
"out vec4 out_color;\n" \
"void main() {\n" \
"out_color = fcolor; }";

struct Position : VertexAttribute<float>
{
	float x, y, z;
};
const string VertexAttributeName<Position>::value = "pos";

struct Offset : VertexAttribute<float>
{
	float x, y, z, w;
};
const string VertexAttributeName<Offset>::value = "offset";

struct Col : VertexAttribute<float>
{
	float r, g, b, a;
};
const string VertexAttributeName<Col>::value = "color";

struct VertexPosition : Vertex<Position>
{
	VertexPosition(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {}
	float x, y, z;
};
VertexFormat Vertex<Position>::format = getDefaultFormat();

struct VertexParticle : Vertex<Offset, Col>
{
	VertexParticle() : position{ 0, 0, 0, 0 }, color{ 1.0f, 0.0f, 0.0f, 1.0f } {}

	struct
	{
		float x, y, z, w;
	} position;
	struct
	{
		float r, g, b, a;
	} color;
};
VertexFormat Vertex<Offset, Col>::format = getDefaultFormat();

struct Particle
{
	Particle()
	{
		reset();
	}

	void reset()
	{
		float rx = static_cast<float>(rand() % 50 - 25) / 15.0f;
		float ry = static_cast<float>(rand() % 25) / 15.0f;
		float rz = static_cast<float>(rand() % 50 - 25) / 15.0f;

		float r = (static_cast<float>(rand() % 255)+0) / 255;
		float g = (static_cast<float>(rand() % 255)+0) / 255;
		float b = (static_cast<float>(rand() % 255)+0) / 255;

		speed = Vector3f(rx, ry, rz);
		color = Color(r, g, b);
		duration = 0.0f;
	}

	Vector3f speed;
	Color color;
	float duration;
};

void initVBO(VertexBuffer& vbo, Vector3f origin, float s)
{
	float ox = origin.x;
	float oy = origin.y;
	float oz = origin.z;

	vector<VertexPosition> vertices =
	{
		VertexPosition(ox - s, oy + s, oz),
		VertexPosition(ox - s, oy - s, oz),
		VertexPosition(ox + s, oy + s, oz),
		VertexPosition(ox + s, oy + s, oz),
		VertexPosition(ox - s, oy - s, oz),
		VertexPosition(ox + s, oy - s, oz)
	};

	vbo.setData(vertices);
}

int main(int argc, char **argv)
{
	try
	{
		srand(42);

		OpenGLRenderSystem renderer;

		Window window(750, 750, "Hello World");

		//Model model;
		//model.loadFromFile("resources/tardis.obj");

		Shader shader;
		shader.addFromMemory(Shader::Type::Vertex, vertexShader);
		shader.addFromMemory(Shader::Type::Fragment, fragmentShader);
		shader.build();

		//shader.setUniform("color", 0.0f, 1.0f, 0.0f, 1.0f);

		vector<VertexPosition> vertices =
		{
			VertexPosition(-0.02f, 0.5f + 0.02f, 0.0f),
			VertexPosition(-0.02f, 0.5f - 0.02f, 0.0f),
			VertexPosition(+0.02f, 0.5f + 0.02f, 0.0f),
			VertexPosition(+0.02f, 0.5f + 0.02f, 0.0f),
			VertexPosition(-0.02f, 0.5f - 0.02f, 0.0f),
			VertexPosition(+0.02f, 0.5f - 0.02f, 0.0f)
		};

		VertexBuffer vbo1, vbo2, vbo3;
		initVBO(vbo1, Vector3f(0.0f, 0.2f, 0.0f), 0.01f);
		initVBO(vbo2, Vector3f(0.5f, 0.5f, -0.1f), 0.005f);
		initVBO(vbo3, Vector3f(-0.5f, 0.7f, -0.2f), 0.003f);

		size_t size = 1000;
		size_t perSec = static_cast<size_t>(static_cast<float>(size) / 1.0f);
		vector<Particle> particles;
		vector<Particle> initParticles;
		particles.resize(size);
		initParticles.resize(size);

		vector<VertexParticle> offsets;
		offsets.resize(size);

		VertexBuffer instanceData;
		instanceData.setData(offsets);

		Renderer pass;
		pass.setTarget(&window);
		pass.setShader(&shader);

		float totalDelta = 0.0f;
		int count = 0;
		float fps = 0.0f;

		size_t lastIndex = 0;

		while (!window.shouldClose())
		{
			auto start = chrono::high_resolution_clock::now();

			window.clearColor(Color(0, 0, 0));
			window.clearDepth();

			//pass.draw(vbo);
			pass.draw(vbo1, instanceData);
			pass.draw(vbo2, instanceData);
			pass.draw(vbo3, instanceData);

			window.processEvents();
			window.swapBuffers();

			auto stop = chrono::high_resolution_clock::now();

			float deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count() / 1000.0f;
			fps += (1.0f / deltaTime) * 1000.0f;
			count++;

			totalDelta += deltaTime / 1000.0f;

			float dt = deltaTime / 1000.0f;
			size_t nbToCreate = static_cast<size_t>(ceil(dt*perSec));

			VertexParticle *data = instanceData.map<VertexParticle>();

			for (size_t i = 0; i < nbToCreate; i++)
			{
				size_t index = (lastIndex + i) % size;
				auto& particle = particles[index];
				particle = initParticles[index];
				particle.duration = 1.0f;

				data[index].position.x = 0.0f;
				data[index].position.y = 0.0f;
				data[index].position.z = 0.0f;

				data[index].color = { particle.color.r, particle.color.g, particle.color.b, 0.5f };
			}

			for (size_t i = 0; i < size; i++)
			{
				auto& particle = particles[i];

				if (particle.duration > 0.0f)
				{
					particle.speed.y -= 9.81f*dt;
				}
				particle.duration -= dt;

				if (particle.duration <= 0.0f)
				{
					particle.duration = 0.0f;
					particle.color.a = 0.0f;
					particle.speed = Vector3f(0.0f, 0.0f, 0.0f);

					data[i].position.x = 10.0f;
					data[i].position.y = 10.0f;
					data[i].position.z = 10.0f;
				}

				if (particle.duration > 0.0f)
				{
					//data[i].color = { 0.0f, 1.0f, 0.0f, 0.0f };
					//data[i].color = { particle.color.r, particle.color.g, particle.color.b, particle.color.a };
					data[i].position.x += particle.speed.x*dt;
					data[i].position.y += particle.speed.y*dt;
					data[i].position.z += particle.speed.z*dt;
				}
			}

			lastIndex = (lastIndex + nbToCreate) % size;

			if (totalDelta > 0.5f)
			{
				window.setTitle(to_string(fps / count));
				totalDelta = 0.0f;
				fps = 0.0f;
				count = 0;
			}

			this_thread::sleep_for(chrono::microseconds(1));
		}
	}
	catch (const exception& e)
	{
		cout << e.what() << endl;

		return -1;
	}

	return 0;
}