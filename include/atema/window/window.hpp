#ifndef ATEMA_WINDOW_HEADER
#define ATEMA_WINDOW_HEADER

#include <atema/window/config.hpp>
#include <atema/context/context.hpp>

#include <atema/utility/flags.hpp>

#include <string>

#if defined(ATEMA_CONTEXT_IMPL_GLFW)
	
	#define ATEMA_WINDOW_IMPL_GLFW
	
#endif

namespace at
{
	class mouse;
	class keyboard;
	
	class ATEMA_WINDOW_API window : public context
	{
		public:
			enum class options : flags
			{
				fullscreen	= 0x0001,
				visible		= 0x0002,
				autoscale	= 0x0004,
				resizable	= 0x0008,
				frame		= 0x0010,
				vsync		= 0x0020,
				adapt		= 0x0040
			};
			
		//To implement for each OS
		public:
			window();
			virtual ~window() noexcept;
			
			void create(unsigned int w, unsigned int h, const char *name, flags flag_list, const context::gl_version& version);
			void create(int x, int y, unsigned int w, unsigned int h, const char *name, flags flag_list, const context::gl_version& version);
			
			void set_position(int x, int y);
			
			operator bool() const noexcept; //true if open, false otherwise
			
			void update();
			
		private:
			friend class at::mouse;
			friend class at::keyboard;
			
		//OS specific
		#if defined(ATEMA_WINDOW_IMPL_GLFW)
		private:
			GLFWwindow* get_glfw_window() const noexcept;
		#endif
			
	};
}

ATEMA_ENUM_FLAGS(at::window::options)

#endif