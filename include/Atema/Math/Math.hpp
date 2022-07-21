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

#ifndef ATEMA_MATH_MATH_HPP
#define ATEMA_MATH_MATH_HPP

#include <Atema/Math/Config.hpp>

#include <limits>

namespace at
{
	namespace Math
	{
		// Constants
		template <typename T>
		constexpr T Pi = static_cast<T>(3.1415926535897932384626433832795);
		template <typename T>
		constexpr T HalfPi = static_cast<T>(1.5707963267948966192313216916398);
		template <typename T>
		constexpr T Epsilon = std::numeric_limits<T>::epsilon();

		// Functions
		template <typename T>
		constexpr bool equals(T a, T b, T epsilon = Epsilon<T>);

		template <typename T>
		constexpr T toRadians(T degrees);

		template <typename T>
		constexpr T toDegrees(T radians);

		template <typename T>
		constexpr T pow(T base, T exponent);

		template <typename T>
		constexpr T mod(T num, T den);

		template <typename T>
		bool isInside(T value, T min, T max) noexcept;

		template <typename T>
		bool isStrictlyInside(T value, T min, T max) noexcept;

		template <typename T>
		T getNextMultiple(T value, T multiple);
	}
}

#include <Atema/Math/Math.inl>

#endif
