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

#include <Atema/Renderer/Window.hpp>
#include <Atema/Core/Error.hpp>

#if defined(ATEMA_SYSTEM_WINDOWS)
	#include "WindowImpl_Win32.hpp"
#else
	#error Operating system not supported by Renderer module
#endif

namespace at
{
	//STATIC
	Ref<Window> Window::create(unsigned int w, unsigned int h, const std::string& name, const Window::Style& style, const Context::Settings& settings, Renderer *renderer)
	{
		if (!renderer)
		{
			renderer = Renderer::get_current();
			
			if (!renderer)
				ATEMA_ERROR("No Renderer is currently active.")
		}
		
		Ref<Window> window = new (std::nothrow) Window();
		
		if (!window)
			ATEMA_ERROR("Allocation of Window failed.")
		
		window->reset(w, h, name, style);
		
		Ref<Context> context = Context::create(window->get_handle(), settings, renderer);
		
		if (!context)
			ATEMA_ERROR("Context creation failed.")
		
		window->m_context = context;
		
		return (window);
	}
	
	Ref<Window> Window::create(int x, int y, unsigned int w, unsigned int h, const std::string& name, const Window::Style& style, const Context::Settings& settings, Renderer *renderer)
	{
		Ref<Window> window = Window::create(w, h, name, style, settings, renderer);
		
		window->set_position(x, y);
		
		return (window);
	}
	
	//PRIVATE
	Window::Window()
	{
		m_impl = new Window::Impl();
	}
	
	//PUBLIC
	Window::~Window()
	{
		delete m_impl;
	}
	
	void Window::reset(unsigned int w, unsigned int h, const std::string& name, const Window::Style& style)
	{
		m_impl->reset(w, h, name, style);
	}
	
	void Window::reset(int x, int y, unsigned int w, unsigned int h, const std::string& name, const Window::Style& style)
	{
		m_impl->reset(w, h, name, style);
		m_impl->set_position(x, y);
	}
	
	void Window::set_position(int x, int y)
	{
		m_impl->set_position(x, y);
	}
	
	void Window::set_title(const std::string& title)
	{
		m_impl->set_title(title);
	}
	
	void Window::close()
	{
		m_impl->close();
	}
	
	bool Window::should_close() const noexcept
	{
		return (m_impl->should_close());
	}
	
	Window::operator bool() const noexcept
	{
		return (!should_close());
	}
	
	void Window::process_events()
	{
		m_impl->process_events();
	}
	
	void Window::update()
	{
		m_context->swap_buffers();
		process_events();
	}
	
	WindowHandle Window::get_handle()
	{
		return (m_impl->get_handle());
	}
	
	const WindowHandle Window::get_handle() const
	{
		return (m_impl->get_handle());
	}
}