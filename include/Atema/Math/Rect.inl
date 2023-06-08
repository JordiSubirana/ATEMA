/*
	Copyright 2023 Jordi SUBIRANA

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

#ifndef ATEMA_MATH_RECT_INL
#define ATEMA_MATH_RECT_INL

#include <Atema/Math/Rect.hpp>

namespace at
{

	template <typename T>
	Rect<T>::Rect() :
		pos(static_cast<T>(0), static_cast<T>(0)),
		size(static_cast<T>(0), static_cast<T>(0))
	{
	}

	template <typename T>
	Rect<T>::Rect(T x, T y, T width, T height) :
		pos(x, y),
		size(width, height)
	{
	}

	template <typename T>
	Rect<T>::Rect(const Vector2<T>& pos, const Vector2<T>& size) :
		pos(pos),
		size(size)
	{
	}

	template <typename T>
	Rect<T>::Rect(T width, T height) :
		pos(static_cast<T>(0), static_cast<T>(0)),
		size(width, height)
	{
	}

	template <typename T>
	Rect<T>::Rect(const Vector2<T>& size) :
		pos(static_cast<T>(0), static_cast<T>(0)),
		size(size)
	{
	}
}

#endif