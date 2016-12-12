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