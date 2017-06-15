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

#include <Atema/OpenGL/IndexBuffer.hpp>
#include <glad/glad.h>

namespace at
{
	namespace
	{
		constexpr size_t indexTypeSizes[] =
		{
			sizeof(unsigned char), sizeof(unsigned short), sizeof(unsigned)
		};

		constexpr GLenum glTypes[] =
		{
			GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT
		};
	}

	OpenGLIndexBuffer::OpenGLIndexBuffer() :
		OpenGLBuffer(Type::ElementArray), m_type(IndexBuffer::Type::UnsignedInt),
		m_size(0), m_glType(GL_UNSIGNED_INT)
	{
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
	}

	void OpenGLIndexBuffer::setData(const void* indices, size_t byteSize, IndexBuffer::Type type)
	{
		OpenGLBuffer::setData(indices, byteSize);

		size_t size = byteSize / indexTypeSizes[static_cast<unsigned>(m_type)];

		if (size != m_size || m_type != type)
			invalidate();

		m_type = type;
		m_size = size;
		m_glType = glTypes[static_cast<unsigned>(m_type)];
	}

	void OpenGLIndexBuffer::getData(void* indices)
	{
		OpenGLBuffer::getData(indices);
	}

	size_t OpenGLIndexBuffer::getSize() const
	{
		return m_size;
	}

	size_t OpenGLIndexBuffer::getByteSize() const
	{
		return indexTypeSizes[static_cast<unsigned>(m_type)];
	}

	IndexBuffer::Type OpenGLIndexBuffer::getType() const
	{
		return m_type;
	}

	unsigned OpenGLIndexBuffer::getOpenGLType() const
	{
		return m_glType;
	}
}
