// ----------------------------------------------------------------------
// Copyright (C) 2016 Jordi SUBIRANA
//
// This file is part of ATEMA.
//
// ATEMA is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ATEMA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ATEMA.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------

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