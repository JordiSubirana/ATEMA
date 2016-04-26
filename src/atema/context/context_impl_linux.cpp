#include <atema/context/context.hpp>

#ifdef ATEMA_SYSTEM_LINUX

#define GLFW_EXPOSE_NATIVE_GLX // context API
#define GLFW_EXPOSE_NATIVE_X11 // window API
#include <GLFW/glfw3native.h>

namespace at
{
	GLXContext Context::get_current_os_context() noexcept
	{
		return (glXGetCurrentContext());
	}
}

#endif