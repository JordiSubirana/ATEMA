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

#ifndef ATEMA_CORE_VARIANT_INL
#define ATEMA_CORE_VARIANT_INL

#include <Atema/Core/Variant.hpp>

namespace at
{
	template <typename ... Args>
	Variant<Args...>::Variant() :
		m_value()
	{
	}

	template <typename ... Args>
	Variant<Args...>::Variant(Variant<Args...>& other) :
		m_value(other.m_value)
	{
	}

	template <typename ... Args>
	Variant<Args...>::Variant(const Variant<Args...>& other) :
		m_value(other.m_value)
	{
	}

	template <typename ... Args>
	Variant<Args...>::Variant(Variant<Args...>&& other) noexcept :
		m_value(std::move(other.m_value))
	{
	}

	template <typename ... Args>
	Variant<Args...>::Variant(const Variant<Args...>&& other) noexcept :
		m_value(std::move(other.m_value))
	{
	}

	template <typename ... Args>
	template <typename T>
	Variant<Args...>::Variant(T&& value) :
		m_value(std::forward<T>(value))
	{
	}

	template <typename ... Args>
	Variant<Args...>::~Variant()
	{
	}

	template <typename ... Args>
	template <typename T>
	bool Variant<Args...>::is() const noexcept
	{
		return std::holds_alternative<T>(m_value);
	}

	template <typename ... Args>
	template <typename First, typename ... Rest>
	bool Variant<Args...>::isOneOf() const noexcept
	{
		if constexpr (sizeof...(Rest) > 0)
			return is<First>() || isOneOf<Rest...>();

		return is<First>();
	}

	template <typename ... Args>
	template <typename T>
	T& Variant<Args...>::get()
	{
		return std::get<T>(m_value);
	}

	template <typename ... Args>
	template <typename T>
	const T& Variant<Args...>::get() const
	{
		return std::get<T>(m_value);
	}

	template <typename ... Args>
	Variant<Args...>& Variant<Args...>::operator=(Variant<Args...>& other)
	{
		m_value = other.m_value;

		return *this;
	}

	template <typename ... Args>
	Variant<Args...>& Variant<Args...>::operator=(const Variant<Args...>& other)
	{
		m_value = other.m_value;
		
		return *this;
	}

	template <typename ... Args>
	Variant<Args...>& Variant<Args...>::operator=(Variant<Args...>&& other) noexcept
	{
		m_value = std::move(other.m_value);

		return *this;
	}

	template <typename ... Args>
	Variant<Args...>& Variant<Args...>::operator=(const Variant<Args...>&& other) noexcept
	{
		m_value = std::move(other.m_value);

		return *this;
	}

	template <typename ... Args>
	template <typename T>
	Variant<Args...>& Variant<Args...>::operator=(T&& value)
	{
		m_value = std::forward<T>(value);

		return *this;
	}

	template <typename ... Args>
	bool Variant<Args...>::operator==(const Variant& other) const
	{
		return m_value == other.m_value;
	}

	template <typename ... Args>
	template <typename T>
	bool Variant<Args...>::operator==(const T& value) const noexcept
	{
		if (is<T>())
		{
			return get<T>() == value;
		}

		return false;
	}
}

#endif