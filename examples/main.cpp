// #include <atema/window/window.hpp>
#include <atema/core/error.hpp>

#include <iostream>
#include <cstdio>

using namespace std;
using namespace at;

int main()
{
	/*
	at::window w;
	
	context::gl_version version;
	version.major = 3;
	version.minor = 3;
	
	w.create(640, 480, "Test", window::options::visible | window::options::frame, version);
	*/
	
	error err("fils de pute");
	
	printf("Hello World !\n");
	printf(err.what());
	
	return 0;
}


