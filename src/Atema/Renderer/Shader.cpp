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

#include <Atema/Renderer/Shader.hpp>
#include <Atema/Renderer/RenderSystem.hpp>
#include <Atema/Core/Error.hpp>
#include <fstream>
#include <sstream>

namespace at
{
	Shader::Implementation::Implementation()
	{
	}

	Shader::Shader() : m_impl(nullptr)
	{
		RenderSystem *renderer = RenderSystem::getCurrent();

		if (!renderer)
			ATEMA_ERROR("There is no current RenderSystem.");

		m_impl = renderer->createShader();
	}

	Shader::~Shader()
	{
		delete m_impl;
	}

	Shader::Implementation* Shader::getImplementation()
	{
		return m_impl;
	}

	const Shader::Implementation* Shader::getImplementation() const
	{
		return m_impl;
	}

	void Shader::addFromFile(Type type, const std::string& shaderFile)
	{
		std::ifstream ifs(shaderFile, std::ifstream::in);
		std::stringstream str;

		while (ifs.good())
		{
			std::string tmp;
			getline(ifs, tmp);
			str << tmp;
		}

		ifs.close();

		addFromMemory(type, str.str());
	}

	void Shader::addFromMemory(Type type, const std::string& shaderCode)
	{
		m_impl->addFromMemory(type, shaderCode);
	}

	void Shader::build()
	{
		m_impl->build();
	}

	void Shader::clear()
	{
		m_impl->clear();
	}

	bool Shader::isValid() const
	{
		return m_impl->isValid();
	}

	void Shader::setUniform(const std::string& variable, int arg)
	{
		m_impl->setUniform(variable, arg);
	}

	void Shader::setUniform(const std::string& variable, int arg1, int arg2)
	{
		m_impl->setUniform(variable, arg1, arg2);
	}

	void Shader::setUniform(const std::string& variable, int arg1, int arg2, int arg3)
	{
		m_impl->setUniform(variable, arg1, arg2, arg3);
	}

	void Shader::setUniform(const std::string& variable, int arg1, int arg2, int arg3, int arg4)
	{
		m_impl->setUniform(variable, arg1, arg2, arg3, arg4);
	}

	void Shader::setUniform(const std::string& variable, unsigned arg)
	{
		m_impl->setUniform(variable, arg);
	}

	void Shader::setUniform(const std::string& variable, unsigned arg1, unsigned arg2)
	{
		m_impl->setUniform(variable, arg1, arg2);
	}

	void Shader::setUniform(const std::string& variable, unsigned arg1, unsigned arg2, unsigned arg3)
	{
		m_impl->setUniform(variable, arg1, arg2, arg3);
	}

	void Shader::setUniform(const std::string& variable, unsigned arg1, unsigned arg2, unsigned arg3, unsigned arg4)
	{
		m_impl->setUniform(variable, arg1, arg2, arg3, arg4);
	}

	void Shader::setUniform(const std::string& variable, float arg)
	{
		m_impl->setUniform(variable, arg);
	}

	void Shader::setUniform(const std::string& variable, float arg1, float arg2)
	{
		m_impl->setUniform(variable, arg1, arg2);
	}

	void Shader::setUniform(const std::string& variable, float arg1, float arg2, float arg3)
	{
		m_impl->setUniform(variable, arg1, arg2, arg3);
	}

	void Shader::setUniform(const std::string& variable, float arg1, float arg2, float arg3, float arg4)
	{
		m_impl->setUniform(variable, arg1, arg2, arg3, arg4);
	}

	void Shader::setUniform(const std::string& variable, double arg)
	{
		m_impl->setUniform(variable, arg);
	}

	void Shader::setUniform(const std::string& variable, double arg1, double arg2)
	{
		m_impl->setUniform(variable, arg1, arg2);
	}

	void Shader::setUniform(const std::string& variable, double arg1, double arg2, double arg3)
	{
		m_impl->setUniform(variable, arg1, arg2, arg3);
	}

	void Shader::setUniform(const std::string& variable, double arg1, double arg2, double arg3, double arg4)
	{
		m_impl->setUniform(variable, arg1, arg2, arg3, arg4);
	}

	void Shader::setUniform(const std::string& variable, const Matrix2f& mat)
	{
		m_impl->setUniform(variable, mat);
	}

	void Shader::setUniform(const std::string& variable, const Matrix3f& mat)
	{
		m_impl->setUniform(variable, mat);
	}

	void Shader::setUniform(const std::string& variable, const Matrix4f& mat)
	{
		m_impl->setUniform(variable, mat);
	}

	void Shader::setUniform(const std::string& variable, const Matrix2d& mat)
	{
		m_impl->setUniform(variable, mat);
	}

	void Shader::setUniform(const std::string& variable, const Matrix3d& mat)
	{
		m_impl->setUniform(variable, mat);
	}

	void Shader::setUniform(const std::string& variable, const Matrix4d& mat)
	{
		m_impl->setUniform(variable, mat);
	}
}
