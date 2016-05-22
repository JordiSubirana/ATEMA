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

#ifndef ATEMA_MATH_MATRIX_IMPLEMENTATION
#define ATEMA_MATH_MATRIX_IMPLEMENTATION

#include <atema/core/error.hpp>

namespace at
{
	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>::Matrix()
	{
		T tmp = static_cast<T>(0);
		size_t size = COL*ROW;
		
		for (size_t i = 0; i < size; i++)
			this->m_data[i] = tmp;
	}
	
	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>::Matrix(T arg) :
		Matrix()
	{
		size_t min = (COL < ROW ? COL : ROW);
		
		for (size_t i = 0; i < min; i++)
			this->m_columns[i][i] = arg;
	}
	
	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>::~Matrix() noexcept
	{
		
	}
	
	template <size_t COL, size_t ROW, typename T>
	Matrix<ROW, COL, T> Matrix<COL, ROW, T>::get_transposed() const noexcept
	{
		Matrix<ROW, COL, T> tmp;
		
		for (size_t c = 0; c < COL; c++)
			for (size_t r = 0; r < ROW; r++)
				tmp[r][c] = this->m_columns[c][r];
		
		return (tmp);
	}
	
	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T> Matrix<COL, ROW, T>::operator +(const Matrix<COL, ROW, T>& arg) const
	{
		Matrix<COL, ROW, T> tmp(*this);
		size_t size = COL*ROW;
		
		for (size_t i = 0; i < size; i++)
			tmp.m_data[i] += arg.m_data[i];
		
		return (tmp);
	}
	
	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T> Matrix<COL, ROW, T>::operator -(const Matrix<COL, ROW, T>& arg) const
	{
		Matrix<COL, ROW, T> tmp(*this);
		size_t size = COL*ROW;
		
		for (size_t i = 0; i < size; i++)
			tmp.m_data[i] -= arg.m_data[i];
		
		return (tmp);
	}
	
	template <size_t COL, size_t ROW, typename T>
	Matrix<ROW, ROW, T> Matrix<COL, ROW, T>::operator *(const Matrix<ROW, COL, T>& arg) const
	{
		Matrix<ROW, ROW, T> tmp;
		
		for (size_t t_c = 0; t_c < ROW; t_c++)
		{
			for (size_t t_r = 0; t_r < ROW; t_r++)
			{
				for (size_t c = 0; c < COL; c++)
				{
					tmp[t_c][t_r] += this->m_columns[c][t_r] * arg[t_c][c];
				}
			}
		}
		
		return (tmp);
	}
	
	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T> Matrix<COL, ROW, T>::operator +(T arg) const
	{
		Matrix<COL, ROW, T> tmp(*this);
		size_t size = COL*ROW;
		
		for (size_t i = 0; i < size; i++)
			tmp.m_data[i] += arg;
		
		return (tmp);
	}
	
	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T> Matrix<COL, ROW, T>::operator -(T arg) const
	{
		Matrix<COL, ROW, T> tmp(*this);
		size_t size = COL*ROW;
		
		for (size_t i = 0; i < size; i++)
			tmp.m_data[i] -= arg;
		
		return (tmp);
	}
	
	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T> Matrix<COL, ROW, T>::operator *(T arg) const
	{
		Matrix<COL, ROW, T> tmp(*this);
		size_t size = COL*ROW;
		
		for (size_t i = 0; i < size; i++)
			tmp.m_data[i] *= arg;
		
		return (tmp);
	}
	
	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T> Matrix<COL, ROW, T>::operator /(T arg) const
	{
		Matrix<COL, ROW, T> tmp(*this);
		size_t size = COL*ROW;
		
		for (size_t i = 0; i < size; i++)
			tmp.m_data[i] /= arg;
		
		return (tmp);
	}
	
	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>& Matrix<COL, ROW, T>::operator +=(const Matrix<COL, ROW, T>& arg)
	{
		size_t size = COL*ROW;
		
		for (size_t i = 0; i < size; i++)
			this->m_data[i] += arg.m_data[i];
		
		return (*this);
	}
	
	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>& Matrix<COL, ROW, T>::operator -=(const Matrix<COL, ROW, T>& arg)
	{
		size_t size = COL*ROW;
		
		for (size_t i = 0; i < size; i++)
			this->m_data[i] -= arg.m_data[i];
		
		return (*this);
	}
	
	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>& Matrix<COL, ROW, T>::operator +=(T arg)
	{
		size_t size = COL*ROW;
		
		for (size_t i = 0; i < size; i++)
			this->m_data[i] += arg;
		
		return (*this);
	}
	
	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>& Matrix<COL, ROW, T>::operator -=(T arg)
	{
		size_t size = COL*ROW;
		
		for (size_t i = 0; i < size; i++)
			this->m_data[i] -= arg;
		
		return (*this);
	}
	
	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>& Matrix<COL, ROW, T>::operator *=(T arg)
	{
		size_t size = COL*ROW;
		
		for (size_t i = 0; i < size; i++)
			this->m_data[i] *= arg;
		
		return (*this);
	}
	
	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>& Matrix<COL, ROW, T>::operator /=(T arg)
	{
		size_t size = COL*ROW;
		
		for (size_t i = 0; i < size; i++)
			this->m_data[i] /= arg;
		
		return (*this);
	}
	
	template <size_t COL, size_t ROW, typename T>
	Vector<ROW, T>& Matrix<COL, ROW, T>::operator[](size_t index)
	{
		if (index >= COL)
			ATEMA_ERROR("Index is out of range.")
		
		return (this->m_columns[index]);
	}
	
	template <size_t COL, size_t ROW, typename T>
	const Vector<ROW, T>& Matrix<COL, ROW, T>::operator[](size_t index) const
	{
		if (index >= COL)
			ATEMA_ERROR("Index is out of range.")
		
		return (this->m_columns[index]);
	}
	
	template <size_t COL, size_t ROW, typename T>
	T* Matrix<COL, ROW, T>::get()
	{
		return (this->m_data);
	}
	
	template <size_t COL, size_t ROW, typename T>
	const T* Matrix<COL, ROW, T>::get() const
	{
		return (this->m_data);
	}
}

#endif