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

#ifndef ATEMA_OPENGL_SHADER_HPP
#define ATEMA_OPENGL_SHADER_HPP

#include <Atema/OpenGL/Config.hpp>
#include <Atema/Renderer/Shader.hpp>
#include <Atema/Renderer/Vertex.hpp>
#include <Atema/OpenGL/VaoKeeper.hpp>
#include <vector>
#include <map>

namespace at
{	
	class ATEMA_OPENGL_API OpenGLShader : public Shader::Implementation, public OpenGLVaoKeeper
	{
	public:
		struct VertexAttribute
		{
			std::string name;
			VertexAttributeType type;
			int glBaseType;
			int location;
		};

		OpenGLShader();
		virtual ~OpenGLShader();

		unsigned getId() const;
		
		void addFromMemory(Shader::Type type, const std::string& shaderCode) override;
		
		void build() override;
		void clear() override;
		
		bool isValid() const override;

		void setUniform(const std::string& variable, int arg) override;
		void setUniform(const std::string& variable, int arg1, int arg2) override;
		void setUniform(const std::string& variable, int arg1, int arg2, int arg3) override;
		void setUniform(const std::string& variable, int arg1, int arg2, int arg3, int arg4) override;
		void setUniform(const std::string& variable, unsigned arg) override;
		void setUniform(const std::string& variable, unsigned arg1, unsigned arg2) override;
		void setUniform(const std::string& variable, unsigned arg1, unsigned arg2, unsigned arg3) override;
		void setUniform(const std::string& variable, unsigned arg1, unsigned arg2, unsigned arg3, unsigned arg4) override;
		void setUniform(const std::string& variable, float arg) override;
		void setUniform(const std::string& variable, float arg1, float arg2) override;
		void setUniform(const std::string& variable, float arg1, float arg2, float arg3) override;
		void setUniform(const std::string& variable, float arg1, float arg2, float arg3, float arg4) override;
		void setUniform(const std::string& variable, double arg) override;
		void setUniform(const std::string& variable, double arg1, double arg2) override;
		void setUniform(const std::string& variable, double arg1, double arg2, double arg3) override;
		void setUniform(const std::string& variable, double arg1, double arg2, double arg3, double arg4) override;
		void setUniform(const std::string& variable, const Matrix2f& mat) override;
		void setUniform(const std::string& variable, const Matrix3f& mat) override;
		void setUniform(const std::string& variable, const Matrix4f& mat) override;
		void setUniform(const std::string& variable, const Matrix2d& mat) override;
		void setUniform(const std::string& variable, const Matrix3d& mat) override;
		void setUniform(const std::string& variable, const Matrix4d& mat) override;
		
		const std::vector<VertexAttribute>& getVertexAttributes() const;
		void bind();

	private:
		struct Uniform
		{
			int location;
			int type;
		};

		int getUniformLocation(const std::string& variable, int type);

		unsigned m_id;
		std::vector<unsigned> m_shaders;
		std::map<std::string, Uniform> m_uniforms;
		std::vector<VertexAttribute> m_attributes;
	};
}

#endif