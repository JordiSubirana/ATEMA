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

#ifndef ATEMA_CONTEXT_OBJECT_GL_HEADER
#define ATEMA_CONTEXT_OBJECT_GL_HEADER

#include <atema/context/config.hpp>
#include <atema/context/opengl.hpp>

namespace at
{
	class ObjectGL
	{
		public:
			ObjectGL() = default;
			virtual ~ObjectGL() = default;
			
			virtual GLuint get_gl_id() const = 0;
			virtual bool is_valid() const = 0;
	};
}

#endif