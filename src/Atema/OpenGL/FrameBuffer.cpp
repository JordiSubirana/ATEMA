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

#include <Atema/OpenGL/FrameBuffer.hpp>
#include <Atema/Core/Error.hpp>
#include <glad/glad.h>

namespace at
{
	OpenGLFrameBuffer::OpenGLFrameBuffer() : m_id(0), m_context(nullptr)
	{
	}

	OpenGLFrameBuffer::~OpenGLFrameBuffer()
	{
	}

	void OpenGLFrameBuffer::initialize(bool isDefault)
	{
		m_context = OpenGLContext::getCurrent();

		if (!m_context)
			ATEMA_ERROR("No OpenGL context available.");

		int max = 0;
		glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max);

		m_drawBuffers.resize(max);

		if (!m_drawBuffers.size())
			ATEMA_ERROR("No available color attachments.");

		for (auto& attachment : m_drawBuffers)
			attachment = GL_NONE;

		if (!isDefault)
			glGenFramebuffers(1, &m_id);

		m_viewport = { 0, 0, 0, 0 };
	}

	unsigned OpenGLFrameBuffer::getId() const
	{
		return m_id;
	}

	OpenGLContext* OpenGLFrameBuffer::getContext()
	{
		return m_context;
	}

	void OpenGLFrameBuffer::bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_id);
		glDrawBuffers(static_cast<GLsizei>(m_drawBuffers.size()), m_drawBuffers.data());
		glViewport(m_viewport.x, m_viewport.y, m_viewport.w, m_viewport.h);
	}

	void OpenGLFrameBuffer::setDrawArea(int x, int y, unsigned w, unsigned h)
	{
		m_viewport = { x, y, static_cast<int>(w), static_cast<int>(h) };
	}

	void OpenGLFrameBuffer::clearColor(const Color& color, int index)
	{
		GLenum attachment = GL_COLOR_ATTACHMENT0 + index;

		if (index >= 0 && !isValid(index))
			return;

		glBindFramebuffer(GL_FRAMEBUFFER, m_id);
		glDrawBuffers(1, index < 0 ? m_drawBuffers.data() : &attachment);

		glClearColor(color.r, color.g, color.b, color.a);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void OpenGLFrameBuffer::clearColor(const Color& color)
	{
		clearColor(color, -1);
	}

	void OpenGLFrameBuffer::clearDepth()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_id);
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	bool OpenGLFrameBuffer::isValid(int index) const
	{
		return m_drawBuffers[index] != GL_NONE;
	}
}
