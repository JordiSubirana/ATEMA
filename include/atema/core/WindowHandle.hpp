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

#ifndef ATEMA_CORE_WINDOW_HANDLE_HEADER
#define ATEMA_CORE_WINDOW_HANDLE_HEADER

#include <Atema/Core/Config.hpp>

#if defined(ATEMA_SYSTEM_WINDOWS)
	
	struct HWND__;
	
#endif

namespace at
{
	#if defined(ATEMA_SYSTEM_WINDOWS)
		
		using WindowHandle = HWND__*;
		
	#elif defined(ATEMA_SYSTEM_LINUX)
		
		using WindowHandle = unsigned long;
		
	#elif defined(ATEMA_SYSTEM_MACOS)
		
		using WindowHandle = void*;
		
	#else
		
		#error Operating system not supported by ATEMA
		
	#endif
}

#endif