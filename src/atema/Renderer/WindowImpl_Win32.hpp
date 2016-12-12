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

#ifndef ATEMA_HMI_WINDOW_IMPL_WIN32_HEADER
#define ATEMA_HMI_WINDOW_IMPL_WIN32_HEADER

#include <Atema/Renderer/Config.hpp>

#if defined(ATEMA_SYSTEM_WINDOWS)

#include <Atema/Renderer/Window.hpp>

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
	#define NOMINMAX
#endif
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX

#include <atomic>

namespace at
{	
	class Window::Impl
	{
		public:
			Impl();
			virtual ~Impl();
			
			void reset(unsigned int w, unsigned int h, const std::string& name, const Window::Style& style = Window::Style());
			
			void set_position(int x, int y);
			void set_title(const std::string& title);
			
			void close();
			bool should_close() const noexcept;
			
			void process_events();
			
			WindowHandle get_handle();
			const WindowHandle get_handle() const;
			
		private:
			static LRESULT CALLBACK global_process_message(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);
			static std::atomic_uint s_count;
			
			bool process_message(UINT message, WPARAM wparam, LPARAM lparam);
			
			WindowHandle m_handle;
			bool m_should_close;
			bool m_fullscreen;
	};
}

#endif

#endif