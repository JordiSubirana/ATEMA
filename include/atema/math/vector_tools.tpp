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

#ifndef ATEMA_MATH_VECTOR_TOOLS_IMPLEMENTATION
#define ATEMA_MATH_VECTOR_TOOLS_IMPLEMENTATION

#include <atema/core/error.hpp>

namespace at
{
	template <size_t N, typename T>
	T dot(const Vector<N, T>& v1, const Vector<N, T>& v2)
	{
		T tmp = static_cast<T>(0);
		
		for (size_t i = 0; i < N; i++)
			tmp += v1[i]*v2[i];
		
		return (tmp);
	}
	
	template <typename T>
	Vector3<T> cross(const Vector3<T>& v1, const Vector3<T>& v2)
	{
		Vector3<T> tmp;
		
		tmp.x = v1.y*v2.z - v1.z*v2.y;
		tmp.y = v1.z*v2.x - v1.x*v2.z;
		tmp.z = v1.x*v2.y - v1.y*v2.x;
		
		return (tmp);
	}
}

#endif