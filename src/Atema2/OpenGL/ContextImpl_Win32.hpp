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

#ifndef ATEMA_OPENGL_CONTEXT_IMPL_WIN32_HEADER
#define ATEMA_OPENGL_CONTEXT_IMPL_WIN32_HEADER

#include <Atema/OpenGL/Config.hpp>

#if defined(ATEMA_SYSTEM_WINDOWS)

#include <Atema/OpenGL/Context.hpp>

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
	#define NOMINMAX
#endif
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX

namespace at
{	
	class OpenGLContext::Impl
	{
		public:
			Impl();
			virtual ~Impl() noexcept;
			
			void reset(unsigned int w, unsigned int h, const Context::Settings& settings);
			void reset(WindowHandle handle, const Context::Settings& settings);
			
			void set_current(bool current);
			
			void swap_buffers();
			
		private:
			void reset(const Context::Settings& settings);
			
			HDC m_device_context;
			HGLRC m_context;
			HWND m_window;
			bool m_owns_window;
	};
}

#endif

#endif