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

#include <Atema/OpenGL/VertexBuffer.hpp>
#include <Atema/Core/Error.hpp>
#include <Atema/OpenGL/Context.hpp>
#include <Atema/OpenGL/IndexBuffer.hpp>
#include <Atema/OpenGL/Shader.hpp>
#include <Atema/OpenGL/VertexArray.hpp>

#include <glad/glad.h>

namespace at
{
	namespace
	{
		constexpr GLenum glDrawModes[] =
		{
			GL_TRIANGLES
		};
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer() :
		OpenGLBuffer(Type::Array), m_mode(VertexBuffer::DrawMode::Triangles),
		m_glMode(GL_TRIANGLES), m_size(0)
	{
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{

	}

	void OpenGLVertexBuffer::setData(const void* vertices, size_t byteSize, const VertexFormat& format)
	{
		OpenGLBuffer::setData(vertices, byteSize);

		size_t size = byteSize / format.getByteSize();

		if (m_size != size || m_format != format)
			invalidate();

		m_format = format;
		m_size = size;
	}

	void OpenGLVertexBuffer::getData(void* vertices)
	{
		OpenGLBuffer::getData(vertices);
	}

	void* OpenGLVertexBuffer::map()
	{
		return OpenGLBuffer::map();
	}

	size_t OpenGLVertexBuffer::getSize() const
	{
		return m_size;
	}

	size_t OpenGLVertexBuffer::getByteSize() const
	{
		return m_format.getByteSize();
	}

	VertexFormat& OpenGLVertexBuffer::getFormat()
	{
		return m_format;
	}

	const VertexFormat& OpenGLVertexBuffer::getFormat() const
	{
		return m_format;
	}

	void OpenGLVertexBuffer::setDrawMode(VertexBuffer::DrawMode mode)
	{
		m_glMode = glDrawModes[static_cast<unsigned>(mode)];
	}

	unsigned OpenGLVertexBuffer::getVaoId(OpenGLContext *context, OpenGLShader *shader, OpenGLIndexBuffer *ibo, OpenGLVertexBuffer *instance)
	{
		VaoKey key(context, shader, ibo, instance);
		bool found = false;
		std::shared_ptr<OpenGLVertexArray> vao;

		size_t size = m_vaos.size();
		for (size_t i = 0; i < size; i++)
		{
			if (m_vaos[i].first == key)
			{
				if (m_vaos[i].second->isValid())
					return m_vaos[i].second->getId();

				vao = m_vaos[i].second;
				found = true;
			}
		}

		// The VAO doesn't exist or is invalid, we need to create it
		vao = std::make_shared<OpenGLVertexArray>(context, shader, this, ibo, instance);

		if (!found)
			m_vaos.push_back({ key, vao });

		return vao->getId();
	}

	unsigned OpenGLVertexBuffer::getOpenGLDrawMode() const
	{
		return m_glMode;
	}
}
