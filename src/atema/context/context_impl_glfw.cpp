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

#include <atema/context/context.hpp>

#ifdef ATEMA_CONTEXT_IMPL_GLFW

#include <atema/window/window.hpp>
#include <atema/core/error.hpp>

#define OPTS Window::options

namespace at
{
	Context::Context() :
		m_thread_active(false),
		m_active(false),
		m_flags(0),
		m_infos(),
		m_name(""),
		m_window(nullptr)
	{
		
	}
	
	Context::~Context() noexcept
	{
		destroy_window();
	}

	bool Context::is_valid() const noexcept
	{
		return ((m_window) && (glfwWindowShouldClose(m_window) == 0));
	}
	
	void Context::create(unsigned int w, unsigned int h, const gl_version& version)
	{
		create(w, h, "", 0, version); //Invisible window
	}
	
	void Context::make_current(bool value)
	{
		if (value == is_current_context())
		{
			RenderTarget::make_current(value);
			return;
		}
		
		if (value)
		{
			glfwMakeContextCurrent(m_window);
			
			set_current(this);
		}
		else
		{
			glfwMakeContextCurrent(nullptr);
			
			set_current(nullptr);
		}
		
		RenderTarget::make_current(value);
	}
	
	//RenderTarget
	void Context::ensure_framebuffer_exists()
	{
		//Nothing to do in here
	}
	
	unsigned int Context::get_width() const
	{
		return (unsigned int) (m_infos.x2);
	}
	
	unsigned int Context::get_height() const
	{
		return (unsigned int) (m_infos.y2);
	}
	
	GLuint Context::get_gl_framebuffer_id() const
	{
		return (0);
	}
	
	GLuint Context::get_gl_id() const noexcept
	{
		return (0);
	}
	
	//--------------------
	//SPECIFIC GLFW
	void Context::create(unsigned int w, unsigned int h, const char *name, Flags flag_list, const gl_version& version)
	{
		try
		{
			static bool glfw_init_done = false;
			
			int bw = 0;
			int bh = 0;
			
			Rect viewport;
			
			Flags tmp_flags = 0;
			
			if ((w == 0) || (h == 0))
			{
				ATEMA_ERROR("Values for width and height must be greater than 0.")
			}
			
			if (!glfw_init_done && !glfwInit())
			{
				ATEMA_ERROR("GLFW init failed.")
			}
			
			glfwDefaultWindowHints();
			
			// OpenGL version
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, version.major);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, version.minor);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			
			// Anti-Aliasing
			glfwWindowHint(GLFW_SAMPLES, 0);
			
			m_flags = 0;
			
			// Options & Creation
			tmp_flags |= (flag_list & OPTS::autoscale);
			
			if (flag_list & OPTS::fullscreen)
			{
				int nb_vid = 0;
				const GLFWvidmode* vid_modes = nullptr;
				GLFWmonitor* monitor = glfwGetPrimaryMonitor();
				
				if (!monitor)
				{
					ATEMA_ERROR("No monitor available.")
				}
				
				vid_modes = glfwGetVideoModes(monitor, &nb_vid);
				
				if (nb_vid <= 0 || !vid_modes)
				{
					ATEMA_ERROR("No video mode available.")
				}
				
				for(int i = 0; i < nb_vid; i++)
				{
					GLFWvidmode vm = vid_modes[i];
					
					if (w == static_cast<unsigned>(vm.width) || h == static_cast<unsigned>(vm.height))
						break;
					else if (i == nb_vid-1)
					{
						ATEMA_ERROR("Invalid video mode.")
					}
				}
				
				tmp_flags |= OPTS::fullscreen;
				
				m_window = glfwCreateWindow( static_cast<int>(w), static_cast<int>(h), name ? name : " ", monitor, nullptr );
			}
			else
			{
				tmp_flags |= (flag_list & OPTS::resizable);
				tmp_flags |= (flag_list & OPTS::visible);
				tmp_flags |= (flag_list & OPTS::autoscale);
				tmp_flags |= (flag_list & OPTS::frame);
				
				glfwWindowHint(GLFW_VISIBLE, flag_list & OPTS::visible ? GL_TRUE : GL_FALSE);
				
				glfwWindowHint(GLFW_DECORATED, flag_list & OPTS::frame ? GL_TRUE : GL_FALSE);
				
				glfwWindowHint(GLFW_RESIZABLE, flag_list & OPTS::resizable ? GL_TRUE : GL_FALSE);
				
				m_window = glfwCreateWindow( static_cast<int>(w), static_cast<int>(h), name ? name : " ", nullptr, nullptr );
			}
			
			if (!m_window)
			{
				ATEMA_ERROR("GLFW could not create window.")
			}
			
			glfwMakeContextCurrent(m_window);
			
			if (!gladLoadGL())
			{
				ATEMA_ERROR("glad init failed.")
			}
			
			tmp_flags |= flag_list & OPTS::vsync;
			if (flag_list & OPTS::vsync)
				glfwSwapInterval(1);
			else
				glfwSwapInterval(0);
			
			glfwDefaultWindowHints();
			
			m_flags = tmp_flags;
			
			if (m_flags & OPTS::autoscale)
			{
				viewport.x1 = 0;
				viewport.y1 = 0;
				viewport.x2 = w-1;
				viewport.y2 = h-1;
			}
			else
			{
				viewport.x1 = 0;
				viewport.y1 = 0;
				viewport.x2 = 0;
				viewport.y2 = 0;
			}
			
			glfwGetWindowSize(m_window, &bw, &bh);
			
			m_infos.x2 = bw;
			m_infos.y2 = bh;
			
			if (name)
				m_name = name;
			
			if ((m_infos.x2 != static_cast<int>(w)) || (m_infos.y2 != static_cast<int>(h)))
			{
				ATEMA_ERROR("Window size not available.")
			}
			
			set_viewport(Rect(0, 0, bw-1, bh-1));
			
			glfwGetWindowPos(m_window, &m_infos.x1, &m_infos.y1);
			
			init_gl_states();
			// register_window();
			
			make_current(true);
		}
		catch (Error& e)
		{
			destroy_window();
			
			throw;
			// ATEMA_ERROR("Creation failed.")
		}
	}
	
	void Context::destroy_window() noexcept
	{
		if (m_window)
			glfwDestroyWindow(m_window);
		
		m_window = nullptr;
	}
}

#endif