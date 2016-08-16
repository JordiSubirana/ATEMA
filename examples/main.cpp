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
		
		string file = "3d/Spider-Man_Symbiote.obj";
		
		cout << File::get_path(file) << " --- " << File::get_extension(file) << endl;
		
		if (File::extension_match(file, "obj"))
			cout << "Extension match !!" << endl;
		
		Model model;
		
		model.create("3d/Spider-Man_Symbiote.obj");
	}
	catch (Error& e)
	{
		printf("ERROR\n");
		printf("%s", e.what());
	}
	
	return 0;
}


