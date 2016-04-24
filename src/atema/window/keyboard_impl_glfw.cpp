#include <atema/window/keyboard.hpp>

#ifdef ATEMA_KEYBOARD_IMPL_GLFW

#include <atema/core/error.hpp>

namespace at
{
	Keyboard::Keyboard() :
		m_window(nullptr)
	{
		
	}
	
	Keyboard::Keyboard(const Window& window) :
		m_window(window.get_glfw_window())
	{
		if (!m_window)
			ATEMA_ERROR("Invalid window.")
	}
	
	Keyboard::~Keyboard()
	{
		
	}
	
	void Keyboard::set_window(const Window& window)
	{
		if (!window.get_glfw_window())
			ATEMA_ERROR("Invalid window.")
		
		m_window = window.get_glfw_window();
	}
	
	bool Keyboard::is_pressed(key k) const
	{
		if (!m_window)
			ATEMA_ERROR("Invalid window.")
		
		return (glfwGetKey(m_window, static_cast<int>(k)) == GLFW_PRESS);
	}
}

#endif