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

#ifndef ATEMA_MATH_VECTOR_HEADER
#define ATEMA_MATH_VECTOR_HEADER

#include <atema/math/config.hpp>

#define ATEMA_VECTOR_BEGIN(DIM)					\
	template <typename T>						\
	class Vec<DIM, T>							\
	{											\
		public:									\
			union								\
			{									\
				T data[DIM];

#define ATEMA_VECTOR_FIELD(...)					\
				struct { T __VA_ARGS__; };

#define ATEMA_VECTOR_END						\
			};									\
	};

#define ATEMA_VECTOR_ALIASES(DIM)								\
	template <typename T>										\
	using Vector ## DIM = Vector<DIM, T>;						\
	using Vector ## DIM ## i = Vector ## DIM <int>;				\
	using Vector ## DIM ## u = Vector ## DIM <unsigned int>;	\
	using Vector ## DIM ## f = Vector ## DIM <float>;			\
	using Vector ## DIM ## d = Vector ## DIM <double>;

namespace at
{
	template <int N, typename T>
	class Vec
	{
		public:
			T data[N];
	};

	ATEMA_VECTOR_BEGIN(2)
		ATEMA_VECTOR_FIELD(x, y)
		ATEMA_VECTOR_FIELD(w, h)
	ATEMA_VECTOR_END

	ATEMA_VECTOR_BEGIN(3)
		ATEMA_VECTOR_FIELD(x, y, z)
		ATEMA_VECTOR_FIELD(w, h, d)
		ATEMA_VECTOR_FIELD(r, g, b)
	ATEMA_VECTOR_END

	ATEMA_VECTOR_BEGIN(4)
		ATEMA_VECTOR_FIELD(x, y, z, w)
		ATEMA_VECTOR_FIELD(r, g, b, a)
	ATEMA_VECTOR_END

	template <size_t N, typename T>
	class ATEMA_MATH_API Vector : public Vec<N, T>
	{
		public:
			// Vector();
			// Vector(T arg);
			
			template <typename... Args>
			Vector(Args... args)
			{
				// init<0>(std::forward<Args>(args)...);
				init<0>(args...);
			}
			
			~Vector() noexcept;

			Vector<N, T>& normalize() noexcept;
			T get_norm() const noexcept;

			Vector<N, T> operator +(const Vector<N, T>& arg) const;
			Vector<N, T> operator -(const Vector<N, T>& arg) const;
			Vector<N, T> operator *(const Vector<N, T>& arg) const;
			Vector<N, T> operator /(const Vector<N, T>& arg) const;
			Vector<N, T> operator +(T arg) const;
			Vector<N, T> operator -(T arg) const;
			Vector<N, T> operator *(T arg) const;
			Vector<N, T> operator /(T arg) const;

			Vector<N, T>& operator +=(const Vector<N, T>& arg);
			Vector<N, T>& operator -=(const Vector<N, T>& arg);
			Vector<N, T>& operator *=(const Vector<N, T>& arg);
			Vector<N, T>& operator /=(const Vector<N, T>& arg);
			Vector<N, T>& operator +=(T arg);
			Vector<N, T>& operator -=(T arg);
			Vector<N, T>& operator *=(T arg);
			Vector<N, T>& operator /=(T arg);
			
		private:
			template <size_t M, typename... Args>
			void init(T arg, Args... args)
			{
				this->data[M] = arg;
				// init<M+1>(std::forward<Args>(args)...);
				init<M+1>(args...);
			}
			
			template <size_t M>
			void init(T arg)
			{
				this->data[M] = arg;
			}
			
			template <size_t M>
			void init()
			{
				
			}
	};

	ATEMA_VECTOR_ALIASES(2)
	ATEMA_VECTOR_ALIASES(3)
	ATEMA_VECTOR_ALIASES(4)
}

#include <atema/math/vector.tpp>

#endif