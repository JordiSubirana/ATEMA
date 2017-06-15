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

#include <Atema/OpenGL/RenderTarget.hpp>
#include <Atema/OpenGL/Texture.hpp>
#include <Atema/Core/Error.hpp>

#include <glad/glad.h>

namespace at
{
	OpenGLRenderTarget::OpenGLRenderTarget()
	{
		initialize(false);
	}

	OpenGLRenderTarget::~OpenGLRenderTarget()
	{
		OpenGLRenderTarget::removeAttachments();

		getContext()->deleteFBO(getId());
	}

	int OpenGLRenderTarget::addAttachment(const Texture& texture, int index)
	{
		auto tex = cast<OpenGLTexture>(texture.getImplementation());

		if (index == -1)
			index = getFirstAvailable();

		if (isValid(index))
			removeAttachment(index);

		glBindFramebuffer(GL_FRAMEBUFFER, getId());
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, tex->getId(), 0);

		m_drawBuffers[index] = GL_COLOR_ATTACHMENT0 + index;

		return index;
	}

	int OpenGLRenderTarget::addAttachment(unsigned width, unsigned height, int index)
	{
		if (index == -1)
			index = getFirstAvailable();

		if (isValid(index))
			removeAttachment(index);

		GLuint rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA32F, width, height);

		glBindFramebuffer(GL_FRAMEBUFFER, getId());
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_RENDERBUFFER, rbo);

		m_drawBuffers[index] = GL_COLOR_ATTACHMENT0 + index;

		m_rbos[index] = rbo;

		return index;
	}

	void OpenGLRenderTarget::removeAttachment(int index)
	{
		auto rboIt = m_rbos.find(index);

		m_drawBuffers[index] = GL_NONE;

		glBindFramebuffer(GL_FRAMEBUFFER, getId());

		if (rboIt != m_rbos.end()) // RenderBuffer
		{
			unsigned rbo = rboIt->second;

			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_RENDERBUFFER, 0);

			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glDeleteFramebuffers(1, &rbo);

			m_rbos.erase(rboIt);
		}
		else // Texture
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, 0, 0);
		}
	}

	void OpenGLRenderTarget::removeAttachments()
	{
		for (unsigned i = 0; i < m_drawBuffers.size(); i++)
			removeAttachment(i);
	}

	void OpenGLRenderTarget::setDrawArea(int x, int y, unsigned w, unsigned h)
	{
		OpenGLFrameBuffer::setDrawArea(x, y, w, h);
	}

	void OpenGLRenderTarget::clearColor(const Color& color, int index)
	{
		OpenGLFrameBuffer::clearColor(color, index);
	}

	void OpenGLRenderTarget::clearColor(const Color& color)
	{
		OpenGLFrameBuffer::clearColor(color);
	}

	void OpenGLRenderTarget::clearDepth()
	{
		OpenGLFrameBuffer::clearDepth();
	}

	bool OpenGLRenderTarget::isValid(int index) const
	{
		return OpenGLFrameBuffer::isValid(index);
	}

	int OpenGLRenderTarget::getFirstAvailable() const
	{
		int count = static_cast<int>(m_drawBuffers.size());

		for (int i = 0; i < count; i++)
		{
			if (m_drawBuffers[i] != GL_NONE)
				return i;
		}

		ATEMA_ERROR("No more attachments available.");

		return -1;
	}
}
