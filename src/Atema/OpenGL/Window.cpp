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

#include <Atema/OpenGL/Window.hpp>
#include <glad/glad.h>

#if defined ATEMA_SYSTEM_WINDOWS
#include "Win32/WindowImpl.hpp"
#else
#define ATEMA_OPENGL_OS_NOT_HANDLED
#endif

#ifdef ATEMA_OPENGL_OS_NOT_HANDLED
#define ATEMA_IMPL(at_call) ATEMA_ERROR("Operating System not handled for OpenGL Window.")
#else
#define ATEMA_IMPL(at_call) at_call
#endif

namespace at
{
	OpenGLWindow::OpenGLWindow() : m_impl(nullptr)
	{
		ATEMA_IMPL(m_impl = new Impl());
		
		initialize(true);
		m_drawBuffers[0] = GL_COLOR_ATTACHMENT0;
	}

	OpenGLWindow::~OpenGLWindow()
	{
		#ifndef ATEMA_OPENGL_OS_NOT_HANDLED
		delete m_impl;
		#endif
	}

	void OpenGLWindow::setPosition(int x, int y)
	{
		ATEMA_IMPL(m_impl->setPosition(x, y));
	}

	void OpenGLWindow::setSize(unsigned width, unsigned height)
	{
		ATEMA_IMPL(m_impl->setSize(width, height));
		setDrawArea(0, 0, width, height);
	}

	void OpenGLWindow::setTitle(const std::string& title)
	{
		ATEMA_IMPL(m_impl->setTitle(title));
	}

	void OpenGLWindow::close()
	{
		ATEMA_IMPL(m_impl->close());
	}

	bool OpenGLWindow::shouldClose() const
	{
		ATEMA_IMPL(return m_impl->shouldClose());
	}

	void OpenGLWindow::processEvents()
	{
		ATEMA_IMPL(m_impl->processEvents());
	}

	void OpenGLWindow::swapBuffers()
	{
		ATEMA_IMPL(m_impl->swapBuffers());
	}

	WindowHandle OpenGLWindow::getHandle()
	{
		ATEMA_IMPL(return m_impl->getHandle());
	}

	const WindowHandle OpenGLWindow::getHandle() const
	{
		ATEMA_IMPL(return m_impl->getHandle());
	}

	void OpenGLWindow::setDrawArea(int x, int y, unsigned w, unsigned h)
	{
		OpenGLFrameBuffer::setDrawArea(x, y, w, h);
	}

	void OpenGLWindow::clearColor(const Color& color)
	{
		OpenGLFrameBuffer::clearColor(color, 0);
	}

	void OpenGLWindow::clearDepth()
	{
		OpenGLFrameBuffer::clearDepth();
	}
}
