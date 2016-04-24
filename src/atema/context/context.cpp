#include <atema/context/context.hpp>
#include <atema/core/error.hpp>

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
		if (t_current_context)
			t_current_context->m_thread_active = false;
		
		t_current_context = ptr;
		
		if (ptr)
			ptr->m_thread_active = true;
	}
	
	void context::check_activity() const noexcept
	{
		if (m_thread_active && !is_active())
			ATEMA_ERROR("Context is already activated in another thread.")
	}
	
	void context::init_gl_states() const noexcept
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
	}
}