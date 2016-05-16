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

#ifndef ATEMA_MATH_RECT_HEADER
#define ATEMA_MATH_RECT_HEADER

#include <atema/math/config.hpp>

namespace at
{
	class ATEMA_MATH_API Rect
	{
		public:
			Rect() :
				x(0), y(0), w(0), h(0)
			{
				
			}
			Rect(int x, int y, unsigned int w, unsigned int h) :
				x(x), y(y), w(w), h(h)
			{
				
			}
			
			int x, y;
			unsigned int w, h;
	};
}

#endif