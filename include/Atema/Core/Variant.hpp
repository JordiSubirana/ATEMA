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

#ifndef ATEMA_CORE_VARIANT_HPP
#define ATEMA_CORE_VARIANT_HPP

#include <variant>

namespace at
{
	template <typename ... Args>
	class Variant
	{
	public:
		Variant();
		Variant(Variant<Args...>& other);
		Variant(const Variant<Args...>& other);
		Variant(Variant<Args...>&& other) noexcept;
		Variant(const Variant<Args...>&& other) noexcept;
		template <typename T>
		Variant(T&& value);
		
		~Variant();

		template <typename T>
		bool is() const noexcept;

		template <typename T>
		T& get();

		template <typename T>
		const T& get() const;

		Variant<Args...>& operator=(Variant<Args...>& other);
		Variant<Args...>& operator=(const Variant<Args...>& other);
		Variant<Args...>& operator=(Variant<Args...>&& other) noexcept;
		Variant<Args...>& operator=(const Variant<Args...>&& other) noexcept;
		template <typename T>
		Variant<Args...>& operator=(T&& value);

	private:
		std::variant<Args...> m_value;
	};
}

#include <Atema/Core/Variant.inl>

#endif
