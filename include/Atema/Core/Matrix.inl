/*
	Copyright 2017 Jordi SUBIRANA

	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
	Software, and to permit persons to whom the Software is furnished to do so, subject
	to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
	PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
	CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
	OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef ATEMA_MATH_MATRIX_INL
#define ATEMA_MATH_MATRIX_INL

#include <Atema/Core/Matrix.hpp>
#include <Atema/Core/Error.hpp>

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
	Matrix<COL, ROW, T>::Matrix(const Matrix<COL, ROW, T>& arg)
	{
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			this->m_data[i] = arg.m_data[i];
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>::~Matrix() noexcept
	{

	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<ROW, COL, T>& Matrix<COL, ROW, T>::identity()
	{
		for (size_t c = 0; c < COL; c++)
			for (size_t r = 0; r < ROW; r++)
				this->m_columns[c][r] = (c == r ? 1 : 0);

		return *this;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<ROW, COL, T> Matrix<COL, ROW, T>::getTransposed() const noexcept
	{
		Matrix<ROW, COL, T> tmp;

		for (size_t c = 0; c < COL; c++)
			for (size_t r = 0; r < ROW; r++)
				tmp[r][c] = this->m_columns[c][r];

		return tmp;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T> Matrix<COL, ROW, T>::operator +(const Matrix<COL, ROW, T>& arg) const
	{
		Matrix<COL, ROW, T> tmp(*this);
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			tmp.m_data[i] += arg.m_data[i];

		return tmp;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T> Matrix<COL, ROW, T>::operator -(const Matrix<COL, ROW, T>& arg) const
	{
		Matrix<COL, ROW, T> tmp(*this);
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			tmp.m_data[i] -= arg.m_data[i];

		return tmp;
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

		return tmp;
	}

	template <size_t COL, size_t ROW, typename T>
	Vector<ROW, T> Matrix<COL, ROW, T>::operator *(const Vector<COL, T>& arg) const
	{
		Vector<ROW, T> tmp;

		for (size_t t_r = 0; t_r < ROW; t_r++)
		{
			for (size_t t_c = 0; t_c < COL; t_c++)
			{
				tmp[t_r] += this->m_columns[t_c][t_r] * arg[t_c];
			}
		}

		return tmp;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T> Matrix<COL, ROW, T>::operator +(T arg) const
	{
		Matrix<COL, ROW, T> tmp(*this);
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			tmp.m_data[i] += arg;

		return tmp;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T> Matrix<COL, ROW, T>::operator -(T arg) const
	{
		Matrix<COL, ROW, T> tmp(*this);
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			tmp.m_data[i] -= arg;

		return tmp;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T> Matrix<COL, ROW, T>::operator *(T arg) const
	{
		Matrix<COL, ROW, T> tmp(*this);
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			tmp.m_data[i] *= arg;

		return tmp;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T> Matrix<COL, ROW, T>::operator /(T arg) const
	{
		Matrix<COL, ROW, T> tmp(*this);
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			tmp.m_data[i] /= arg;

		return tmp;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>& Matrix<COL, ROW, T>::operator +=(const Matrix<COL, ROW, T>& arg)
	{
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			this->m_data[i] += arg.m_data[i];

		return *this;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>& Matrix<COL, ROW, T>::operator -=(const Matrix<COL, ROW, T>& arg)
	{
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			this->m_data[i] -= arg.m_data[i];

		return *this;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>& Matrix<COL, ROW, T>::operator +=(T arg)
	{
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			this->m_data[i] += arg;

		return *this;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>& Matrix<COL, ROW, T>::operator -=(T arg)
	{
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			this->m_data[i] -= arg;

		return *this;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>& Matrix<COL, ROW, T>::operator *=(T arg)
	{
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			this->m_data[i] *= arg;

		return *this;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>& Matrix<COL, ROW, T>::operator /=(T arg)
	{
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			this->m_data[i] /= arg;

		return *this;
	}

	template <size_t COL, size_t ROW, typename T>
	Vector<ROW, T>& Matrix<COL, ROW, T>::operator[](size_t index)
	{
		if (index >= COL)
			ATEMA_ERROR("Index is out of range.");

		return this->m_columns[index];
	}

	template <size_t COL, size_t ROW, typename T>
	const Vector<ROW, T>& Matrix<COL, ROW, T>::operator[](size_t index) const
	{
		if (index >= COL)
			ATEMA_ERROR("Index is out of range.");

		return this->m_columns[index];
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>& Matrix<COL, ROW, T>::operator=(const Matrix<COL, ROW, T>& arg)
	{
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			this->m_data[i] = arg.m_data[i];

		return *this;
	}

	template <size_t COL, size_t ROW, typename T>
	T* Matrix<COL, ROW, T>::get()
	{
		return this->m_data;
	}

	template <size_t COL, size_t ROW, typename T>
	const T* Matrix<COL, ROW, T>::get() const
	{
		return this->m_data;
	}
}

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

#endif