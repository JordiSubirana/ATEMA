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

#include <Atema/OpenGL/Renderer.hpp>
#include <Atema/OpenGL/IndexBuffer.hpp>
#include <Atema/OpenGL/RenderTarget.hpp>
#include <Atema/OpenGL/Shader.hpp>
#include <Atema/OpenGL/VertexBuffer.hpp>
#include <Atema/OpenGL/Window.hpp>
#include <glad/glad.h>

namespace at
{
	OpenGLRenderer::OpenGLRenderer() :
		m_target(nullptr), m_shader(nullptr), m_vbo(nullptr), m_ibo(nullptr), m_instanceData(nullptr),
		m_context(nullptr), m_safe(0), m_updateVao(false), m_vao(0)
	{
	}

	OpenGLRenderer::~OpenGLRenderer()
	{
	}

	void OpenGLRenderer::setTarget(RenderTarget* target)
	{
		auto tmp = target ? cast<OpenGLRenderTarget>(target->getImplementation()) : nullptr;

		setTarget(static_cast<OpenGLFrameBuffer*>(tmp));
	}

	void OpenGLRenderer::setTarget(Window* window)
	{
		auto tmp = window ? cast<OpenGLWindow>(window->getImplementation()) : nullptr;

		setTarget(static_cast<OpenGLFrameBuffer*>(tmp));
	}

	void OpenGLRenderer::setShader(Shader* shader)
	{
		auto tmp = shader ? cast<OpenGLShader>(shader->getImplementation()) : nullptr;

		if (tmp != m_shader)
		{
			m_shader = tmp;

			if (m_safe && m_shader)
				m_shader->bind();

			m_updateVao = true;
		}
	}

	void OpenGLRenderer::setVertexBuffer(VertexBuffer* vertexBuffer)
	{
		auto tmp = vertexBuffer ? cast<OpenGLVertexBuffer>(vertexBuffer->getImplementation()) : nullptr;

		if (tmp != m_vbo)
		{
			m_vbo = tmp;

			m_updateVao = true;
		}
	}

	void OpenGLRenderer::setIndexBuffer(IndexBuffer* indexBuffer)
	{
		auto tmp = indexBuffer ? cast<OpenGLIndexBuffer>(indexBuffer->getImplementation()) : nullptr;

		if (tmp != m_ibo)
		{
			m_ibo = tmp;

			m_updateVao = true;
		}
	}

	void OpenGLRenderer::setInstanceBuffer(VertexBuffer* instanceData)
	{
		auto tmp = instanceData ? cast<OpenGLVertexBuffer>(instanceData->getImplementation()) : nullptr;

		if (tmp != m_instanceData)
		{
			m_instanceData = tmp;

			m_updateVao = true;
		}
	}

	void OpenGLRenderer::draw()
	{
		if (!m_vbo || !m_target || !m_context)
			return;

		if (!m_safe)
		{
			m_shader->bind();
			m_target->bind();
		}

		if (m_updateVao || !m_safe)
		{
			m_vao = m_vbo->getVaoId(m_context, m_shader, m_ibo, m_instanceData);
			glBindVertexArray(m_vao);
			m_updateVao = false;
		}

		auto drawMode = m_vbo->getOpenGLDrawMode();

		m_vbo->unmap();

		if (m_instanceData)
		{
			m_instanceData->unmap();

			if (m_ibo)
				glDrawElementsInstanced(drawMode, m_ibo->getSize(), m_ibo->getOpenGLType(), nullptr, m_instanceData->getSize());
			else
				glDrawArraysInstanced(drawMode, 0, m_vbo->getSize(), m_instanceData->getSize());
		}
		else
		{
			if (m_ibo)
				glDrawElements(drawMode, m_ibo->getSize(), m_ibo->getOpenGLType(), nullptr);
			else
				glDrawArrays(drawMode, 0, m_vbo->getSize());
		}
	}

	void OpenGLRenderer::draw(VertexBuffer& vertexBuffer)
	{
		auto vbo = cast<OpenGLVertexBuffer>(vertexBuffer.getImplementation());
		draw(vbo, nullptr, nullptr);
	}

	void OpenGLRenderer::draw(VertexBuffer& vertexBuffer, VertexBuffer& instanceData)
	{
		auto vbo = cast<OpenGLVertexBuffer>(vertexBuffer.getImplementation());
		auto instances = cast<OpenGLVertexBuffer>(instanceData.getImplementation());
		draw(vbo, nullptr, instances);
	}

	void OpenGLRenderer::draw(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer)
	{
		auto vbo = cast<OpenGLVertexBuffer>(vertexBuffer.getImplementation());
		auto ibo = cast<OpenGLIndexBuffer>(indexBuffer.getImplementation());
		draw(vbo, ibo, nullptr);
	}

	void OpenGLRenderer::draw(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, VertexBuffer& instanceData)
	{
		auto vbo = cast<OpenGLVertexBuffer>(vertexBuffer.getImplementation());
		auto ibo = cast<OpenGLIndexBuffer>(indexBuffer.getImplementation());
		auto instances = cast<OpenGLVertexBuffer>(instanceData.getImplementation());
		draw(vbo, ibo, instances);
	}

	void OpenGLRenderer::preDraw()
	{
		m_safe++;

		if (m_target)
			m_target->bind();

		if (m_shader)
			m_shader->bind();
	}

	void OpenGLRenderer::postDraw()
	{
		glBindVertexArray(0);

		m_safe--;
	}

	void OpenGLRenderer::setTarget(OpenGLFrameBuffer* framebuffer)
	{
		if (framebuffer != m_target)
		{
			m_target = framebuffer;

			if (m_target)
			{
				m_context = m_target->getContext();

				if (m_safe && m_context)
					m_target->bind();
			}
			else
			{
				m_context = nullptr;
			}

			m_updateVao = true;
		}
	}

	void OpenGLRenderer::draw(OpenGLVertexBuffer *vbo, OpenGLIndexBuffer *ibo, OpenGLVertexBuffer *instanceData)
	{
		unsigned vao = vbo->getVaoId(m_target->getContext(), m_shader, ibo, instanceData);

		auto drawMode = vbo->getOpenGLDrawMode();

		vbo->unmap();

		m_shader->bind();
		m_target->bind();

		glBindVertexArray(vao);

		if (instanceData)
		{
			instanceData->unmap();

			if (ibo)
				glDrawElementsInstanced(drawMode, ibo->getSize(), ibo->getOpenGLType(), nullptr, instanceData->getSize());
			else
				glDrawArraysInstanced(drawMode, 0, vbo->getSize(), instanceData->getSize());
		}
		else
		{
			if (ibo)
				glDrawElements(drawMode, ibo->getSize(), ibo->getOpenGLType(), nullptr);
			else
				glDrawArrays(drawMode, 0, vbo->getSize());
		}
	}
}
