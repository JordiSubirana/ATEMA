#include <atema/window/mouse.hpp>

#ifdef ATEMA_MOUSE_IMPL_GLFW

#include <atema/core/error.hpp>

namespace at
{
	Mouse::Mouse() :
		m_window(nullptr),
		m_enabled(false)
	{
		
	}
	
	Mouse::Mouse(const Window& window) :
		m_window(window.get_glfw_window()),
		m_enabled(true)
	{
		if (!m_window)
			ATEMA_ERROR("Invalid window.")
	}
	
	Mouse::~Mouse()
	{
		
	}
	
	void Mouse::set_window(const Window& window)
	{
		if (!window.get_glfw_window())
			ATEMA_ERROR("Invalid window.")
		
		m_window = window.get_glfw_window();
	}
	
	bool Mouse::is_pressed(button b) const
	{
		if (!m_window)
			ATEMA_ERROR("Invalid window.")
		
		return (glfwGetMouseButton(m_window, static_cast<int>(b)) == GLFW_PRESS);
	}
	
	bool Mouse::is_enabled() const
	{
		return (m_enabled);
	}
	
	void Mouse::disable()
	{
		if (!m_window)
			ATEMA_ERROR("Invalid window.")
		
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		
		m_enabled = false;
	}
	
	void Mouse::enable()
	{
		if (!m_window)
			ATEMA_ERROR("Invalid window.")
		
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		
		m_enabled = true;
	}
	
	double Mouse::get_x() const
	{
		if (!m_window)
			ATEMA_ERROR("Invalid window.")
		
		double value = 0.0;
		glfwGetCursorPos(m_window, &value, nullptr);
		
		return (value);
	}
	
	double Mouse::get_y() const
	{
		if (!m_window)
			ATEMA_ERROR("Invalid window.")
		
		double value = 0.0;
		glfwGetCursorPos(m_window, nullptr, &value);
		
		return (value);
	}
}

#endif