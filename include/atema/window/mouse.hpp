#ifndef ATEMA_MOUSE_HEADER
#define ATEMA_MOUSE_HEADER

#include <atema/window/config.hpp>
#include <atema/window/window.hpp>

#if defined(ATEMA_WINDOW_IMPL_GLFW)
	
	#define ATEMA_MOUSE_IMPL_GLFW
	
#endif

namespace at
{
	class ATEMA_WINDOW_API mouse
	{
		public:
			enum class button
			{
				left = GLFW_MOUSE_BUTTON_LEFT,
				middle = GLFW_MOUSE_BUTTON_MIDDLE,
				right = GLFW_MOUSE_BUTTON_RIGHT
			};
			
		//To implement for each OS
		public:
			mouse();
			mouse(const window& window);
			~mouse();
			
			void set_window(const window& window);
			
			bool is_pressed(button b) const;
			
			bool is_enabled() const;
			void disable();
			void enable();
			
			double get_x() const;
			double get_y() const;
			
		//OS specific
		#if defined(ATEMA_MOUSE_IMPL_GLFW)
		private:
			GLFWwindow *m_window;
			bool m_enabled;
			
		#endif
	};
}

#endif