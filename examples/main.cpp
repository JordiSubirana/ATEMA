#include <Atema/Atema.hpp>
#include <iostream>

using namespace std;
using namespace at;

int main(int argc, char ** argv)
{
	try
	{
		OpenGLRenderer renderer;
		Ref<Window> window = Window::create(640, 480, "Hello World");
		
		Ref<Texture> texture = Texture::create(640, 480);
		
		VertexBufferStorage<Vector3f> vbo_storage;
		for (int i = 0; i < 10; i++)
			vbo_storage.data.push_back(Vector3f(i, i, i));
		
		Ref<VertexBuffer> vbo = VertexBuffer::create(vbo_storage);
		
		vbo_storage.data.clear();
		vbo->download(vbo_storage);
		cout << "vbo data:" << endl;
		for (int i = 0; i < (int)vbo_storage.data.size(); i++)
			cout << i << endl;
		
		while (!window->should_close())
		{
			window->update();
		}
	}
	catch (const exception& e)
	{
		cout << e.what() << endl;
	}
	
	return (0);
}