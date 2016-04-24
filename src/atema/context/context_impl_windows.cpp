#include <atema/context/context.hpp>

#ifdef ATEMA_SYSTEM_WINDOWS

namespace at
{
	HGLRC Context::get_current_os_context() noexcept
	{
		return (wglGetCurrentContext());
	}
}

#endif