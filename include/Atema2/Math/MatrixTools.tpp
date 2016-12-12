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

#include <Atema/Core/Error.hpp>

template <typename T>
at::Vector3<T> operator*(const at::Matrix4<T>& mat, const at::Vector3<T>& vec)
{
	at::Vector4<T> vec_tmp;
	at::Vector3<T> tmp;
	
	vec_tmp.x = vec.x;
	vec_tmp.y = vec.y;
	vec_tmp.z = vec.z;
	vec_tmp.w = 1;
	
	vec_tmp = mat * vec_tmp;
	
	tmp.x = vec_tmp.x;
	tmp.y = vec_tmp.y;
	tmp.z = vec_tmp.z;
	
	return (tmp);
}

namespace at
{
	template <typename T>
	Matrix4<T> Transform::look_at(const Vector3<T>& origin_position, const Vector3<T>& target_position, const Vector3<T>& up_direction)
	{
		Matrix4<T> tmp;
		
		Vector3<T> x_axis;
		Vector3<T> y_axis;
		Vector3<T> z_axis;
		
		z_axis = (target_position - origin_position).normalize();
		x_axis = cross(up_direction, z_axis).normalize();
		y_axis = cross(z_axis, x_axis);
		
		tmp[0][0] = x_axis.x;
		tmp[0][1] = y_axis.x;
		tmp[0][2] = -z_axis.x;
		tmp[0][3] = 0;
		
		tmp[1][0] = x_axis.y;
		tmp[1][1] = y_axis.y;
		tmp[1][2] = -z_axis.y;
		tmp[1][3] = 0;
		
		tmp[2][0] = x_axis.z;
		tmp[2][1] = y_axis.z;
		tmp[2][2] = -z_axis.z;
		tmp[2][3] = 0;
		
		tmp[3][0] = -dot(x_axis, origin_position);
		tmp[3][1] = -dot(y_axis, origin_position);
		tmp[3][2] = dot(z_axis, origin_position);
		tmp[3][3] = 1;
		
		return (tmp);
	}
	
	template <typename T>
	Matrix4<T> Transform::perspective(T fov, T ratio, T z_near, T z_far)
	{
		Matrix4<T> tmp;
		
		T range = z_near * static_cast<T>(std::tan(fov/static_cast<T>(2)));
		T left = -range * ratio;
		T right = range * ratio;
		T bottom = -range;
		T top = range;
		
		tmp[0][0] = (2.0 * z_near) / (right - left);
		tmp[0][1] = 0;
		tmp[0][2] = 0;
		tmp[0][3] = 0;
		
		tmp[1][0] = 0;
		tmp[1][1] = (2.0 * z_near) / (top - bottom);
		tmp[1][2] = 0;
		tmp[1][3] = 0;
		
		tmp[2][0] = 0;
		tmp[2][1] = 0;
		tmp[2][2] = -(z_far + z_near) / (z_far - z_near);
		tmp[2][3] = -1;
		
		tmp[3][0] = 0;
		tmp[3][1] = 0;
		tmp[3][2] = -z_far*z_near*2 / (z_far - z_near);
		tmp[3][3] = 1;
		
		return (tmp);
	}
	
	template <typename T>
	Matrix4<T> Transform::rotation(T angle_rad, const Vector3<T>& axis)
	{
		Matrix4<T> tmp;
		
		//Axis normalized
		Vector3<T> an = axis.get_normalized();
		T cos_a = static_cast<T>(std::cos(angle_rad));
		T sin_a = static_cast<T>(std::sin(angle_rad));
		T one_minus_cos = static_cast<T>(1.0) - cos_a;
		
		tmp[0][0] = an.x*an.x*one_minus_cos + cos_a;
		tmp[0][1] = an.x*an.y*one_minus_cos + an.z*sin_a;
		tmp[0][2] = an.x*an.z*one_minus_cos - an.y*sin_a;
		tmp[0][3] = 0;
		
		tmp[1][0] = an.y*an.x*one_minus_cos - an.z*sin_a;
		tmp[1][1] = an.y*an.y*one_minus_cos + cos_a;
		tmp[1][2] = an.y*an.z*one_minus_cos + an.x*sin_a;
		tmp[1][3] = 0;
		
		tmp[2][0] = an.z*an.x*one_minus_cos + an.y*sin_a;
		tmp[2][1] = an.z*an.y*one_minus_cos - an.x*sin_a;
		tmp[2][2] = an.z*an.z*one_minus_cos + cos_a;
		tmp[2][3] = 0;
		
		tmp[3][0] = 0;
		tmp[3][1] = 0;
		tmp[3][2] = 0;
		tmp[3][3] = 1;
		
		return (tmp);
	}
	
	template <typename T>
	Matrix4<T> Transform::translation(const Vector3<T>& vector)
	{
		Matrix4<T> tmp;
		
		tmp[0][0] = 0;
		tmp[0][1] = 0;
		tmp[0][2] = 0;
		tmp[0][3] = vector.x;
		
		tmp[1][0] = 0;
		tmp[1][1] = 0;
		tmp[1][2] = 0;
		tmp[1][3] = vector.y;
		
		tmp[2][0] = 0;
		tmp[2][1] = 0;
		tmp[2][2] = 0;
		tmp[2][3] = vector.z;
		
		tmp[3][0] = 0;
		tmp[3][1] = 0;
		tmp[3][2] = 0;
		tmp[3][3] = 1;
		
		return (tmp);
	}
	
	template <typename T>
	Matrix4<T> Transform::scale(const Vector3<T>& vector)
	{
		Matrix4<T> tmp;
		
		tmp[0][0] = vector.x;
		tmp[0][1] = 0;
		tmp[0][2] = 0;
		tmp[0][3] = 0;
		
		tmp[1][0] = 0;
		tmp[1][1] = vector.y;
		tmp[1][2] = 0;
		tmp[1][3] = 0;
		
		tmp[2][0] = 0;
		tmp[2][1] = 0;
		tmp[2][2] = vector.z;
		tmp[2][3] = 0;
		
		tmp[3][0] = 0;
		tmp[3][1] = 0;
		tmp[3][2] = 0;
		tmp[3][3] = 1;
		
		return (tmp);
	}
}

#endif