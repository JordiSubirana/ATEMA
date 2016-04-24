#include <atema/context/context.hpp>

#ifdef ATEMA_CONTEXT_IMPL_GLFW

namespace at
{
	context::context() :
		m_thread_active(false),
		m_valid(false),
		m_active(false),
		m_flags(0),
		m_viewport{0,0,0,0},
		m_infos{0,0,0,0},
		m_name(""),
		m_window(nullptr)
	{
		
	}
	
	context::~context() noexcept
	{
		
	}

	bool context::is_valid() const noexcept
	{
		
	}
	
	void context::create(const gl_version& version)
	{
		
	}
	
	void context::activate(bool value)
	{
		if (value == is_active())
			return;
		
		if (value)
		{
			set_current(this);
		}
		else
		{
			set_current(nullptr);
		}
	}

	//--------------------
	//SPECIFIC GLFW
	void context::create(unsigned int w, unsigned int h, const char *name, flags flag_list, const gl_version& version)
	{
		
	}
	
	void context::destroy_window() noexcept
	{
		if (m_window)
			glfwDestroyWindow(m_window);
		
		m_window = nullptr;
	}
}

#endif