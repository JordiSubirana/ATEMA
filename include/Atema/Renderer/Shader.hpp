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

#ifndef ATEMA_RENDERER_SHADER_HPP
#define ATEMA_RENDERER_SHADER_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Renderer/RendererDependent.hpp>
#include <string>
#include <Atema/Core/Matrix.hpp>

namespace at
{
	class ATEMA_RENDERER_API Shader
	{
	public:
		enum class Type
		{
			Vertex, Fragment
		};

		class ATEMA_RENDERER_API Implementation : public RendererDependent
		{
		public:
			Implementation();
			virtual ~Implementation() = default;

			virtual void addFromMemory(Type type, const std::string& shaderCode) = 0;

			virtual void build() = 0;
			virtual void clear() = 0;

			virtual bool isValid() const = 0;

			virtual void setUniform(const std::string& variable, int arg) = 0;
			virtual void setUniform(const std::string& variable, int arg1, int arg2) = 0;
			virtual void setUniform(const std::string& variable, int arg1, int arg2, int arg3) = 0;
			virtual void setUniform(const std::string& variable, int arg1, int arg2, int arg3, int arg4) = 0;
			virtual void setUniform(const std::string& variable, unsigned arg) = 0;
			virtual void setUniform(const std::string& variable, unsigned arg1, unsigned arg2) = 0;
			virtual void setUniform(const std::string& variable, unsigned arg1, unsigned arg2, unsigned arg3) = 0;
			virtual void setUniform(const std::string& variable, unsigned arg1, unsigned arg2, unsigned arg3, unsigned arg4) = 0;
			virtual void setUniform(const std::string& variable, float arg) = 0;
			virtual void setUniform(const std::string& variable, float arg1, float arg2) = 0;
			virtual void setUniform(const std::string& variable, float arg1, float arg2, float arg3) = 0;
			virtual void setUniform(const std::string& variable, float arg1, float arg2, float arg3, float arg4) = 0;
			virtual void setUniform(const std::string& variable, double arg) = 0;
			virtual void setUniform(const std::string& variable, double arg1, double arg2) = 0;
			virtual void setUniform(const std::string& variable, double arg1, double arg2, double arg3) = 0;
			virtual void setUniform(const std::string& variable, double arg1, double arg2, double arg3, double arg4) = 0;
			virtual void setUniform(const std::string& variable, const Matrix2f& mat) = 0;
			virtual void setUniform(const std::string& variable, const Matrix3f& mat) = 0;
			virtual void setUniform(const std::string& variable, const Matrix4f& mat) = 0;
			virtual void setUniform(const std::string& variable, const Matrix2d& mat) = 0;
			virtual void setUniform(const std::string& variable, const Matrix3d& mat) = 0;
			virtual void setUniform(const std::string& variable, const Matrix4d& mat) = 0;
		};

		Shader();
		virtual ~Shader();

		Implementation* getImplementation();
		const Implementation* getImplementation() const;

		void addFromFile(Type type, const std::string& shaderFile);
		void addFromMemory(Type type, const std::string& shaderCode);

		void build();
		void clear();

		bool isValid() const;

		void setUniform(const std::string& variable, int arg);
		void setUniform(const std::string& variable, int arg1, int arg2);
		void setUniform(const std::string& variable, int arg1, int arg2, int arg3);
		void setUniform(const std::string& variable, int arg1, int arg2, int arg3, int arg4);
		void setUniform(const std::string& variable, unsigned arg);
		void setUniform(const std::string& variable, unsigned arg1, unsigned arg2);
		void setUniform(const std::string& variable, unsigned arg1, unsigned arg2, unsigned arg3);
		void setUniform(const std::string& variable, unsigned arg1, unsigned arg2, unsigned arg3, unsigned arg4);
		void setUniform(const std::string& variable, float arg);
		void setUniform(const std::string& variable, float arg1, float arg2);
		void setUniform(const std::string& variable, float arg1, float arg2, float arg3);
		void setUniform(const std::string& variable, float arg1, float arg2, float arg3, float arg4);
		void setUniform(const std::string& variable, double arg);
		void setUniform(const std::string& variable, double arg1, double arg2);
		void setUniform(const std::string& variable, double arg1, double arg2, double arg3);
		void setUniform(const std::string& variable, double arg1, double arg2, double arg3, double arg4);
		void setUniform(const std::string& variable, const Matrix2f& mat);
		void setUniform(const std::string& variable, const Matrix3f& mat);
		void setUniform(const std::string& variable, const Matrix4f& mat);
		void setUniform(const std::string& variable, const Matrix2d& mat);
		void setUniform(const std::string& variable, const Matrix3d& mat);
		void setUniform(const std::string& variable, const Matrix4d& mat);  

	private:
		Implementation *m_impl;
	};
}

#endif