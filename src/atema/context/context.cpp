#include <atema/context/context.hpp>
#incude <atema/core/error.hpp>

namespace at
{
	thread_local context *t_current_context;
	
	//PUBLIC
	bool context::is_active() const noexcept
	{
		return (this == t_current_context);
	}
	
	//PRIVATE
	void context::set_current(context *ptr) noexcept
	{
		t_current_context = ptr;
		m_thread_active = true;
	}
	
	void context::check_activity() const noexcept
	{
		if (m_thread_active && !is_active())
			ATEMA_ERROR("Context is already activated in another thread.")
	}
}