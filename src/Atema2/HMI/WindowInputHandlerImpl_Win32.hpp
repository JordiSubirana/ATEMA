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

#ifndef ATEMA_HMI_WINDOW_INPUT_HANDLER_IMPL_WIN32_HEADER
#define ATEMA_HMI_WINDOW_INPUT_HANDLER_IMPL_WIN32_HEADER

#include <Atema/HMI/Config.hpp>

#if defined(ATEMA_SYSTEM_WINDOWS)

#include <Atema/HMI/WindowInputHandler.hpp>

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
	#define NOMINMAX
#endif
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX

#include <unordered_map>
#include <list>
#include <mutex>

namespace at
{	
	class WindowInputHandler::Impl
	{
		public:
			Impl();
			virtual ~Impl() noexcept;
			
			void reset(WindowHandle handle);
			
			WindowHandle get_handle();
			const WindowHandle get_handle() const;
			
		private:
			static LRESULT CALLBACK global_process_message(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);
			static std::mutex s_mutex;
			static std::unordered_map<WindowHandle, std::list<Impl*>> s_handlers;
			
			void process_message(UINT message, WPARAM wparam, LPARAM lparam);
			LONG_PTR m_callback;
			
			WindowHandle m_handle;
	};
}

#endif

#endif