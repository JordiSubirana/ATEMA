#ifndef ATEMA_CONTEXT_IMPL_GLFW_HEADER
#define ATEMA_CONTEXT_IMPL_GLFW_HEADER

#include "context_impl.hpp"

#include "internal_config.hpp"

#ifdef ATEMA_CONTEXT_IMPL_GLFW

#include "../window/window_impl_glfw.hpp"

namespace at
{
	class context_impl_glfw : public window_impl_glfw
	{
		public:
			context_impl_glfw();
			virtual ~context_impl_glfw() noexcept;
	};
}

#endif

#endif