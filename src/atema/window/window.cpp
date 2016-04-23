#include <atema/window/window.hpp>

#include <atema/core/error.hpp>
#include "internal_config.hpp"

#if defined(ATEMA_WINDOW_IMPL_GLFW)
	
	#include "window_impl_glfw.hpp"
	
#else
	
	#error No existing window implementation
	
#endif

namespace at
{
	window::window()
	try :
	#if defined(ATEMA_WINDOW_IMPL_GLFW)
		m_pimpl(ATEMA_PIMPL_SHARE(window_impl)(ATEMA_PIMPL_CTOR(window_impl_glfw)())),
		// window_impl_glfw <- window_impl <- context_impl
		context(ATEMA_PIMPL_SHARE(context_impl)(m_pimpl))
	#endif
	{
		
	}
	catch (...)
	{
		ATEMA_ERROR("Creation failed.")
	}
	
	window::~window() noexcept
	{
		
	}
	
	void window::create(unsigned int w, unsigned int h, const char *name, flags flag_list, const context::gl_version& version)
	{
		m_pimpl->create(w, h, name, flag_list, version);
	}
	
	void window::create(int x, int y, unsigned int w, unsigned int h, const char *name, flags flag_list, const context::gl_version& version)
	{
		m_pimpl->create(x, y, w, h, name, flag_list, version);
	}
}