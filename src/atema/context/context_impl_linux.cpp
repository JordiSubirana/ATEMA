#include <atema/context/context.hpp>

#ifdef ATEMA_SYSTEM_LINUX

namespace at
{
	GLXContext Context::get_current_os_context() noexcept
	{
		return (glXGetCurrentContext());
	}
}

#endif