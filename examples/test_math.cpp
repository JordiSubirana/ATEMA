#include <atema/atema.hpp>

#include <iostream>

using namespace std;
using namespace at;

int main(int argc, char ** argv)
{
	try
	{
		Matrix3x2i mat;
		Vector4f vec(4.4f, 3.3f, 2.2f, 1.1f);
		
		cout << "Matrix :" << endl;
		cout << mat << endl;
		
		cout << "Vector :" << endl;
		cout << vec << endl;
	}
	catch (const std::exception& e)
	{
		printf("ERROR\n");
		printf("%s", e.what());
	}
}