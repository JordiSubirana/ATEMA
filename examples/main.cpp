#include <atema/atema.hpp>

#include <iostream>
#include <cstdio>

using namespace std;
using namespace at;

int main()
{
	try
	{
		//Setup OpenGL version
		Context::gl_version version;
		version.major = 3;
		version.minor = 3;
		
		Context context;
		context.create(512, 512, version);
		
		Texture tex;
		
		tex.create("images/lena-diagonals.png");
		
		tex.save("main_lena_save.png");
	}
	catch (Error& e)
	{
		printf("ERROR\n");
		printf("%s", e.what());
	}
	
	return 0;
}


