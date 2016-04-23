#ifndef ATEMA_WINDOW_IMPL_GLFW_HEADER
#define ATEMA_WINDOW_IMPL_GLFW_HEADER

#include "window_impl.hpp"

#include "internal_config.hpp"

#ifdef ATEMA_WINDOW_IMPL_GLFW

// Ensure that glad is included before GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace at
{
	class window_impl_glfw : public window_impl
	{
		public:
			window_impl_glfw();
			virtual ~window_impl_glfw() noexcept;
			
			void create(const context::gl_version& version);
			
			void activate();
			bool is_active() const noexcept;
			
			void create(unsigned int w, unsigned int h, const char *name, flags flag_list, const context::gl_version& version);
			void create(int x, int y, unsigned int w, unsigned int h, const char *name, flags flag_list, const context::gl_version& version);
			
		private:
			GLFWwindow *m_window;
			bool m_active;
	};
}

#endif

#endif