/*
	Copyright 2021 Jordi SUBIRANA

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

#ifndef ATEMA_CORE_VECTOR_INL
#define ATEMA_CORE_VECTOR_INL

#include <Atema/Core/Vector.hpp>
#include <Atema/Core/Error.hpp>

namespace at
{
	template <size_t N, typename T>
	Vector<N, T>::Vector()
	{
	}

	template <size_t N, typename T>
	Vector<N, T>::~Vector() noexcept
	{

	}

	template <size_t N, typename T>
	Vector<N, T>& Vector<N, T>::normalize() noexcept
	{
		return (operator /=(getNorm()));
	}

	template <size_t N, typename T>
	T Vector<N, T>::getNorm() const noexcept
	{
		T tmp = static_cast<T>(0);

		for (size_t i = 0; i < N; i++)
			tmp += this->data[i] * this->data[i];

		return (static_cast<T>(sqrt(tmp)));
	}

	template <size_t N, typename T>
	Vector<N, T> Vector<N, T>::getNormalized() const noexcept
	{
		return (*this) / getNorm();
	}

	template <size_t N, typename T>
	Vector<N, T> Vector<N, T>::operator +(const Vector<N, T>& arg) const
	{
		Vector<N, T> tmp;

		for (size_t i = 0; i < N; i++)
			tmp.data[i] = this->data[i] + arg.data[i];

		return (tmp);
	}

	template <size_t N, typename T>
	Vector<N, T> Vector<N, T>::operator -(const Vector<N, T>& arg) const
	{
		Vector<N, T> tmp;

		for (size_t i = 0; i < N; i++)
			tmp.data[i] = this->data[i] - arg.data[i];

		return (tmp);
	}

	template <size_t N, typename T>
	Vector<N, T> Vector<N, T>::operator *(const Vector<N, T>& arg) const
	{
		Vector<N, T> tmp;

		for (size_t i = 0; i < N; i++)
			tmp.data[i] = this->data[i] * arg.data[i];

		return (tmp);
	}

	template <size_t N, typename T>
	Vector<N, T> Vector<N, T>::operator /(const Vector<N, T>& arg) const
	{
		Vector<N, T> tmp;

		for (size_t i = 0; i < N; i++)
			tmp.data[i] = this->data[i] / arg.data[i];

		return (tmp);
	}

	template <size_t N, typename T>
	Vector<N, T> Vector<N, T>::operator +(T arg) const
	{
		Vector<N, T> tmp;

		for (size_t i = 0; i < N; i++)
			tmp.data[i] = this->data[i] + arg;

		return (tmp);
	}

	template <size_t N, typename T>
	Vector<N, T> Vector<N, T>::operator -(T arg) const
	{
		Vector<N, T> tmp;

		for (size_t i = 0; i < N; i++)
			tmp.data[i] = this->data[i] - arg;

		return (tmp);
	}

	template <size_t N, typename T>
	Vector<N, T> Vector<N, T>::operator *(T arg) const
	{
		Vector<N, T> tmp;

		for (size_t i = 0; i < N; i++)
			tmp.data[i] = this->data[i] * arg;

		return (tmp);
	}

	template <size_t N, typename T>
	Vector<N, T> Vector<N, T>::operator /(T arg) const
	{
		Vector<N, T> tmp;

		for (size_t i = 0; i < N; i++)
			tmp.data[i] = this->data[i] / arg;

		return (tmp);
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

	template <size_t N, typename T>
	T& Vector<N, T>::operator[](size_t index)
	{
		if (index >= N)
			ATEMA_ERROR("Index is out of range.");

			return (this->data[index]);
	}

	template <size_t N, typename T>
	const T& Vector<N, T>::operator[](size_t index) const
	{
		if (index >= N)
			ATEMA_ERROR("Index is out of range.");

			return (this->data[index]);
	}
}

#endif