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

#include <Atema/OpenGL/Buffer.hpp>
#include <Atema/Core/Error.hpp>

#include <glad/glad.h>

namespace at
{
	namespace
	{
		constexpr int openGLBufferTypes[] =
		{
			GL_ARRAY_BUFFER, GL_ATOMIC_COUNTER_BUFFER, GL_COPY_READ_BUFFER,
			GL_COPY_WRITE_BUFFER, GL_DISPATCH_INDIRECT_BUFFER, GL_DRAW_INDIRECT_BUFFER,
			GL_ELEMENT_ARRAY_BUFFER, GL_PIXEL_PACK_BUFFER, GL_PIXEL_UNPACK_BUFFER,
			GL_QUERY_BUFFER, GL_SHADER_STORAGE_BUFFER, GL_TEXTURE_BUFFER,
			GL_TRANSFORM_FEEDBACK_BUFFER, GL_UNIFORM_BUFFER
		};
	}

	OpenGLBuffer::OpenGLBuffer(Type type) : m_byteSize(0), m_map(nullptr)
	{
		m_type = openGLBufferTypes[static_cast<int>(type)];

		glGenBuffers(1, &m_id);
		glBindBuffer(m_type, m_id);

		if (glIsBuffer(m_id) == GL_FALSE)
			ATEMA_ERROR("OpenGL could not create a buffer.");
	}

	OpenGLBuffer::~OpenGLBuffer()
	{
		unmap();

		glBindBuffer(m_type, 0);
		glDeleteBuffers(1, &m_id);
	}

	unsigned OpenGLBuffer::getId() const
	{
		return m_id;
	}

	void OpenGLBuffer::setData(const void* data, size_t byteSize)
	{
		if (!data || !byteSize)
			ATEMA_ERROR("Invalid buffer data.");

		unmap();

		glBindBuffer(m_type, m_id);
		glBufferData(m_type, byteSize, data, GL_STATIC_DRAW);

		m_byteSize = byteSize;
	}

	void OpenGLBuffer::getData(void* data)
	{
		if (!data)
			return;

		unmap();

		glBindBuffer(m_type, m_id);
		glGetBufferSubData(m_type, 0, m_byteSize, data);
	}

	void* OpenGLBuffer::map()
	{
		if (m_map)
			return m_map;

		glBindBuffer(m_type, m_id);
		m_map = glMapBuffer(m_type, GL_READ_WRITE);

		if (!m_map)
			ATEMA_ERROR("Error while mapping buffer.");

		return m_map;
	}

	void OpenGLBuffer::unmap() const
	{
		if (m_map)
		{
			glBindBuffer(m_type, m_id);
			glUnmapBuffer(m_type);
			m_map = nullptr;
		}
	}

	size_t OpenGLBuffer::getTotalByteSize() const
	{
		return m_byteSize;
	}
}
