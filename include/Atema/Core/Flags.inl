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

#ifndef ATEMA_CORE_FLAGS_INL
#define ATEMA_CORE_FLAGS_INL

#include <Atema/Core/Flags.hpp>

namespace at
{
	template <typename E>
	Flags<E>::Flags() : m_value(0)
	{
	}

	template <typename E>
	Flags<E>::Flags(int value) : m_value(value)
	{
	}

	template <typename E>
	Flags<E>::Flags(E value) : m_value(static_cast<int>(value))
	{
	}

	template <typename E>
	Flags<E>::Flags(const Flags<E>& value) : m_value(value.getValue())
	{
	}

	template <typename E>
	Flags<E>::~Flags()
	{
	}

	template <typename E>
	int Flags<E>::getValue() const noexcept
	{
		return m_value;
	}

	template <typename E>
	Flags<E>::operator bool() const noexcept
	{
		return static_cast<bool>(m_value);
	}

	template <typename E>
	Flags<E> Flags<E>::operator|(E value) const noexcept
	{
		return Flags<E>(m_value | static_cast<int>(value));
	}

	template <typename E>
	Flags<E> Flags<E>::operator|(const Flags<E>& value) const noexcept
	{
		return Flags<E>(m_value | value.getValue());
	}

	template <typename E>
	Flags<E> Flags<E>::operator&(E value) const noexcept
	{
		return Flags<E>(m_value & static_cast<int>(value));
	}

	template <typename E>
	Flags<E> Flags<E>::operator&(const Flags<E>& value) const noexcept
	{
		return Flags<E>(m_value & value.getValue());
	}

	template <typename E>
	Flags<E>& Flags<E>::operator|=(E value) noexcept
	{
		m_value |= static_cast<int>(value);
		return *this;
	}

	template <typename E>
	Flags<E>& Flags<E>::operator|=(const Flags<E>& value) noexcept
	{
		m_value |= value.getValue();
		return *this;
	}

	template <typename E>
	Flags<E>& Flags<E>::operator&=(E value) noexcept
	{
		m_value &= static_cast<int>(value);
		return *this;
	}

	template <typename E>
	Flags<E>& Flags<E>::operator&=(const Flags<E>& value) noexcept
	{
		m_value &= value.getValue();
		return *this;
	}
}

#endif
