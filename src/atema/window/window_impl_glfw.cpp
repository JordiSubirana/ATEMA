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

#include <atema/window/window.hpp>

#ifdef ATEMA_WINDOW_IMPL_GLFW

#include <atema/core/error.hpp>


namespace at
{
	Window::Window()
	{
		
	}
	
	Window::~Window() noexcept
	{
		
	}
	
	void Window::create(unsigned int w, unsigned int h, const char *name, Flags flag_list, const Context::gl_version& version)
	{
		Context::create(w, h, name, flag_list, version);
	}
	
	void Window::create(int x, int y, unsigned int w, unsigned int h, const char *name, Flags flag_list, const Context::gl_version& version)
	{
		create(w, h, name, flag_list, version);
		
		set_position(x, y);
	}
	
	void Window::set_position(int x, int y)
	{
		if (!is_valid())
			ATEMA_ERROR("Window is not initialized.")
		
		glfwSetWindowPos(m_window, x, y);
		
		glfwGetWindowPos(m_window, &m_infos.x1, &m_infos.y1);
		
		if ((m_infos.x1 != x) || (m_infos.y1 != y))
			ATEMA_ERROR("Position was not properly set.")
	}
	
	Window::operator bool() const noexcept
	{
		return (is_valid());
	}
	
	void Window::update()
	{
		if (!is_valid())
			ATEMA_ERROR("Window is not initialized.")
		/*
		//Be careful, this rect is working like x1,y1,x2,y2, not x,y,w,h
		Rect tmp_rect;
		
		if (m_flags & options::autoscale)
		{
			tmp_rect.x = 0;
			tmp_rect.y = 0;
			tmp_rect.w = m_infos.w-1;
			tmp_rect.h = m_infos.h-1;
		}
		else if (m_flags & options::adapt)
		{
			float scale = static_cast<float>(m_infos.w) / static_cast<float>(m_infos.h);
			float ratio = 1; //static_cast<float>(m_tex.get_width()) / static_cast<float>(m_tex.get_height());
			
			if (scale > ratio)
			{
				tmp_rect.h = m_infos.h;
				tmp_rect.w = (unsigned int) (static_cast<float>(tmp_rect.h) * ratio);
			}
			else if (scale < ratio)
			{
				tmp_rect.w = m_infos.w;
				tmp_rect.h = (unsigned int) (static_cast<float>(tmp_rect.w) / ratio);
			}
			else
			{
				tmp_rect.w = m_infos.w;
				tmp_rect.h = m_infos.h;
			}
			
			tmp_rect.x = m_infos.w/2 - tmp_rect.w/2;
			tmp_rect.y = m_infos.h/2 - tmp_rect.h/2;
			tmp_rect.w += tmp_rect.x;
			tmp_rect.h += tmp_rect.y;
		}
		else
		{
			// tmp_rect.x = m_infos.w/2 - m_tex.get_width()/2;
			// tmp_rect.y = m_infos.h/2 - m_tex.get_height()/2;
			// tmp_rect.w = m_infos.w/2 + m_tex.get_width()/2;
			// tmp_rect.h = m_infos.h/2 + m_tex.get_height()/2;
		}
		*/
		Context::make_current(true);
		
		// glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		// glEnable(GL_DEPTH_TEST);
		// glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		// glEnable(GL_TEXTURE_2D);
		// glActiveTexture(GL_TEXTURE0);
		
		/*
		glBindFramebuffer (GL_DRAW_FRAMEBUFFER, 0); // 0 = Window (default framebuffer)
		glDrawBuffer(GL_BACK);

		glBindFramebuffer (GL_READ_FRAMEBUFFER, m_render.get_gl_fbo());
		glReadBuffer(GL_COLOR_ATTACHMENT0);

		glBlitFramebuffer (0, 0, m_tex.get_width(), m_tex.get_height(),
						   tmp_rect.x, tmp_rect.y, tmp_rect.w, tmp_rect.h,
						   GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
						   GL_NEAREST);
		// */
		
		// glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		glfwSwapBuffers(m_window);
		
		glfwPollEvents();
	}
	
	//PRIVATE
	GLFWwindow* Window::get_glfw_window() const noexcept
	{
		return (m_window);
	}

	void Window::set_title(std::string title) {
		glfwSetWindowTitle(m_window, title.c_str());
	}
}

#endif