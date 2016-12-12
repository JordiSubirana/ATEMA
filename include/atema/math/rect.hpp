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

#include <Atema/Math/Config.hpp>

#include <cmath>

namespace at
{
	class Rect
	{
		public:
			Rect() :
				x1(0), y1(0), x2(0), y2(0)
			{
				
			}
			Rect(int x1, int y1, int x2, int y2) :
				x1(x1), y1(y1), x2(x2), y2(y2)
			{
				
			}
			
			void create(int x1, int y1, int x2, int y2)
			{
				this->x1 = x1;
				this->y1 = y1;
				this->x2 = x2;
				this->y2 = y2;
			}
			
			void set_width(unsigned int width) noexcept
			{
				x2 = x1 + static_cast<int>(width-1);
			}
			
			unsigned int get_width() const noexcept
			{
				return (static_cast<unsigned int>(abs(x2-x1)+1));
			}
			
			void set_height(unsigned int height) noexcept
			{
				y2 = y1 + static_cast<int>(height-1);
			}
			
			unsigned int get_height() const noexcept
			{
				return (static_cast<unsigned int>(abs(y2-y1)+1));
			}
			
			int x1, y1, x2, y2;
	};
}

#endif