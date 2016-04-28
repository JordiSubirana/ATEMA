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

#ifdef ATEMA_SYSTEM_LINUX

#define GLFW_EXPOSE_NATIVE_GLX // context API
#define GLFW_EXPOSE_NATIVE_X11 // window API
#include <GLFW/glfw3native.h>

namespace at
{
	GLXContext Context::get_current_os_context() noexcept
	{
		return (glXGetCurrentContext());
	}
}

#endif