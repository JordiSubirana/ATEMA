// ----------------------------------------------------------------------
// Copyright (C) 2016 Jordi SUBIRANA
//
// This file is part of ATEMA.
//
// ATEMA is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ATEMA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ATEMA.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------


#include <atema/parallel/parogl.hpp>
#include <iostream>

using namespace std;


namespace at {


    static void glCheckError(std::string src) {

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


    Parogl::Parogl() {

        _progID = glCreateProgram();
        glCheckError("glCreateProgram");

        _shadID = glCreateShader(GL_COMPUTE_SHADER);
        glCheckError("glCreateShader");

        std::string headers =
                "#version 430\n"
                "#extension GL_ARB_compute_shader : enable\n"
                "#extension GL_ARB_shader_storage_buffer_object : enable\n"
                "#extension GL_ARB_compute_variable_group_size : enable\n"
                "layout (local_size_variable) in;\n";

        _srcs.push_back(headers);


        _uniform_i = 0;
        _texunit_i = 0;
    }


    Parogl::~Parogl() {
        glDeleteProgram(_progID);
    }


    void at::Parogl::add_src(std::string const& s) {
        _srcs.push_back(s);
    }



    void at::Parogl::_build() {
        int retval;
        GLsizei length;
        static GLchar log[8192];
        std::string error;
        string src;

        for (auto &s : _srcs) {
            src += s;
        }

        const char *csrc = src.c_str();
        glShaderSource(_shadID, (GLsizei) 1, &csrc, NULL);
        glCheckError("glShaderSource");

        glCompileShader(_shadID);
        glGetShaderiv(_shadID, GL_COMPILE_STATUS, &retval);
        if (!retval) {
            error = "OpenGL error in compiling compute shader:\n";
            glGetShaderInfoLog(_shadID, 8191, &length, log);
            error += log;
            ATEMA_ERROR(error.c_str());
        }

        glAttachShader(_progID, _shadID);
        glCheckError("glAttachShader");

        glLinkProgram(_progID);
        glGetProgramiv(_progID, GL_LINK_STATUS, &retval);
        if (!retval) {
            error = "OpenGL error in linking compute shader:\n";
            glGetShaderInfoLog(_shadID, 8191, &length, log);
            error += log;
            ATEMA_ERROR(error.c_str());
        }

        GLint dst;
        glGetProgramInterfaceiv(_progID, GL_UNIFORM, GL_ACTIVE_RESOURCES, &dst);
        glCheckError("glGetProgamInterfaceiv");

        _uniform_count = dst;

        /*
        for (int i = 0; i < dst; ++i) {
            char n[955] = {0};
            int k;
            glGetProgramResourceName(_progID, GL_UNIFORM, i, 954, &k, n);
            k = glGetProgramResourceLocation(_progID, GL_UNIFORM, n);
            cout << k << " " << n << endl;
        }//*/

    }



    void Parogl::setRange(ComputeSize groupCount, ComputeSize groupSize) {
        gcx = (GLuint) groupCount.x;
        gcy = (GLuint) groupCount.y;
        gcz = (GLuint) groupCount.z;
        gsx = (GLuint) groupSize.x;
        gsy = (GLuint) groupSize.y;
        gsz = (GLuint) groupSize.z;
    }


    void at::Parogl::run() {
        glDispatchComputeGroupSizeARB(gcx, gcy, gcz, gsx, gsy, gsz); // 512^2 threads in blocks of 16^2
        glCheckError("glDispatchComputeGroupSizeARB");
    }

    void Parogl::prerun() {
        _uniform_i = 0;
        _texunit_i = 0;

        glUseProgram(_progID);
    }


    void Parogl::setArg(unsigned i, Texture const& image) {
        (void)i;

        GLint location;

        if (_uniform_i >= _uniform_count)
            ATEMA_ERROR("OpenGL error uniform count overflow");

        glActiveTexture(GL_TEXTURE0 + _texunit_i);
        glCheckError("glActivateTexture");

        glBindImageTexture(_texunit_i, image.get_gl_id(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glCheckError("glBindImageTexture");

        location = glGetUniformLocation(_progID, _argList[i].c_str());
        glCheckError("getGetUniformLocation");

        glUniform1i(location, _texunit_i);
        glCheckError("glUniform1i(image2d)");

        _uniform_i++;
        _texunit_i++;
    }


    #define implem_glUniform1T(T, suf) \
    void Parogl::setArg(unsigned i, T val) {\
        (void)i;\
        \
        GLint location;\
        \
        if (_uniform_i >= _uniform_count)\
            ATEMA_ERROR("OpenGL error uniform count overflow");\
        \
        location = glGetUniformLocation(_progID, _argList[i].c_str());\
        glCheckError("getGetUniformLocation"); \
        \
        glUniform1 ## suf(location, val);\
        glCheckError("glUniform1" #suf);\
        \
        _uniform_i++;\
    }


    implem_glUniform1T(unsigned, ui)
    implem_glUniform1T(int, i)
    implem_glUniform1T(float, f)


}
