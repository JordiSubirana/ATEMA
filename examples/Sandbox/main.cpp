#include <Atema/Atema.hpp>

#include "SandboxApplication.hpp"

#include <iostream>

using namespace at;

// MAIN
int main(int argc, char** argv)
{
	try
	{
		SandboxApplication app;

		app.run();
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		
		return -1;
	}
	
	return 0;
}