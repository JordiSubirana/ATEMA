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

#include <Atema/OpenGL/Texture.hpp>
#include <Atema/Core/Error.hpp>

#include <glad/glad.h>

namespace at
{
	OpenGLTexture::OpenGLTexture()
		: m_width(0), m_height(0)
	{
		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_2D, m_id);

		if (glIsTexture(m_id) == GL_FALSE)
			ATEMA_ERROR("OpenGL could not create a texture.");

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT); //GL_CLAMP_TO_EDGE - GL_REPEAT
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
	}

	OpenGLTexture::~OpenGLTexture()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &m_id);
	}

	unsigned OpenGLTexture::getId() const
	{
		return m_id;
	}

	void OpenGLTexture::setData(const std::vector<Color>& pixels, unsigned width, unsigned height)
	{
		m_width = width;
		m_height = height;

		//TODO: Manage custom internal types
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height), 0, GL_RGBA, GL_FLOAT, reinterpret_cast<const float*>(pixels.data()));
	}

	void OpenGLTexture::getData(std::vector<Color>& pixels) const
	{
		pixels.resize(m_width*m_height);

		glBindTexture(GL_TEXTURE_2D, m_id);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, reinterpret_cast<float*>(pixels.data()));
	}

	void OpenGLTexture::resize(unsigned width, unsigned height)
	{
		m_width = width;
		m_height = height;

		//TODO: Manage custom internal types
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height), 0, GL_RGBA, GL_FLOAT, nullptr);
	}

	unsigned OpenGLTexture::getWidth() const
	{
		return m_width;
	}

	unsigned OpenGLTexture::getHeight() const
	{
		return m_height;
	}
}
