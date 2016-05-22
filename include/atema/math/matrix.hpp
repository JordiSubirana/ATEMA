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

#ifndef ATEMA_MATH_MATRIX_HEADER
#define ATEMA_MATH_MATRIX_HEADER

#include <atema/math/config.hpp>
#include <atema/math/vector.hpp>

#include <iostream>

#define ATEMA_MATRIX_ALIASES(COL, ROW)													\
	template <typename T>																\
	using Matrix ## COL ## x ## ROW = Matrix<COL, ROW, T>;								\
	using Matrix ## COL ## x ## ROW ## i = Matrix ## COL ## x ## ROW <int>;				\
	using Matrix ## COL ## x ## ROW ## u = Matrix ## COL ## x ## ROW <unsigned int>;	\
	using Matrix ## COL ## x ## ROW ## f = Matrix ## COL ## x ## ROW <float>;			\
	using Matrix ## COL ## x ## ROW ## d = Matrix ## COL ## x ## ROW <double>;

#define ATEMA_MATRIX_ALIASES_SINGLE(DIM)						\
	template <typename T>										\
	using Matrix ## DIM = Matrix<DIM, DIM, T>;					\
	using Matrix ## DIM ## i = Matrix ## DIM <int>;				\
	using Matrix ## DIM ## u = Matrix ## DIM <unsigned int>;	\
	using Matrix ## DIM ## f = Matrix ## DIM <float>;			\
	using Matrix ## DIM ## d = Matrix ## DIM <double>;

namespace at
{
	template <size_t COL, size_t ROW, typename T>
	class Matrix
	{
		public:
			Matrix();
			Matrix(T arg);
			virtual ~Matrix() noexcept;
			
			Matrix<ROW, COL, T> get_transposed() const noexcept;
			
			Matrix<COL, ROW, T> operator +(const Matrix<COL, ROW, T>& arg) const;
			Matrix<COL, ROW, T> operator -(const Matrix<COL, ROW, T>& arg) const;
			Matrix<ROW, ROW, T> operator *(const Matrix<ROW, COL, T>& arg) const;
			Matrix<COL, ROW, T> operator +(T arg) const;
			Matrix<COL, ROW, T> operator -(T arg) const;
			Matrix<COL, ROW, T> operator *(T arg) const;
			Matrix<COL, ROW, T> operator /(T arg) const;
			
			Matrix<COL, ROW, T>& operator +=(const Matrix<COL, ROW, T>& arg);
			Matrix<COL, ROW, T>& operator -=(const Matrix<COL, ROW, T>& arg);
			Matrix<COL, ROW, T>& operator +=(T arg);
			Matrix<COL, ROW, T>& operator -=(T arg);
			Matrix<COL, ROW, T>& operator *=(T arg);
			Matrix<COL, ROW, T>& operator /=(T arg);
			
			Vector<ROW, T>& operator[](size_t index);
			const Vector<ROW, T>& operator[](size_t index) const;
			
			T* get();
			const T* get() const;
			
		private:
			union
			{
				T m_data[COL*ROW];
				Vector<ROW, T> m_columns[COL];
			};
	};
	
	ATEMA_MATRIX_ALIASES_SINGLE(2)
	ATEMA_MATRIX_ALIASES(2,2)
	ATEMA_MATRIX_ALIASES(2,3)
	ATEMA_MATRIX_ALIASES(2,4)
	ATEMA_MATRIX_ALIASES_SINGLE(3)
	ATEMA_MATRIX_ALIASES(3,2)
	ATEMA_MATRIX_ALIASES(3,3)
	ATEMA_MATRIX_ALIASES(3,4)
	ATEMA_MATRIX_ALIASES_SINGLE(4)
	ATEMA_MATRIX_ALIASES(4,2)
	ATEMA_MATRIX_ALIASES(4,3)
	ATEMA_MATRIX_ALIASES(4,4)
}

template <size_t COL, size_t ROW, typename T>
std::ostream& operator<<(std::ostream& os, const at::Matrix<COL, ROW, T>& mat)
{
    for (size_t r = 0; r < ROW; r++)
	{
		for (size_t c = 0; c < COL; c++)
		{
			os << mat[c][r];
			
			if (c < COL - 1)
				os << ", ";
		}
		
		if (r < ROW-1)
			os << '\n';
	}
	
    return (os);
}

#include <atema/math/matrix.tpp>

#endif