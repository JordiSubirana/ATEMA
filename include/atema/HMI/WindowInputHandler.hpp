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

#ifndef ATEMA_HMI_WINDOW_INPUT_HANDLER_HEADER
#define ATEMA_HMI_WINDOW_INPUT_HANDLER_HEADER

#include <Atema/HMI/Config.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Core/WindowHandle.hpp>

namespace at
{	
	class ATEMA_HMI_API WindowInputHandler : public NonCopyable
	{
		public:
			WindowInputHandler();
			WindowInputHandler(WindowHandle handle);
			virtual ~WindowInputHandler() noexcept;
			
			void reset(WindowHandle handle);
			
			WindowHandle get_handle();
			const WindowHandle get_handle() const;
			
		private:
			class Impl;
			
			WindowInputHandler::Impl *m_impl;
	};
}

#endif