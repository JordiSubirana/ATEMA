# define CL_HPP_TARGET_OPENCL_VERSION 200
#include <CL/cl2.hpp>

#include <atema/context/context.hpp>
#include <atema/window/window.hpp>
#include <atema/window/keyboard.hpp>
#include <atema/core/error.hpp>

#include <iostream>
#include <cstdio>
#include <atema/parallel/parallel.hpp>
#include <atema/context/opengl.hpp>

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









void checkErrors(std::string desc) {
	GLenum e = glGetError();
	if (e != GL_NO_ERROR) {
		//fprintf(stderr, "OpenGL error in \"%s\": %s (%d)\n", desc.c_str(), gluErrorString(e), e);
		fprintf(stderr, "OpenGL error in \"%s\": %s (%d)\n", desc.c_str(), "???", e);
		exit(20);
	}
}

GLuint genRenderProg(GLuint texHandle) {
	GLuint progHandle = glCreateProgram();
	GLuint vp = glCreateShader(GL_VERTEX_SHADER);
	GLuint fp = glCreateShader(GL_FRAGMENT_SHADER);

	const char *vpSrc[] = {
			"#version 430\n",
			"in vec2 pos;\
		 out vec2 texCoord;\
		 void main() {\
			 texCoord = pos*0.5f + 0.5f;\
			 gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);\
		 }"
	};

	const char *fpSrc[] = {
			"#version 430\n",
			"uniform sampler2D srcTex;\
		 in vec2 texCoord;\
		 out vec4 color;\
		 void main() {\
			 float c = texture(srcTex, texCoord).x;\
			 color = vec4(c, 1.0, 1.0, 1.0);\
		 }"
	};

	glShaderSource(vp, 2, vpSrc, NULL);
	glShaderSource(fp, 2, fpSrc, NULL);

	glCompileShader(vp);
	int rvalue;
	glGetShaderiv(vp, GL_COMPILE_STATUS, &rvalue);
	if (!rvalue) {
		fprintf(stderr, "Error in compiling vp\n");
		exit(30);
	}
	glAttachShader(progHandle, vp);

	glCompileShader(fp);
	glGetShaderiv(fp, GL_COMPILE_STATUS, &rvalue);
	if (!rvalue) {
		fprintf(stderr, "Error in compiling fp\n");
		exit(31);
	}
	glAttachShader(progHandle, fp);

	glBindFragDataLocation(progHandle, 0, "color");
	glLinkProgram(progHandle);

	glGetProgramiv(progHandle, GL_LINK_STATUS, &rvalue);
	if (!rvalue) {
		fprintf(stderr, "Error in linking sp\n");
		exit(32);
	}

	glUseProgram(progHandle);
	glUniform1i(glGetUniformLocation(progHandle, "srcTex"), 0);

	GLuint vertArray;
	glGenVertexArrays(1, &vertArray);
	glBindVertexArray(vertArray);

	GLuint posBuf;
	glGenBuffers(1, &posBuf);
	glBindBuffer(GL_ARRAY_BUFFER, posBuf);
	float data[] = {
			-1.0f, -1.0f,
			-1.0f, 1.0f,
			1.0f, -1.0f,
			1.0f, 1.0f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*8, data, GL_STREAM_DRAW);
	GLint posPtr = glGetAttribLocation(progHandle, "pos");
	glVertexAttribPointer(posPtr, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posPtr);

	checkErrors("Render shaders");
	return progHandle;
}

GLuint genTexture() {
	// We create a single float channel 512^2 texture
	GLuint texHandle;
	glGenTextures(1, &texHandle);

	glActiveTexture(GL_TEXTURE0);
	checkErrors("Gen texture0");
	glBindTexture(GL_TEXTURE_2D, texHandle);
	checkErrors("Gen texture1");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	checkErrors("Gen texture2");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	checkErrors("Gen texture3");
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 512, 512, 0, GL_RED, GL_FLOAT, NULL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_FLOAT, NULL);
	checkErrors("Gen texture4");

	// Because we're also using this tex as an image (in order to write to it),
	// we bind it to an image unit as well
	//glBindImageTexture(0, texHandle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glBindImageTexture(0, texHandle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	checkErrors("Gen texture5");
	return texHandle;
}


GLuint genComputeProg(GLuint texHandle) {
	// Creating the compute shader, and the program object containing the shader
	GLuint progHandle = glCreateProgram();
	GLuint cs = glCreateShader(GL_COMPUTE_SHADER);

	// In order to write to a texture, we have to introduce it as image2D.
	// local_size_x/y/z layout variables define the work group size.
	// gl_GlobalInvocationID is a uvec3 variable giving the global ID of the thread,
	// gl_LocalInvocationID is the local index within the work group, and
	// gl_WorkGroupID is the work group's index
	#define STRINGIFY(A) #A



	string code = STRINGIFY(
			layout (local_size_x = 16, local_size_y = 16) in;
			uniform float roll;

			//uniform writeonly image2D destTex;
			//uniform writeonly image2D destTex;
			//layout(binding = 0) writeonly uniform image2D destTex;
			layout(rgba8, binding = 0) uniform mediump image2D destTex;

			void main() {
				ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
				//float localCoef = length(vec2(ivec2(gl_LocalInvocationID.xy) - 8) / 8.0);
				//float globalCoef = sin(float(gl_WorkGroupID.x + gl_WorkGroupID.y) * 0.1 + roll) * 0.5;
				//imageStore(destTex, storePos, vec4(1.0-globalCoef*localCoef, 0.0, 0.0, 1.0));
				imageStore(destTex, storePos, vec4(0.5, 0, 0, 0.5));
			}
	);
	const char *csSrc[] = {	"#version 430\n", code.c_str()};

	glShaderSource(cs, 2, csSrc, NULL);
	glCompileShader(cs);
	int rvalue;
	glGetShaderiv(cs, GL_COMPILE_STATUS, &rvalue);
	if (!rvalue) {
		fprintf(stderr, "Error in compiling the compute shader\n");
		GLchar log[10240];
		GLsizei length;
		glGetShaderInfoLog(cs, 10239, &length, log);
		fprintf(stderr, "Compiler log:\n%s\n", log);
		exit(40);
	}
	glAttachShader(progHandle, cs);

	glLinkProgram(progHandle);
	glGetProgramiv(progHandle, GL_LINK_STATUS, &rvalue);
	if (!rvalue) {
		fprintf(stderr, "Error in linking compute shader program\n");
		GLchar log[10240];
		GLsizei length;
		glGetProgramInfoLog(progHandle, 10239, &length, log);
		fprintf(stderr, "Linker log:\n%s\n", log);
		exit(41);
	}
	glUseProgram(progHandle);

	glUniform1i(glGetUniformLocation(progHandle, "destTex"), 0);

	checkErrors("Compute shader");
	return progHandle;
}







void test();

int main() {

	try
	{

		test();

		scan_machine();

		Window window;
		Keyboard keyboard;

		Context::gl_version version;
		version.major = 4;
		version.minor = 3;

		window.create(512, 512, "Test", Window::options::visible | Window::options::frame | Window::options::resizable , version);

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


		////////////////////////////////////////////////////////////////////////////////////////
		glViewport(0, 0, 512, 512);
		glGetError();
		GLuint renderHandle, computeHandle;
		GLuint texHandle = genTexture();
		renderHandle = genRenderProg(texHandle);
		computeHandle = genComputeProg(texHandle);


		int frame = 0;
		while (window && !keyboard.is_pressed(Keyboard::key::escape))
		{

			glUseProgram(computeHandle);
			glUniform1f(glGetUniformLocation(computeHandle, "roll"), (float)frame*0.1f);
			glBindImageTexture(0, texHandle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
			glDispatchCompute(512/16, 512/16, 1); // 512^2 threads in blocks of 16^2
			checkErrors("Dispatch compute shader");
			frame++;

			glUseProgram(renderHandle);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//*/

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



struct A {

	A() {
		cout << "A()" << endl;
	}

	/*
	template <typename T> A(T &&a) {
		cout << "A(T &&a)" << a << endl;
	}//*/



/*
	A(int& a) {
		cout << "A(int& a)" << a << endl;
	}
*/
	A(int const& a) {
		cout << "A(int const& a)" << a << endl;
	}
/*
	A(int&& a) {
		cout << "A(int&& a)" << a << endl;
	}

	A(int const&& a) {
		cout << "A(int const&& a)" << a << endl;
	}

*/

	A(A& a) {
		cout << "A(A& a)" << endl;
	}

	A(A const& a) {
		cout << "A(A const& a)" << endl;
	}

	A(A&& a) {
		cout << "A(A&& a)" << endl;
	}

	A(A const&& a) {
		cout << "A(A const&& a)" << endl;
	}




	A(string& a) {
		cout << "A(string& a)" << a << endl;
	}

	A(string const& a) {
		cout << "A(string const& a)" << a << endl;
	}

	A(string&& a) {
		cout << "A(string&& a)" << a << endl;
		string s(std::move(a));
	}

	A(string const&& a) {
		cout << "A(string const&& a)" << a << endl;
	}

//*/


};



class Runner {
public:

	void add_src(std::string s)  {
		cout << "Runner::add_src(\"" << s << "\");" << endl;
	}

	void build() {
		cout << "Runner::build();";
	}

	template<typename T>
	void setArg(unsigned i, T arg) {
		cout << "Runner::setArg(" << i << ", " << arg << ");" << endl;
	}


	void run() {
		cout << "Runner::run();" << endl;
	}



};


template<> // specialisation en dehors de la classe
void Runner::setArg<char>(unsigned i, char arg) {
	cout << "Runner::setArg<char>(" << i << ", " << arg << ");" << endl;
}


void test() {

	Parallel<Runner> p;

	p.add_src("s1", "s2");
	p.add_file("f1", "f2");

	p.build();

	p(1, 5, '3', "nicole");


	return ;
	const A a;

	string s = "hello;";
	int i = 64;

	A b(a);
	A c(42);
	A d(i);
	A e(std::move(s));
	A f(std::string("sDAsd"));
	cout << "s after:" << (s);

}










