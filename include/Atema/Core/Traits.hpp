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

#ifndef ATEMA_CORE_TRAITS_HPP
#define ATEMA_CORE_TRAITS_HPP

namespace at
{
	template <size_t I>
	struct IsPowerOfTwo
	{
		static constexpr bool getValue()
		{
			return !(I == 0) && !(I & (I - 1));
		}

		static constexpr bool value = getValue();
	};

	template <typename T>
	struct IsChar
	{
		static constexpr bool value = false;
	};

	template <>
	struct IsChar<char>
	{
		static constexpr bool value = true;
	};

	template <>
	struct IsChar<wchar_t>
	{
		static constexpr bool value = true;
	};

	template <>
	struct IsChar<char16_t>
	{
		static constexpr bool value = true;
	};

	template <>
	struct IsChar<char32_t>
	{
		static constexpr bool value = true;
	};
}

#endif