/*
	Copyright 2021 Jordi SUBIRANA

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

#include <Atema/Core/Window.hpp>
#include <Atema/Core/Error.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

using namespace at;

// Implementation
namespace
{
	unsigned s_windowCount = 0;
}

class Window::Implementation
{
public:
	Implementation() : m_window(nullptr)
	{
		if (s_windowCount == 0)
		{
			if (!glfwInit())
			{
				ATEMA_ERROR("GLFW initialization failed");
			}
		}

		s_windowCount++;
	}
	
	virtual ~Implementation()
	{
		destroy(); // Ensure the window was deleted

		s_windowCount--;

		if (s_windowCount == 0)
			glfwTerminate();
	}

	void create(const Window::Description& description)
	{
		destroy(); // Ensure there is no previous window

		m_window = glfwCreateWindow(
			description.width,
			description.height,
			description.title.c_str(),
			nullptr,
			nullptr);

		if (!m_window)
		{
			ATEMA_ERROR("Window creation failed");
		}
	}

	void destroy()
	{
		if (m_window)
		{
			glfwDestroyWindow(m_window);

			m_window = nullptr;
		}
	}

	bool shouldClose() const noexcept
	{
		return glfwWindowShouldClose(m_window);
	}

	void processEvents()
	{
		glfwPollEvents();
	}

	void swapBuffers()
	{
		glfwSwapBuffers(m_window);
	}

private:
	GLFWwindow* m_window;
};

// Window
Window::Window()
{
	m_implementation.reset(new Window::Implementation());
}

Window::~Window()
{
}

Ptr<Window> Window::create(const Window::Description& description)
{
	Ptr<Window> window(new Window());

	window->initialize(description);
	
	return window;
}

void Window::initialize(const Description& description)
{
	m_implementation->create(description);
}

bool Window::shouldClose() const noexcept
{
	return m_implementation->shouldClose();
}

void Window::processEvents()
{
	m_implementation->processEvents();
}

void Window::swapBuffers()
{
	m_implementation->swapBuffers();
}
