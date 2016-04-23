#ifdef ATEMA_WINDOW_IMPL_GLFW

#include "window_impl_glfw.hpp"

#include <atema/core/error.hpp>

#define OPTS window::options

namespace at
{
	window_impl_glfw::window_impl_glfw() :
		m_window(nullptr),
		m_active(false)
	{
		
	}
	
	window_impl_glfw::~window_impl_glfw() noexcept
	{
		
	}
	
	void window_impl_glfw::create(const context::gl_version& version)
	{
		
	}
	
	void window_impl_glfw::activate()
	{
		
	}
	
	bool window_impl_glfw::is_active() const noexcept
	{
		
	}
	
	void window_impl_glfw::create(unsigned int w, unsigned int h, const char *name, flags flag_list, const context::gl_version& version)
	{
		try
		{
			static bool glfw_init_done = false;
			flags tmp_flags = 0;
			
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
			tmp_flags |= (flag_list & OPTS.autoscale);
			
			if (flag_list & OPTS.fullscreen)
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
				
				tmp_flags |= OPTS.fullscreen;
				
				m_window = glfwCreateWindow( static_cast<int>(w), static_cast<int>(h), name ? name : " ", monitor, window );
			}
			else
			{
				tmp_flags |= (flag_list & OPTS.resizable);
				tmp_flags |= (flag_list & OPTS.visible);
				tmp_flags |= (flag_list & OPTS.autoscale);
				tmp_flags |= (flag_list & OPTS.frame);
				
				glfwWindowHint(GLFW_VISIBLE, flag_list & OPTS.visible ? GL_TRUE : GL_FALSE);
				
				glfwWindowHint(GLFW_DECORATED, flag_list & OPTS.frame ? GL_TRUE : GL_FALSE);
				
				glfwWindowHint(GLFW_RESIZABLE, flag_list & OPTS.resizable ? GL_TRUE : GL_FALSE);
				
				m_window = glfwCreateWindow( static_cast<int>(w), static_cast<int>(h), name ? name : " ", nullptr, window );
			}
			
			if (!m_window)
			{
				ATEMA_ERROR("GLFW could not create window.")
			}
			
			glfwMakeContextCurrent(m_window);	
			
			glewExperimental = true; // Needed in core profile
			
			if (gladLoadGL())
			{
				glfwDestroyWindow(m_window);
				
				ATEMA_ERROR("glad init failed.")
			}
			
			tmp_flags |= flag_list & options.vsync;
			if (flag_list & options.vsync)
				glfwSwapInterval(1);
			else
				glfwSwapInterval(0);
			
			glfwDefaultWindowHints();
			
			m_opts = tmp_flags;
			
			if (m_opts & options.autoscale)
				m_rect = {0, 0, static_cast<int>(w), static_cast<int>(h)};
			else
				m_rect = {0, 0, 0, 0};
			
			glfwGetWindowSize(m_window, &bw, &bh);
			
			m_width = static_cast<unsigned>(bw);
			m_height = static_cast<unsigned>(bh);
			
			if (name)
				m_name = name;
			
			if ((m_width != w) || (m_height != h))
			{
				ATEMA_ERROR("Window size not available.")
			}
			
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			glEnable(GL_TEXTURE_2D);
			glActiveTexture(GL_TEXTURE0);
			
			glfwGetWindowPos(m_window, &m_x, &m_y);
			
			// register_window();
		}
		catch (...)
		{
			ATEMA_ERROR("Creation failed.")
		}
	}
	
	void window_impl_glfw::create(int x, int y, unsigned int w, unsigned int h, const char *name, flags flag_list, const context::gl_version& version)
	{
		
	}
}

#endif