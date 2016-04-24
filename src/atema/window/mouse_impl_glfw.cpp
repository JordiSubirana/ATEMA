#include <atema/window/mouse.hpp>

#ifdef ATEMA_MOUSE_IMPL_GLFW

#include <atema/core/error.hpp>

namespace at
{
	mouse::mouse() :
		m_window(nullptr),
		m_enabled(false)
	{
		
	}
	
	mouse::mouse(const window& window) :
		m_window(window.get_glfw_window()),
		m_enabled(true)
	{
		if (!m_window)
			ATEMA_ERROR("Invalid window.")
	}
	
	mouse::~mouse()
	{
		
	}
	
	void mouse::set_window(const window& window)
	{
		if (!window.get_glfw_window())
			ATEMA_ERROR("Invalid window.")
		
		m_window = window.get_glfw_window();
	}
	
	bool mouse::is_pressed(button b) const
	{
		if (!m_window)
			ATEMA_ERROR("Invalid window.")
		
		return (glfwGetMouseButton(m_window, static_cast<int>(b)) == GLFW_PRESS);
	}
	
	bool mouse::is_enabled() const
	{
		return (m_enabled);
	}
	
	void mouse::disable()
	{
		if (!m_window)
			ATEMA_ERROR("Invalid window.")
		
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		
		m_enabled = false;
	}
	
	void mouse::enable()
	{
		if (!m_window)
			ATEMA_ERROR("Invalid window.")
		
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		
		m_enabled = true;
	}
	
	double mouse::get_x() const
	{
		if (!m_window)
			ATEMA_ERROR("Invalid window.")
		
		double value = 0.0;
		glfwGetCursorPos(m_window, &value, nullptr);
		
		return (value);
	}
	
	double mouse::get_y() const
	{
		if (!m_window)
			ATEMA_ERROR("Invalid window.")
		
		double value = 0.0;
		glfwGetCursorPos(m_window, nullptr, &value);
		
		return (value);
	}
}

#endif