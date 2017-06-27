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

#include <Atema/OpenGL/VertexArray.hpp>
#include <Atema/OpenGL/Context.hpp>
#include <Atema/OpenGL/IndexBuffer.hpp>
#include <Atema/OpenGL/Shader.hpp>
#include <Atema/OpenGL/VertexBuffer.hpp>
#include <glad/glad.h>

namespace at
{
	OpenGLVertexArray::OpenGLVertexArray(OpenGLContext *context, OpenGLShader *shader, OpenGLVertexBuffer *vbo, OpenGLIndexBuffer *ibo, OpenGLVertexBuffer *instanceData)
	{
		auto& attributes = shader->getVertexAttributes();
		auto& vertexFormat = vbo->getFormat();

		glGenVertexArrays(1, &m_id);
		glBindVertexArray(m_id);

		if (ibo)
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo->getId());

		vbo->unmap();

		glBindBuffer(GL_ARRAY_BUFFER, vbo->getId());

		for (auto& attrib : attributes)
		{
			for (auto& vertexAttrib : vertexFormat)
			{
				if (!vertexAttrib.name.compare(attrib.name))
				{
					glEnableVertexAttribArray(attrib.location);

					glVertexAttribPointer(attrib.location, static_cast<GLint>(vertexAttrib.count()), attrib.glBaseType, GL_FALSE, static_cast<GLsizei>(vertexFormat.getByteSize()), (void*)vertexAttrib.getByteOffset());

					if (instanceData)
						glVertexAttribDivisor(attrib.location, 0);

					break;
				}
			}
		}

		if (instanceData)
		{
			auto& instanceFormat = instanceData->getFormat();

			instanceData->unmap();

			glBindBuffer(GL_ARRAY_BUFFER, instanceData->getId());

			for (auto& attrib : attributes)
			{
				for (auto& vertexAttrib : instanceFormat)
				{
					if (!vertexAttrib.name.compare(attrib.name))
					{
						glEnableVertexAttribArray(attrib.location);
						
						glVertexAttribPointer(attrib.location, static_cast<GLint>(vertexAttrib.count()), attrib.glBaseType, GL_FALSE, static_cast<GLsizei>(instanceFormat.getByteSize()), (void*)vertexAttrib.getByteOffset());

						glVertexAttribDivisor(attrib.location, 1);

						break;
					}
				}
			}
		}

		glBindVertexArray(0);

		m_valid = true;
		m_context = context;
		m_shader = shader;
		m_vbo = vbo;
		m_ibo = ibo;
		m_instanceData = instanceData;

		m_vbo->add(this);
		
		if (m_ibo)
			m_ibo->add(this);
		if (m_shader)
			m_shader->add(this);
		if (m_context)
			m_context->add(this);
		if (m_instanceData)
			m_instanceData->add(this);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		invalidate();
	}

	unsigned OpenGLVertexArray::getId() const
	{
		return m_id;
	}

	void OpenGLVertexArray::invalidate()
	{
		m_valid = false;

		if (m_id)
		{
			m_context->deleteVAO(m_id);
			m_id = 0;
		}

		m_vbo->remove(this);

		if (m_ibo)
			m_ibo->remove(this);
		if (m_shader)
			m_shader->remove(this);
		if (m_context)
			m_context->remove(this);
		if (m_instanceData)
			m_instanceData->remove(this);
	}

	bool OpenGLVertexArray::isValid() const
	{
		return m_valid;
	}
}
