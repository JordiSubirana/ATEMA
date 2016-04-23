#ifndef ATEMA_WINDOW_HEADER
#define ATEMA_WINDOW_HEADER

#include <atema/window/config.hpp>
#include <atema/context/context.hpp>

#include <atema/utility/enum_flags.hpp>
#include <atema/utility/pimpl.hpp>

namespace at
{
	class window_impl;
	
	class ATEMA_WINDOW_API window : public context
	{
		public:
			enum class options : int
			{
				fullscreen	= 0x0001,
				visible		= 0x0002,
				autoscale	= 0x0004,
				resizable	= 0x0008,
				frame		= 0x0010,
				vsync		= 0x0020
			};
			
		public:
			window();
			virtual ~window() noexcept;
			
			void create(unsigned int w, unsigned int h, const char *name, flags flag_list, const context::gl_version& version);
			void create(int x, int y, unsigned int w, unsigned int h, const char *name, flags flag_list, const context::gl_version& version);

		// TODO SCHLO
		protected:
			ATEMA_PIMPL(window_impl) m_pimpl;
	};
}

ATEMA_ENUM_FLAGS(at::window::options)

#endif