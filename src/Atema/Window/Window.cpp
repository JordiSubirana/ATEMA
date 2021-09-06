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

#include <Atema/Window/Window.hpp>
#include <Atema/Core/Error.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifdef ATEMA_SYSTEM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#include <functional>

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

	void setResizedCallback(const std::function<void(unsigned, unsigned)>& callback)
	{
		m_resizedCallback = callback;
	}

	void create(const Window::Settings& description)
	{
		destroy(); // Ensure there is no previous window

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, description.resizable ? GLFW_TRUE : GLFW_FALSE);
		
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

		glfwSetWindowUserPointer(m_window, this);

		glfwSetFramebufferSizeCallback(m_window, onFramebufferResized);

		int w, h;
		glfwGetWindowSize(m_window, &w, &h);

		m_resizedCallback(static_cast<unsigned>(w), static_cast<unsigned>(h));
	}

	void destroy()
	{
		if (m_window)
		{
			glfwDestroyWindow(m_window);

			m_window = nullptr;
		}
	}

	void* getHandle() const
	{
#ifdef ATEMA_SYSTEM_WINDOWS
		HWND handle = glfwGetWin32Window(m_window);
		
		return handle;
#endif

		return nullptr;
	}

	void setTitle(const std::string& title)
	{
		glfwSetWindowTitle(m_window, title.c_str());
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

	void framebufferResized(int width, int height)
	{
		m_resizedCallback(static_cast<unsigned>(width), static_cast<unsigned>(height));
	}

	static void onFramebufferResized(GLFWwindow* window, int width, int height)
	{
		auto w = reinterpret_cast<Window::Implementation*>(glfwGetWindowUserPointer(window));

		w->framebufferResized(width, height);
	}

private:
	GLFWwindow* m_window;
	std::function<void(unsigned, unsigned)> m_resizedCallback;
};

// Window
Window::Window()
{
	m_implementation.reset(new Window::Implementation());
	m_implementation->setResizedCallback([this](unsigned int w, unsigned int h)
		{
			resizedCallback(w, h);
		});
}

Window::~Window()
{
}

Ptr<Window> Window::create(const Window::Settings& description)
{
	Ptr<Window> window(new Window());

	window->initialize(description);
	
	return window;
}

void Window::setTitle(const std::string& title)
{
	m_implementation->setTitle(title);
}

void Window::initialize(const Settings& description)
{
	m_implementation->create(description);
}

void Window::resizedCallback(unsigned width, unsigned height)
{
	m_size.x = width;
	m_size.y = height;
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

Vector2u Window::getSize() const noexcept
{
	return m_size;
}

void* Window::getHandle() const
{
	return m_implementation->getHandle();
}

const std::vector<const char*>& Window::getVulkanExtensions()
{
	static std::vector<const char*> requiredExtensions;

	if (requiredExtensions.empty())
	{
		uint32_t requiredExtensionCount = 0;
		auto requiredExtensionNames = glfwGetRequiredInstanceExtensions(&requiredExtensionCount);

		for (uint32_t i = 0; i < requiredExtensionCount; i++)
			requiredExtensions.push_back(requiredExtensionNames[i]);
	}

	return requiredExtensions;
}
