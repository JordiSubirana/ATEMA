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

#ifndef ATEMA_MATH_VECTOR2_HEADER
#define ATEMA_MATH_VECTOR2_HEADER

#include <atema/math/config.hpp>

namespace at
{
	template <typename T>
	class ATEMA_MATH_API Vector2
	{
		public:
			Vector2();
			Vector2(T arg);
			Vector2(T x, T y);
			~Vector2() noexcept;
			
			union
			{
				struct { T x, y; };
				struct { T w, h; };
			};
			
			Vector2& normalize() noexcept;
			T get_norm() const noexcept;
			
			Vector2 operator +(const Vector2& arg) const;
			Vector2 operator -(const Vector2& arg) const;
			Vector2 operator *(const Vector2& arg) const;
			Vector2 operator /(const Vector2& arg) const;
			Vector2 operator +(T arg) const;
			Vector2 operator -(T arg) const;
			Vector2 operator *(T arg) const;
			Vector2 operator /(T arg) const;
			
			void operator +=(const Vector2& arg);
			void operator -=(const Vector2& arg);
			void operator *=(const Vector2& arg);
			void operator /=(const Vector2& arg);
			void operator +=(T arg);
			void operator -=(T arg);
			void operator *=(T arg);
			void operator /=(T arg);
	};
	
	using Vector2i = Vector2<int>;
	using Vector2u = Vector2<unsigned int>;
	using Vector2f = Vector2<float>;
	using Vector2d = Vector2<double>;
}

#define ATEMA_OPERATOR(type, symbol, arg) \
	

#endif