// ----------------------------------------------------------------------
// Copyright (C) 2016 Jordi SUBIRANA
//
// This file is part of ATEMA.
//
// ATEMA is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ATEMA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ATEMA.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------

#include <atema/context/context.hpp>
#include <atema/core/error.hpp>

namespace at
{
	thread_local Context *t_current_context = nullptr;
	
	//PUBLIC
	bool Context::is_current_context() const noexcept
	{
		return (this == t_current_context);
	}
	
	//PRIVATE
	void Context::set_current(Context *ptr) noexcept
	{
		if (t_current_context)
			t_current_context->m_thread_active = false;
		
		t_current_context = ptr;
		
		if (ptr)
			ptr->m_thread_active = true;
	}
	
	void Context::check_activity() const noexcept
	{
		if (m_thread_active && !is_current_context())
			ATEMA_ERROR("Context is already activated in another thread.")
	}
	
	void Context::init_gl_states() const noexcept
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		glActiveTexture(GL_TEXTURE0);
		// glEnable(GL_TEXTURE_2D); //Deprecated for non-fixed pipeline ! 3.X
	}
}