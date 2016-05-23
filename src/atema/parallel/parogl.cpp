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
#include <sstream>

using namespace std;


namespace at {


    static inline void glCheckError(std::string src) {

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
                //"#extension GL_NV_shader_buffer_load : enable\n"
                "layout (local_size_variable) in;\n"
                "const float PI = 3.14159265359f;\n"
                "#define image2Dwr(format) layout(format) uniform image2D\n";

        _srcs.push_back(headers);


        _ssbo_i = 0;
        _ssbo_count = 0;
        _uniform_i = 0;
        _uniform_count = 0;
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


        glGetProgramInterfaceiv(_progID, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &dst);
        glCheckError("glGetProgamInterfaceiv");

        _ssbo_count = dst;



        /*
        for (int i = 0; i < dst; ++i) {
            char n[955] = {0};
            int k;
            glGetProgramResourceName(_progID, GL_SHADER_STORAGE_BLOCK, i, 954, &k, n);
            k = glGetProgramResourceLocation(_progID, GL_UNIFORM, n);
            cout << k << " " << n << endl;
        }//*/

    }



    void Parogl::set_range(ComputeSize groupCount, ComputeSize groupSize) {
        gcx = (GLuint) groupCount.x;
        gcy = (GLuint) groupCount.y;
        gcz = (GLuint) groupCount.z;
        gsx = (GLuint) groupSize.x;
        gsy = (GLuint) groupSize.y;
        gsz = (GLuint) groupSize.z;
    }



    void Parogl::prerun() {
        _uniform_i = 0;
        _texunit_i = 0;
        _ssbo_i = 0;

        glUseProgram(_progID);
        glCheckError("glUseProgram");
    }


    void at::Parogl::run() {
        glDispatchComputeGroupSizeARB(gcx, gcy, gcz, gsx, gsy, gsz); // 512^2 threads in blocks of 16^2
        glCheckError("glDispatchComputeGroupSizeARB");
    }



    void Parogl::set_arg(unsigned i, Texture const& image) {

        GLint location;

        if (_uniform_i >= _uniform_count)
            ATEMA_ERROR("OpenGL error uniform count overflow");

        glActiveTexture(GL_TEXTURE0 + _texunit_i);
        glCheckError("glActivateTexture");

        //todo manage read/write qualifiers (inside texture)
        //todo magage textures/images
        glBindImageTexture(_texunit_i, image.get_gl_id(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        glCheckError("glBindImageTexture");

        location = glGetUniformLocation(_progID, _argList[i].c_str());
        glCheckError("getGetUniformLocation");

        if (location == -1) {
            ostringstream oss;
            oss << "OpenGL error : variable \"" << _argList[i] << "\" in shader can not be found";
            ATEMA_ERROR(oss.str().c_str())
        }

        glUniform1i(location, _texunit_i);
        glCheckError("glUniform1i(image2d)");

        _uniform_i++;
        _texunit_i++;
    }



    void Parogl::set_arg_buffer(unsigned i, GLuint id) {

        if (_ssbo_i >= _ssbo_count)
            ATEMA_ERROR("OpenGL error SSBO count overflow");

        GLuint block_index;

        //glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, _ssbo_i, id);
        glCheckError("glBindBufferBase");

        block_index = glGetProgramResourceIndex(_progID, GL_SHADER_STORAGE_BLOCK, _argList[i].c_str());
        glCheckError("glGetProgramResourceIndex");

        if (block_index == -1) {
            ostringstream oss;
            oss << "OpenGL error : variable \"" << _argList[i] << "\" in shader can not be found";
            ATEMA_ERROR(oss.str().c_str())
        }

        glShaderStorageBlockBinding(_progID, block_index, _ssbo_i);
        glCheckError("glShaderStorageBlockBinding");

        _ssbo_i++;
    }




    #define implem_glUniform(T, suf, par) \
    void Parogl::set_arg(unsigned i, T val) {\
        \
        GLint location;\
        \
        if (_uniform_i >= _uniform_count)\
            ATEMA_ERROR("OpenGL error uniform count overflow");\
        \
        location = glGetUniformLocation(_progID, _argList[i].c_str());\
        glCheckError("getGetUniformLocation"); \
        \
        if (location == -1) {\
            ostringstream oss;\
            oss << "OpenGL error : variable \"" << _argList[i] << "\" in shader can not be found";\
            ATEMA_ERROR(oss.str().c_str())\
        }\
        \
        glUniform ## suf(location, par);\
        glCheckError("glUniform" #suf);\
        \
        _uniform_i++;\
    }



    #define COMMA ,

    implem_glUniform(unsigned, 1ui, val)
    implem_glUniform(int,      1i, val)
    implem_glUniform(float,    1f, val)
    implem_glUniform(double,   1d, val)

    implem_glUniform(Vector2u, 2ui, val.x COMMA val.y)
    implem_glUniform(Vector2i, 2i, val.x COMMA val.y)
    implem_glUniform(Vector2f, 2f, val.x COMMA val.y)
    implem_glUniform(Vector2d, 2d, val.x COMMA val.y)

    implem_glUniform(Vector3u, 3ui, val.x COMMA val.y COMMA val.z)
    implem_glUniform(Vector3i, 3i, val.x COMMA val.y COMMA val.z)
    implem_glUniform(Vector3f, 3f, val.x COMMA val.y COMMA val.z)
    implem_glUniform(Vector3d, 3d, val.x COMMA val.y COMMA val.z)

    implem_glUniform(Vector4u, 4ui, val.x COMMA val.y COMMA val.z COMMA val.z)
    implem_glUniform(Vector4i, 4i, val.x COMMA val.y COMMA val.z COMMA val.z)
    implem_glUniform(Vector4f, 4f, val.x COMMA val.y COMMA val.z COMMA val.z)
    implem_glUniform(Vector4d, 4d, val.x COMMA val.y COMMA val.z COMMA val.z)



}
