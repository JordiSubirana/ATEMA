
#include <atema/context/context.hpp>
#include <atema/window/window.hpp>
#include <atema/window/keyboard.hpp>
#include <atema/core/error.hpp>

#include <iostream>
#include <cstdio>
#include <atema/parallel/parallel.hpp>
#include <atema/parallel/parogl.hpp>
#include <atema/parallel/ocl.hpp>
#include <atema/context/opengl.hpp>

using namespace std;
using namespace at;


#define STRINGIFY(A) #A

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








static void checkErrors(std::string src) {

	GLenum err = glGetError();

	if (err != GL_NO_ERROR) {
		string error = "OpenGL error in \"";
		error += src;
		error += "\": ";

		switch(err) {
			case GL_INVALID_OPERATION:
				error += "INVALID_OPERATION";
				break;
			case GL_INVALID_ENUM:
				error += "INVALID_ENUM";
				break;
			case GL_INVALID_VALUE:
				error += "INVALID_VALUE";
				break;
			case GL_OUT_OF_MEMORY:
				error += "OUT_OF_MEMORY";
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				error += "INVALID_FRAMEBUFFER_OPERATION";
				break;
			case GL_STACK_OVERFLOW:
				error += "STACK_OVERFLOW";
				break;
			case GL_STACK_UNDERFLOW:
				error += "STACK_UNDERFLOW";
				break;
			default:
				error += "Unknown error";
				break;
		}

		error += " (";
		error += to_string(err);
		error += ")";

		ATEMA_ERROR(error.c_str());
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
			 color =  texture(srcTex, texCoord);\
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 512, 512, 0, GL_RGBA, GL_FLOAT, NULL);
	checkErrors("Gen texture4");

	// Because we're also using this tex as an image (in order to write to it),
	// we bind it to an image unit as well
	//glBindImageTexture(0, texHandle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

	glBindImageTexture(0, texHandle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	checkErrors("Gen texture5");
	return texHandle;
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

		cout << "GL_VENDOR " << (char const*)glGetString(GL_VENDOR) << endl;
		cout << "GL_RENDERER " << (char const*)glGetString(GL_RENDERER) << endl;


		glGetError();
		////////////////////////////////////////////////////////////////////////////////////////
		glViewport(0, 0, 512, 512);
		glGetError();
		GLuint renderHandle;
		GLuint texHandle = genTexture();
		renderHandle = genRenderProg(texHandle);







		string code = STRINGIFY(
				uniform writeonly image2D destTex;
				uniform uint salut;
				//buffer Pos { float Position[]; };
				uniform uint salzut;

				void main() {
					//if (salut == 0) { Position[0] = 5.0f; }

					ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
					float kk = salut*salzut;
					imageStore(destTex, storePos, vec4( 0.5*(1+cos((storePos.x+storePos.y*cos(kk/1000))/5.0f)) , (1.0f+sin(kk/100))/2.0f, storePos.x*0.5*(1+cos(sqrt(2)*kk/100))/512, 1));
				}
		);

		Parallel<Parogl> cpt;
		cpt.add_src(code);
		cpt.build();
		cpt.setRange(ComputeSize(512/16, 512/16), ComputeSize(16,16));

		cout << "==================================================\n";



		unsigned frame = 1;
		while (window && !keyboard.is_pressed(Keyboard::key::escape))
		{

			cpt(GLO_Image(texHandle), frame, (unsigned)1);


			glUseProgram(renderHandle);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			checkErrors("draw");

			window.update();
			checkErrors("update");
			frame++;
		}
	}
	catch (Error& e)
	{
		printf("ERROR\n");
		printf("%s", e.what());
	}


    return 0;
}





class Runner {
public:

	void add_src(std::string s)  {
		cout << "Runner::add_src(\"" << s << "\");" << endl;
	}

	void build() {
		cout << "Runner::build();" << endl;
	}

	void prerun() {
		cout << "Runner::prerun();" << endl;
	}

	void run() {
		cout << "Runner::run();" << endl;
	}

	void setArg(unsigned i, string arg) {
		cout << "Runner::setArg<string>(" << i << ", " << arg << ");" << endl;
	}

	void setArg(unsigned i, double arg) {
		cout << "Runner::setArg<double>(" << i << ", " << arg << ");" << endl;
	}

};



void test() {

	Parallel<Runner> p;


	p.add_src("s1", "s2");
	//p.add_file("f1", "f2");

	p.build();

	p(1, 5, '3', "nicole");


	return ;

}

/*
 * z b.t
 * HLM
 * f?  ; repeat
 * D  suppr la fin
 *
 * :s/old/new    sur la ligne
 * :l1,l2s/old/new    sur le range
 * :%s/old/new      fichier
 * I    debut ligne + insert
 */








