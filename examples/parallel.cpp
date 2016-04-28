# define CL_HPP_TARGET_OPENCL_VERSION 200
#include <CL/cl2.hpp>

#include <atema/context/context.hpp>
#include <atema/window/window.hpp>
#include <atema/window/keyboard.hpp>
#include <atema/core/error.hpp>

#include <iostream>
#include <cstdio>





using namespace std;
using namespace at;



void scan_machine() {

    //get all platforms (drivers)
    std::vector<cl::Platform> platforms;
    std::vector<cl::Device> devices;

    cl::Platform::get(&platforms);

    printf("Found %d platform(s).\n", (int)platforms.size());

    if(platforms.size() == 0){
        std::cout<<"No platforms found. Check OpenCL installation!\n";
        return ;
    }
    for (unsigned p=0 ; p<platforms.size() ; p++) {
        std::cout << "Platform["<< p <<"]: "<< platforms[p].getInfo<CL_PLATFORM_NAME>() << std::endl;

        platforms[p].getDevices(CL_DEVICE_TYPE_ALL, &devices);

        std::cout << "Found " << devices.size() << " device(s) on platform " << p << std::endl;

        if(devices.size()==0){
            std::cout<<"    No devices found.\n";
        }
        for (unsigned d=0 ; d<devices.size() ; d++) {
            std::cout << "    Found device ["<< d << "]: "<<devices[d].getInfo<CL_DEVICE_NAME>()<<"\n";
        }
    }
    std::cout << "\n";
}




int main() {
    scan_machine();

	try
	{
		Window window;
		Keyboard keyboard;

		Context::gl_version version;
		version.major = 4;
		version.minor = 5;

		window.create(640, 480, "Test", Window::options::visible | Window::options::frame | Window::options::resizable , version);

		keyboard.set_window(window);

		printf("Hello World !\n");

		#if defined(ATEMA_SYSTEM_WINDOWS)
		HGLRC test_context = Window::get_current_os_context();

		if (test_context)
			printf("Windows gl context found !!!\n");
		#elif defined(ATEMA_SYSTEM_LINUX)

		GLXContext test_context = Window::get_current_os_context();

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



