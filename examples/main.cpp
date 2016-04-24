#include <atema/context/context.hpp>
#include <atema/window/window.hpp>
#include <atema/window/keyboard.hpp>
//#include <atema/utility/test.hpp>
#include <atema/core/error.hpp>

#include <iostream>
#include <cstdio>

using namespace std;
using namespace at;

int main()
{
	try
	{
		at::window window;
		at::keyboard keyboard;
		
		context::gl_version version;
		version.major = 3;
		version.minor = 3;
		
		window.create(640, 480, "Test", window::options::visible | window::options::frame, version);
		
		keyboard.set_window(window);
		
		printf("Hello World !\n");
		
		while (window && !keyboard.is_pressed(keyboard::key::escape))
		{
			window.update();
		}
	}
	catch (error& e)
	{
		printf("ERROR\n");
		printf(e.what());
	}
	
	return 0;
}


