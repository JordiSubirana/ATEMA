#include <atema/window/keyboard.hpp>

#ifdef ATEMA_KEYBOARD_IMPL_GLFW

#include <atema/core/error.hpp>

namespace at
{
	keyboard::keyboard() :
		m_window(nullptr)
	{
		
	}
	
	keyboard::keyboard(const window& window) :
		m_window(window.get_glfw_window())
	{
		if (!m_window)
			ATEMA_ERROR("Invalid window.")
	}
	
	keyboard::~keyboard()
	{
		
	}
	
	void keyboard::set_window(const window& window)
	{
		if (!window.get_glfw_window())
			ATEMA_ERROR("Invalid window.")
		
		m_window = window.get_glfw_window();
	}
	
	bool keyboard::is_pressed(key k) const
	{
		if (!m_window)
			ATEMA_ERROR("Invalid window.")
		
		return (glfwGetKey(m_window, static_cast<int>(k)) == GLFW_PRESS);
	}
}

#endif