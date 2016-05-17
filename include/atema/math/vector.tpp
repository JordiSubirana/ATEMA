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

#ifndef ATEMA_MATH_VECTOR_IMPLEMENTATION
#define ATEMA_MATH_VECTOR_IMPLEMENTATION

#include <cmath>

namespace at
{
	template <size_t N, typename T>
	Vector<N, T>::Vector()
	{
		T tmp = static_cast<T>(0);
		
		for (size_t i = 0; i < N; i++)
			this->data[i] = tmp;
	}
	
	template <size_t N, typename T>
	Vector<N, T>::Vector(T arg)
	{
		for (size_t i = 0; i < N; i++)
			this->data[i] = arg;
	}
	
	template <size_t N, typename T>
	Vector<N, T>::~Vector() noexcept
	{
		
	}

	template <size_t N, typename T>
	Vector<N, T>& Vector<N, T>::normalize() noexcept
	{
		return (operator /=(get_norm()));
	}
	
	template <size_t N, typename T>
	T Vector<N, T>::get_norm() const noexcept
	{
		T tmp = static_cast<T>(0);
		
		for (size_t i = 0; i < N; i++)
			tmp += this->data[i]*this->data[i];
		
		return (static_cast<T>(sqrt(tmp)));
	}

	template <size_t N, typename T>
	Vector<N, T> Vector<N, T>::operator +(const Vector<N, T>& arg) const
	{
		Vector<N, T> tmp;
		
		for (size_t i = 0; i < N; i++)
			tmp.data[i] = this->data[i] + arg.data[i];
	}
	
	template <size_t N, typename T>
	Vector<N, T> Vector<N, T>::operator -(const Vector<N, T>& arg) const
	{
		Vector<N, T> tmp;
		
		for (size_t i = 0; i < N; i++)
			tmp.data[i] = this->data[i] - arg.data[i];
	}
	
	template <size_t N, typename T>
	Vector<N, T> Vector<N, T>::operator *(const Vector<N, T>& arg) const
	{
		Vector<N, T> tmp;
		
		for (size_t i = 0; i < N; i++)
			tmp.data[i] = this->data[i] * arg.data[i];
	}
	
	template <size_t N, typename T>
	Vector<N, T> Vector<N, T>::operator /(const Vector<N, T>& arg) const
	{
		Vector<N, T> tmp;
		
		for (size_t i = 0; i < N; i++)
			tmp.data[i] = this->data[i] / arg.data[i];
	}
	
	template <size_t N, typename T>
	Vector<N, T> Vector<N, T>::operator +(T arg) const
	{
		Vector<N, T> tmp;
		
		for (size_t i = 0; i < N; i++)
			tmp.data[i] = this->data[i] + arg;
	}
	
	template <size_t N, typename T>
	Vector<N, T> Vector<N, T>::operator -(T arg) const
	{
		Vector<N, T> tmp;
		
		for (size_t i = 0; i < N; i++)
			tmp.data[i] = this->data[i] - arg;
	}
	
	template <size_t N, typename T>
	Vector<N, T> Vector<N, T>::operator *(T arg) const
	{
		Vector<N, T> tmp;
		
		for (size_t i = 0; i < N; i++)
			tmp.data[i] = this->data[i] * arg;
	}
	
	template <size_t N, typename T>
	Vector<N, T> Vector<N, T>::operator /(T arg) const
	{
		Vector<N, T> tmp;
		
		for (size_t i = 0; i < N; i++)
			tmp.data[i] = this->data[i] / arg;
	}
	
	template <size_t N, typename T>
	Vector<N, T>& Vector<N, T>::operator +=(const Vector<N, T>& arg)
	{
		for (size_t i = 0; i < N; i++)
			this->data[i] = this->data[i] + arg.data[i];
		
		return (*this);
	}
	
	template <size_t N, typename T>
	Vector<N, T>& Vector<N, T>::operator -=(const Vector<N, T>& arg)
	{
		for (size_t i = 0; i < N; i++)
			this->data[i] = this->data[i] - arg.data[i];
		
		return (*this);
	}
	
	template <size_t N, typename T>
	Vector<N, T>& Vector<N, T>::operator *=(const Vector<N, T>& arg)
	{
		for (size_t i = 0; i < N; i++)
			this->data[i] = this->data[i] * arg.data[i];
		
		return (*this);
	}
	
	template <size_t N, typename T>
	Vector<N, T>& Vector<N, T>::operator /=(const Vector<N, T>& arg)
	{
		for (size_t i = 0; i < N; i++)
			this->data[i] = this->data[i] / arg.data[i];
		
		return (*this);
	}
	
	template <size_t N, typename T>
	Vector<N, T>& Vector<N, T>::operator +=(T arg)
	{
		for (size_t i = 0; i < N; i++)
			this->data[i] = this->data[i] + arg;
		
		return (*this);
	}
	
	template <size_t N, typename T>
	Vector<N, T>& Vector<N, T>::operator -=(T arg)
	{
		for (size_t i = 0; i < N; i++)
			this->data[i] = this->data[i] - arg;
		
		return (*this);
	}
	
	template <size_t N, typename T>
	Vector<N, T>& Vector<N, T>::operator *=(T arg)
	{
		for (size_t i = 0; i < N; i++)
			this->data[i] = this->data[i] * arg;
		
		return (*this);
	}
	
	template <size_t N, typename T>
	Vector<N, T>& Vector<N, T>::operator /=(T arg)
	{
		for (size_t i = 0; i < N; i++)
			this->data[i] = this->data[i] / arg;
		
		return (*this);
	}
	
}

#endif