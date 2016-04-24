#include <atema/window/window.hpp>

#include <atema/core/error.hpp>
#include "internal_config.hpp"

#if defined(ATEMA_WINDOW_IMPL_GLFW)
	
	#define ATEMA_WINDOW_IMPL_GLFW_IMPLEMENTATION
	#include "window_impl_glfw.hpp"
	
#else
	
	#error No existing window implementation
	
#endif

namespace at
{
	window::window()
	try :
	#if defined(ATEMA_WINDOW_IMPL_GLFW)
		m_pimpl(std::make_shared<window_impl_glfw>()),
		context(std::static_pointer_cast<context_impl>(m_pimpl))
	#endif
	{
		
	}
	catch (const error& e)
	{
		throw;
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