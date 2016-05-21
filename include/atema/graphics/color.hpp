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

#ifndef ATEMA_GRAPHICS_COLOR_HEADER
#define ATEMA_GRAPHICS_COLOR_HEADER

#include <atema/graphics/config.hpp>

namespace at
{
	class Color
	{
		public:
			Color() noexcept :
				Color(0.0f, 0.0f, 0.0f, 1.0f)
			{
				
			}
			
			Color(float r, float g, float b, float a) noexcept :
				r(r), b(b), g(g), a(a)
			{
				
			}
			
			float r;
			float g;
			float b;
			float a;
	};
}

#endif