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

#include <Atema/Renderer/Window.hpp>
#include <Atema/Renderer/RenderSystem.hpp>

namespace at
{
	Window::Implementation::Implementation()
	{
	}

	Window::Window(unsigned width, unsigned height, const std::string& title, RenderSystem* system) :
		m_impl(nullptr)
	{
		if (!system)
			system = RenderSystem::getCurrent();
		
		if (!system)
			ATEMA_ERROR("Invalid RenderSystem.");

		m_impl = system->createWindow();
		m_abstractImpl = m_impl;

		m_impl->setTitle(title);
		m_impl->setSize(width, height);
	}

	Window::~Window()
	{
		delete m_impl;
	}

	void Window::setPosition(int x, int y)
	{
		m_impl->setPosition(x, y);
	}

	void Window::setSize(unsigned width, unsigned height)
	{
		m_impl->setSize(width, height);
	}

	void Window::setTitle(const std::string& title)
	{
		m_impl->setTitle(title);
	}

	void Window::close()
	{
		m_impl->close();
	}

	bool Window::shouldClose() const noexcept
	{
		return m_impl->shouldClose();
	}

	void Window::processEvents()
	{
		m_impl->processEvents();
	}

	void Window::swapBuffers()
	{
		m_impl->swapBuffers();
	}

	WindowHandle Window::getHandle()
	{
		return m_impl->getHandle();
	}

	const WindowHandle Window::getHandle() const
	{
		return m_impl->getHandle();
	}

	void Window::setDrawArea(int x, int y, unsigned w, unsigned h)
	{
		m_impl->setDrawArea(x, y, w, h);
	}

	void Window::clearColor(const Color& color)
	{
		m_impl->clearColor(color);
	}

	void Window::clearDepth()
	{
		m_impl->clearDepth();
	}
}
