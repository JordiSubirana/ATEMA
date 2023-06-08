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

#ifndef ATEMA_MATH_RECT_HPP
#define ATEMA_MATH_RECT_HPP

#include <Atema/Math/Config.hpp>
#include <Atema/Math/Vector.hpp>

namespace at
{
	template <typename T>
	class Rect
	{
	public:
		Rect();
		Rect(T x, T y, T width, T height);
		Rect(const Vector2<T>& pos, const Vector2<T>& size);
		Rect(T width, T height);
		Rect(const Vector2<T>& size);
		Rect(const Rect& other) = default;
		Rect(Rect&& other) noexcept = default;
		~Rect() = default;

		Rect& operator=(const Rect& other) = default;
		Rect& operator=(Rect&& other) noexcept = default;

		Vector2<T> pos;
		Vector2<T> size;
	};

	using Recti = Rect<int>;
	using Rectu = Rect<unsigned>;
	using Rectf = Rect<float>;
	using Rectd = Rect<double>;
}

#include <Atema/Math/Rect.inl>

#endif