#ifndef ATEMA_WINDOW_IMPL_HEADER
#define ATEMA_WINDOW_IMPL_HEADER

#include <atema/window/window.hpp>

#include "../context/context_impl.hpp"

namespace at
{
	class window_impl : public context_impl
	{
		public:
			window_impl() noexcept;
			virtual ~window_impl() noexcept;
			
			virtual void create(unsigned int w, unsigned int h, const char *name, flags flag_list, const context::gl_version& version) = 0;
			virtual void create(int x, int y, unsigned int w, unsigned int h, const char *name, flags flag_list, const context::gl_version& version) = 0;

		// TODO SCHLO
		protected:
			flags m_flags;
	};
}

#endif