#include <atema/context/context.hpp>

#include <atema/core/error.hpp>
#include "internal_config.hpp"

#if defined(ATEMA_CONTEXT_IMPL_GLFW)
	
	#include "context_impl_glfw.hpp"
	
#else
	
	#error No existing context implementation
	
#endif

namespace at
{
	//PUBLIC
	context::context()
	try :
	#if defined(ATEMA_CONTEXT_IMPL_GLFW)
		m_pimpl(ATEMA_PIMPL_SHARE(context_impl)(ATEMA_PIMPL_CTOR(context_impl_glfw)()))
	#endif
	{
		
	}
	catch (...)
	{
		ATEMA_ERROR("Creation failed.")
	}
	
	context::~context() noexcept
	{
		
	}
	
	void context::create(const gl_version& version)
	{
		
	}
	
	void context::activate()
	{
		
	}
	
	bool context::is_active() const noexcept
	{
		
	}
	
	//PROTECTED
	context::context(ATEMA_PIMPL(context_impl) pimpl)
	try :
	#if defined(ATEMA_CONTEXT_IMPL_GLFW)
		m_pimpl(pimpl)
	#endif
	{
		
	}
	catch (...)
	{
		ATEMA_ERROR("Creation failed.")
	}
}