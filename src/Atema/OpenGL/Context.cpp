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

#include <Atema/OpenGL/Context.hpp>

#if defined(ATEMA_SYSTEM_WINDOWS)
	#include "ContextImpl_Win32.hpp"
#else
	#error Operating system not supported by Renderer module
#endif

#include <glad/glad.h>

namespace at
{
	OpenGLContext::OpenGLContext()
	{
		m_impl = new OpenGLContext::Impl();
	}

	OpenGLContext::~OpenGLContext()
	{
		delete m_impl;
	}

	void OpenGLContext::reset(unsigned int w, unsigned int h, const Context::Settings& settings)
	{
		m_impl->reset(w, h, settings);
		
		if (!gladLoadGL())
			ATEMA_ERROR("Loading of OpenGL functions failed.")
	}

	void OpenGLContext::reset(WindowHandle handle, const Context::Settings& settings)
	{
		m_impl->reset(handle, settings);
		
		if (!gladLoadGL())
			ATEMA_ERROR("Loading of OpenGL functions failed.")
	}
	
	void OpenGLContext::set_current(bool current)
	{
		m_impl->set_current(current);
	}

	void OpenGLContext::swap_buffers()
	{
		m_impl->swap_buffers();
	}
}