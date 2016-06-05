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

#ifndef ATEMA_MATH_MATRIX_TOOLS_IMPLEMENTATION
#define ATEMA_MATH_MATRIX_TOOLS_IMPLEMENTATION

#include <atema/core/error.hpp>

namespace at
{
	template <typename T>
	Matrix4<T> look_at(const Vector3<T>& origin_position, const Vector3<T>& target_position, const Vector3<T>& up_direction)
	{
		Matrix4<T> tmp;
		
		Vector3<T> x_axis;
		Vector3<T> y_axis;
		Vector3<T> z_axis;
		
		z_axis = (target_position - origin_position).normalize();		
		x_axis = cross(up_direction, z_axis).normalize();		
		y_axis = cross(z_axis, x_axis);
		
		tmp[0][0] = x_axis.x;
		tmp[0][1] = x_axis.y;
		tmp[0][2] = x_axis.z;
		tmp[0][3] = -dot(x_axis, origin_position);
		
		tmp[1][0] = y_axis.x;
		tmp[1][1] = y_axis.y;
		tmp[1][2] = y_axis.z;
		tmp[1][3] = -dot(y_axis, origin_position);
		
		tmp[2][0] = z_axis.x;
		tmp[2][1] = z_axis.y;
		tmp[2][2] = z_axis.z;
		tmp[2][3] = -dot(z_axis, origin_position);
		
		tmp[3][0] = 0;
		tmp[3][1] = 0;
		tmp[3][2] = 0;
		tmp[3][3] = 1;
		
		return (tmp);
	}
}

#endif