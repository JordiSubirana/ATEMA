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

#ifndef ATEMA_CONTEXT_RESOURCE_GL_HEADER
#define ATEMA_CONTEXT_RESOURCE_GL_HEADER

#include <atema/context/config.hpp>
#include <atema/context/opengl.hpp>
#include <atema/context/object_gl.hpp>

namespace at
{
	class ResourceGL : public ObjectGL
	{
		public:
			using ObjectGL::get_gl_id;
			using ObjectGL::is_valid;
			
		public:
			ResourceGL() = default;
			virtual ~ResourceGL() = default;
			
			virtual void to_cpu() = 0;
			virtual void to_gpu() = 0;
	};
}

#endif