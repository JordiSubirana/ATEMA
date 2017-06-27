/*
	Copyright 2017 Jordi SUBIRANA

	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
	Software, and to permit persons to whom the Software is furnished to do so, subject
	to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
	PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
	CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
	OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <Atema/OpenGL/Shader.hpp>
#include <Atema/Core/Error.hpp>
#include <glad/glad.h>
#include <iostream>

#define ATEMA_UNIFORM(at_call) \
	if (m_id) \
	{ \
		glUseProgram(m_id); \
		at_call; \
	} \
	else \
	{ \
		ATEMA_ERROR("Invalid shader."); \
	}

#define ATEMA_UNIFORM_ARG_1(at_func, at_type) \
	ATEMA_UNIFORM(at_func(getUniformLocation(variable, at_type), arg))

#define ATEMA_UNIFORM_ARG_2(at_func, at_type) \
	ATEMA_UNIFORM(at_func(getUniformLocation(variable, at_type), arg1, arg2))

#define ATEMA_UNIFORM_ARG_3(at_func, at_type) \
	ATEMA_UNIFORM(at_func(getUniformLocation(variable, at_type), arg1, arg2, arg3))

#define ATEMA_UNIFORM_ARG_4(at_func, at_type) \
	ATEMA_UNIFORM(at_func(getUniformLocation(variable, at_type), arg1, arg2, arg3, arg4))

#define ATEMA_UNIFORM_MATRIX(at_func, at_type) \
	ATEMA_UNIFORM(at_func(getUniformLocation(variable, at_type), 1, GL_FALSE, mat.get()))

namespace at
{
	namespace
	{
		constexpr GLenum openGLShaderTypes[] =
		{
			GL_VERTEX_SHADER, GL_FRAGMENT_SHADER
		};

		at::VertexAttribute::Type getAtemaType(GLenum glType)
		{
			switch (glType)
			{
				case GL_INT:
					return at::VertexAttribute::Type::Int;
				case GL_INT_VEC2:
					return at::VertexAttribute::Type::Int2;
				case GL_INT_VEC3:
					return at::VertexAttribute::Type::Int3;
				case GL_INT_VEC4:
					return at::VertexAttribute::Type::Int4;
				case GL_UNSIGNED_INT:
					return at::VertexAttribute::Type::Unsigned;
				case GL_UNSIGNED_INT_VEC2:
					return at::VertexAttribute::Type::Unsigned2;
				case GL_UNSIGNED_INT_VEC3:
					return at::VertexAttribute::Type::Unsigned3;
				case GL_UNSIGNED_INT_VEC4:
					return at::VertexAttribute::Type::Unsigned4;
				case GL_FLOAT:
					return at::VertexAttribute::Type::Float;
				case GL_FLOAT_VEC2:
					return at::VertexAttribute::Type::Float2;
				case GL_FLOAT_VEC3:
					return at::VertexAttribute::Type::Float3;
				case GL_FLOAT_VEC4:
					return at::VertexAttribute::Type::Float4;
				case GL_DOUBLE:
					return at::VertexAttribute::Type::Double;
				case GL_DOUBLE_VEC2:
					return at::VertexAttribute::Type::Double2;
				case GL_DOUBLE_VEC3:
					return at::VertexAttribute::Type::Double3;
				case GL_DOUBLE_VEC4:
					return at::VertexAttribute::Type::Double4;
				default:
					break;
			}

			return at::VertexAttribute::Type::Undefined;
		}

		GLenum getOpenGLBaseType(GLenum glType)
		{
			switch (glType)
			{
				case GL_INT:
				case GL_INT_VEC2:
				case GL_INT_VEC3:
				case GL_INT_VEC4:
					return GL_INT;
				case GL_UNSIGNED_INT:
				case GL_UNSIGNED_INT_VEC2:
				case GL_UNSIGNED_INT_VEC3:
				case GL_UNSIGNED_INT_VEC4:
					return GL_UNSIGNED_INT;
				case GL_FLOAT:
				case GL_FLOAT_VEC2:
				case GL_FLOAT_VEC3:
				case GL_FLOAT_VEC4:
					return GL_FLOAT;
				case GL_DOUBLE:
				case GL_DOUBLE_VEC2:
				case GL_DOUBLE_VEC3:
				case GL_DOUBLE_VEC4:
					return GL_DOUBLE;
				default:
					break;
			}

			return 0;
		}
	}

	OpenGLShader::OpenGLShader() : m_id(0)
	{
	}

	OpenGLShader::~OpenGLShader()
	{
		if (glIsProgram(m_id))
			glDeleteProgram(m_id);

		for (auto shader : m_shaders)
		{
			if (glIsShader(shader))
				glDeleteShader(shader);
		}
	}

	unsigned OpenGLShader::getId() const
	{
		return m_id;
	}

	void OpenGLShader::addFromMemory(Shader::Type type, const std::string& shaderCode)
	{
		GLuint shaderId = 0;

		try
		{
			GLint result = GL_FALSE;

			glGetError();

			if (shaderCode.empty())
				ATEMA_ERROR("No shader given.");

			shaderId = glCreateShader(openGLShaderTypes[static_cast<int>(type)]);

			if ((glGetError() != GL_NO_ERROR) || (shaderId == 0))
				ATEMA_ERROR("Shader creation failed.");

			const char *tmp = shaderCode.c_str();

			glShaderSource(shaderId, 1, &(tmp), nullptr);

			if (glGetError() != GL_NO_ERROR)
				ATEMA_ERROR("Shader allocation failed.");

			glCompileShader(shaderId);

			//TODO: get the OpenGL compilation errors.
			glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);

			// /*
			int logLength = 0;
			glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
			if (logLength > 1)
			{
				std::vector<char> shaderErrorMessage(logLength + 1);
				glGetShaderInfoLog(shaderId, logLength, nullptr, &shaderErrorMessage[0]);
				std::cout << static_cast<const char*>(&shaderErrorMessage[0]) << std::endl;
			}
			// */

			if ((glGetError() != GL_NO_ERROR) || (result == GL_FALSE))
				ATEMA_ERROR("OpenGL internal error: Shader compilation failed.");

			m_shaders.push_back(shaderId);
		}
		catch (...)
		{
			if (glIsShader(shaderId))
				glDeleteShader(shaderId);

			throw;
		}
	}

	void OpenGLShader::build()
	{
		GLuint prog = 0;

		if (glIsProgram(m_id))
			glDeleteProgram(m_id);

		invalidate();

		m_id = 0;

		m_uniforms.clear();
		m_attributes.clear();

		try
		{
			GLint result = GL_FALSE;

			glGetError();

			prog = glCreateProgram();

			if ((glGetError() != GL_NO_ERROR) || (prog == 0))
				ATEMA_ERROR("Shader creation failed.");

			for (auto shader : m_shaders)
				glAttachShader(prog, shader);

			glLinkProgram(prog);

			//TODO: get the OpenGL linking errors.
			glGetProgramiv(prog, GL_LINK_STATUS, &result);

			// /*
			int InfoLogLength = 0;
			glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &InfoLogLength);
			if (InfoLogLength > 1)
			{
				std::vector<char> programErrorMessage(InfoLogLength + 1);
				glGetProgramInfoLog(prog, InfoLogLength, nullptr, &programErrorMessage[0]);
				std::cout << static_cast<const char*>(&programErrorMessage[0]) << std::endl;
			}
			// */

			if ((glGetError() != GL_NO_ERROR) || (result == GL_FALSE))
				ATEMA_ERROR("Shader linking failed.");

			m_id = prog;

			GLint size; // size of the variable
			GLenum type; // type of the variable (float, vec3 or mat4, etc)
			const int bufSize = 32; // maximum name length
			char name[bufSize]; // variable name in GLSL
			int length; // name length
			int count = 0;

			glGetProgramiv(m_id, GL_ACTIVE_ATTRIBUTES, &count);
			for (int i = 0; i < count; i++)
			{
				glGetActiveAttrib(m_id, i, bufSize, &length, &size, &type, name);

				VertexAttribute attribute;
				attribute.name = name;
				attribute.type = getAtemaType(type);
				attribute.glBaseType = getOpenGLBaseType(type);
				attribute.location = glGetAttribLocation(m_id, name);

				m_attributes.push_back(attribute);
			}

			glGetProgramiv(m_id, GL_ACTIVE_UNIFORMS, &count);
			for (int i = 0; i < count; i++)
			{
				glGetActiveUniform(m_id, i, bufSize, &length, &size, &type, name);

				Uniform uniform;
				uniform.location = glGetUniformLocation(m_id, name);
				uniform.type = type;

				m_uniforms[std::string(name, length)] = uniform;
			}
		}
		catch (...)
		{
			if (glIsProgram(prog))
				glDeleteProgram(prog);

			throw;
		}
	}

	void OpenGLShader::clear()
	{
		if (glIsProgram(m_id))
			glDeleteProgram(m_id);

		m_id = 0;

		for (auto shader : m_shaders)
		{
			if (glIsShader(shader))
				glDeleteShader(shader);
		}
	}

	bool OpenGLShader::isValid() const
	{
		return m_id != 0;
	}

	void OpenGLShader::setUniform(const std::string& variable, int arg)
	{
		ATEMA_UNIFORM_ARG_1(glUniform1i, GL_INT);
	}

	void OpenGLShader::setUniform(const std::string& variable, int arg1, int arg2)
	{
		ATEMA_UNIFORM_ARG_2(glUniform2i, GL_INT_VEC2);
	}

	void OpenGLShader::setUniform(const std::string& variable, int arg1, int arg2, int arg3)
	{
		ATEMA_UNIFORM_ARG_3(glUniform3i, GL_INT_VEC3);
	}

	void OpenGLShader::setUniform(const std::string& variable, int arg1, int arg2, int arg3, int arg4)
	{
		ATEMA_UNIFORM_ARG_4(glUniform4i, GL_INT_VEC4);
	}

	void OpenGLShader::setUniform(const std::string& variable, unsigned arg)
	{
		ATEMA_UNIFORM_ARG_1(glUniform1ui, GL_UNSIGNED_INT);
	}

	void OpenGLShader::setUniform(const std::string& variable, unsigned arg1, unsigned arg2)
	{
		ATEMA_UNIFORM_ARG_2(glUniform2ui, GL_UNSIGNED_INT_VEC2);
	}

	void OpenGLShader::setUniform(const std::string& variable, unsigned arg1, unsigned arg2, unsigned arg3)
	{
		ATEMA_UNIFORM_ARG_3(glUniform3ui, GL_UNSIGNED_INT_VEC3);
	}

	void OpenGLShader::setUniform(const std::string& variable, unsigned arg1, unsigned arg2, unsigned arg3, unsigned arg4)
	{
		ATEMA_UNIFORM_ARG_4(glUniform4ui, GL_UNSIGNED_INT_VEC4);
	}

	void OpenGLShader::setUniform(const std::string& variable, float arg)
	{
		ATEMA_UNIFORM_ARG_1(glUniform1f, GL_FLOAT);
	}

	void OpenGLShader::setUniform(const std::string& variable, float arg1, float arg2)
	{
		ATEMA_UNIFORM_ARG_2(glUniform2f, GL_FLOAT_VEC2);
	}

	void OpenGLShader::setUniform(const std::string& variable, float arg1, float arg2, float arg3)
	{
		ATEMA_UNIFORM_ARG_3(glUniform3f, GL_FLOAT_VEC3);
	}

	void OpenGLShader::setUniform(const std::string& variable, float arg1, float arg2, float arg3, float arg4)
	{
		ATEMA_UNIFORM_ARG_4(glUniform4f, GL_FLOAT_VEC4);
	}

	void OpenGLShader::setUniform(const std::string& variable, double arg)
	{
		ATEMA_UNIFORM_ARG_1(glUniform1d, GL_DOUBLE);
	}

	void OpenGLShader::setUniform(const std::string& variable, double arg1, double arg2)
	{
		ATEMA_UNIFORM_ARG_2(glUniform2d, GL_DOUBLE_VEC2);
	}

	void OpenGLShader::setUniform(const std::string& variable, double arg1, double arg2, double arg3)
	{
		ATEMA_UNIFORM_ARG_3(glUniform3d, GL_DOUBLE_VEC3);
	}

	void OpenGLShader::setUniform(const std::string& variable, double arg1, double arg2, double arg3, double arg4)
	{
		ATEMA_UNIFORM_ARG_4(glUniform4d, GL_DOUBLE_VEC4);
	}

	void OpenGLShader::setUniform(const std::string& variable, const Matrix2f& mat)
	{
		ATEMA_UNIFORM_MATRIX(glUniformMatrix2fv, GL_FLOAT_MAT2);
	}

	void OpenGLShader::setUniform(const std::string& variable, const Matrix3f& mat)
	{
		ATEMA_UNIFORM_MATRIX(glUniformMatrix3fv, GL_FLOAT_MAT3);
	}

	void OpenGLShader::setUniform(const std::string& variable, const Matrix4f& mat)
	{
		ATEMA_UNIFORM_MATRIX(glUniformMatrix4fv, GL_FLOAT_MAT4);
	}

	void OpenGLShader::setUniform(const std::string& variable, const Matrix2d& mat)
	{
		ATEMA_UNIFORM_MATRIX(glUniformMatrix2dv, GL_DOUBLE_MAT2);
	}

	void OpenGLShader::setUniform(const std::string& variable, const Matrix3d& mat)
	{
		ATEMA_UNIFORM_MATRIX(glUniformMatrix3dv, GL_DOUBLE_MAT3);
	}

	void OpenGLShader::setUniform(const std::string& variable, const Matrix4d& mat)
	{
		ATEMA_UNIFORM_MATRIX(glUniformMatrix4dv, GL_DOUBLE_MAT4);
	}

	const std::vector<OpenGLShader::VertexAttribute>& OpenGLShader::getVertexAttributes() const
	{
		return m_attributes;
	}

	void OpenGLShader::bind()
	{
		glUseProgram(m_id);
	}

	int OpenGLShader::getUniformLocation(const std::string& variable, int type)
	{
		auto it = m_uniforms.find(variable);

		if (it == m_uniforms.end())
			ATEMA_ERROR("Specified uniform doesn't exist.");

		if (it->second.type != type)
			ATEMA_ERROR("The required uniform name doesn't have the right type.");

		return it->second.location;
	}
}
