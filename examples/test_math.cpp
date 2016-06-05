#include <atema/atema.hpp>

#include <iostream>

using namespace std;
using namespace at;

int main(int argc, char ** argv)
{
	try
	{
		Matrix3x2i mat;
		
		Matrix4f transform;
		Vector4f vec(1.0f, 2.0f, 3.0f, 4.0f);
		
		//Transform 2*I
		for (int i = 0; i < 4; i++)
			transform[i][i] = 2.0f;
		
		cout << "Matrix :" << endl;
		cout << mat << endl;
		
		cout << "Vector :" << endl;
		cout << vec << endl;
		
		cout << "Transformed vector :" << endl;
		cout << (transform * vec) << endl;
		
		transform = look_at(Vector3f(0,0,0), Vector3f(1,0,0), Vector3f(0,1,0));
		
		cout << "Look At :" << endl;
		cout << transform << endl;
	}
	catch (const std::exception& e)
	{
		printf("ERROR\n");
		printf("%s", e.what());
	}
}