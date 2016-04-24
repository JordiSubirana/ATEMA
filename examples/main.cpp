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
		at::Window window;
		Keyboard keyboard;
		
		Context::gl_version version;
		version.major = 3;
		version.minor = 3;
		
		window.create(640, 480, "Test", at::Window::options::visible | at::Window::options::frame, version);
		
		keyboard.set_window(window);
		
		printf("Hello World !\n");
		
		#if defined(ATEMA_SYSTEM_WINDOWS)
		HGLRC test_context = Window::get_current_os_context();
		
		if (test_context)
			printf("Windows gl context found !!!\n");
		#elif defined(ATEMA_SYSTEM_LINUX)

		GLXContext test_context = at::Window::get_current_os_context();
		
		if (test_context)
			printf("Linux gl context found !!!\n");
		//*/
		#endif
		
		while (window && !keyboard.is_pressed(Keyboard::key::escape))
		{
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


