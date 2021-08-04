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

#ifndef ATEMA_CORE_FLAGS_HPP
#define ATEMA_CORE_FLAGS_HPP

#define ATEMA_DECLARE_FLAGS(at_type) \
	inline at::Flags<at_type> operator|(at_type a, at_type b) \
	{ \
		return at::Flags<at_type>(static_cast<int>(a) | static_cast<int>(b)); \
	} \
	inline at::Flags<at_type> operator&(at_type a, at_type b) \
	{ \
		return at::Flags<at_type>(static_cast<int>(a) & static_cast<int>(b)); \
	}

namespace at
{
	template <typename E>
	class Flags
	{
		static_assert(std::is_enum<E>::value, "Flags class requires an enum");
	
	public:
		Flags();
		Flags(int value);
		Flags(E value);
		Flags(const Flags<E>& value);
		~Flags();

		int getValue() const noexcept;

		operator bool() const noexcept;

		Flags operator|(E value) const noexcept;
		Flags operator|(const Flags<E>& value) const noexcept;
		Flags operator&(E value) const noexcept;
		Flags operator&(const Flags<E>& value) const noexcept;
		Flags& operator|=(E value) noexcept;
		Flags& operator|=(const Flags<E>& value) noexcept;
		Flags& operator&=(E value) noexcept;
		Flags& operator&=(const Flags<E>& value) noexcept;
		
	private:
		int m_value;
	};
}

#include <Atema/Core/Flags.inl>

#endif
