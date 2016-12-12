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

#include <Atema/HMI/WindowInputHandler.hpp>
#include <Atema/Core/Error.hpp>

#if defined(ATEMA_SYSTEM_WINDOWS)
	#include "WindowInputHandlerImpl_Win32.hpp"
#else
	#error Operating system not supported by HMI module
#endif

namespace at
{
	WindowInputHandler::WindowInputHandler()
	{
		m_impl = new WindowInputHandler::Impl();
	}
	
	WindowInputHandler::WindowInputHandler(WindowHandle handle) :
		WindowInputHandler()
	{
		reset(handle);
	}
	
	WindowInputHandler::~WindowInputHandler() noexcept
	{
		delete m_impl;
	}
	
	void WindowInputHandler::reset(WindowHandle handle)
	{
		m_impl->reset(handle);
	}
	
	WindowHandle WindowInputHandler::get_handle()
	{
		m_impl->get_handle();
	}
	
	const WindowHandle WindowInputHandler::get_handle() const
	{
		m_impl->get_handle();
	}
}