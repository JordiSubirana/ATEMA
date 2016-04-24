#include <atema/window/window.hpp>

#ifdef ATEMA_WINDOW_IMPL_GLFW

#include <atema/core/error.hpp>

namespace at
{
	window::window()
	{
		
	}
	
	window::~window() noexcept
	{
		
	}
	
	void window::create(unsigned int w, unsigned int h, const char *name, flags flag_list, const context::gl_version& version)
	{
		context::create(w, h, name, flag_list, version);
	}
	
	void window::create(int x, int y, unsigned int w, unsigned int h, const char *name, flags flag_list, const context::gl_version& version)
	{
		create(w, h, name, flag_list, version);
		
		set_position(x, y);
	}
	
	void window::set_position(int x, int y)
	{
		if (!is_valid())
			ATEMA_ERROR("Window is not initialized.")
		
		glfwSetWindowPos(m_window, x, y);
		
		glfwGetWindowPos(m_window, &m_infos.x, &m_infos.y);
		
		if ((m_infos.x != x) || (m_infos.y != y))
			ATEMA_ERROR("Position was not properly set.")
	}
	
	window::operator bool() const noexcept
	{
		return (is_valid());
	}
	
	void window::update()
	{
		if (!is_valid())
			ATEMA_ERROR("Window is not initialized.")
		
		//Be careful, this rect is working like x1,y1,x2,y2, not x,y,w,h
		rect tmp_rect;
		
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
				tmp_rect.w = static_cast<float>(tmp_rect.h) * ratio;
			}
			else if (scale < ratio)
			{
				tmp_rect.w = m_infos.w;
				tmp_rect.h = static_cast<float>(tmp_rect.w) / ratio;
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
		
		activate(true);
		
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
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
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		glfwSwapBuffers(m_window);
		
		glfwPollEvents();
	}
	
	//PRIVATE
	GLFWwindow* window::get_glfw_window() const noexcept
	{
		return (m_window);
	}
}

#endif