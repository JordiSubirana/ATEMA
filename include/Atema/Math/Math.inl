/*
	Copyright 2022 Jordi SUBIRANA

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

#ifndef ATEMA_MATH_MATH_INL
#define ATEMA_MATH_MATH_INL

#include <Atema/Math/Math.hpp>

#include <algorithm>

namespace at
{
	namespace Math
	{
		template <typename T>
		constexpr bool equals(T a, T b, T epsilon)
		{
			if (a < b)
				std::swap(a, b);

			return (a - b) <= epsilon;
		}

		template <typename T>
		constexpr T toRadians(T degrees)
		{
			return degrees * Pi<T> / static_cast<T>(180);
		}

		template <typename T>
		constexpr T toDegrees(T radians)
		{
			return radians * static_cast<T>(180) / Pi<T>;
		}

		template <typename T>
		bool isInside(T value, T min, T max) noexcept
		{
			return min <= value && value <= max;
		}

		template <typename T>
		bool isStrictlyInside(T value, T min, T max) noexcept
		{
			return min < value && value < max;
		}

		template <typename T>
		T getNextMultiple(T value, T multiple)
		{
			static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value, "getNextMultiple is only valid for positive integral types");
			ATEMA_ASSERT(multiple != static_cast<T>(0), "Can't find next multiple of zero");

			//https://stackoverflow.com/questions/44110606/rounding-off-a-positive-number-to-the-next-nearest-multiple-of-5
			return (value + multiple - 1) / multiple * multiple;
		}
	}
}

#endif
