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

#ifndef ATEMA_CORE_HASH_HPP
#define ATEMA_CORE_HASH_HPP

#include <Atema/Core/Config.hpp>

#include <cstdint>
#include <cstddef>

#ifndef ATEMA_HASH_SIZE
#define ATEMA_HASH_SIZE 32
#endif

namespace at
{
#if ATEMA_HASH_SIZE == 32
	using HashType = std::uint32_t;
#elif ATEMA_HASH_SIZE == 64
	using HashType = std::uint64_t;
#endif

	// FNV-1A
	namespace detail
	{
		// Constants (depending on ID size)
		template <typename T>
		struct fnv1a_data;

		template <>
		struct fnv1a_data<std::uint32_t>
		{
			using type = std::uint32_t;
			static constexpr type prime = 16777619ul;
			static constexpr type offset_basis = 2166136261ul;
		};

		template <>
		struct fnv1a_data<std::uint64_t>
		{
			using type = std::uint64_t;
			static constexpr type prime = 1099511628211ull;
			static constexpr type offset_basis = 14695981039346656037ull;
		};

		// Algorithm
		template <typename T>
		struct fnv1a
		{
			using data = fnv1a_data<T>;

			static constexpr T hash(const char* c)
			{
				auto value = data::offset_basis;

				while (*c != 0)
				{
					value = (value ^ static_cast<T>(*(c++))) * data::prime;
				}

				return value;
			}
		};
	}

	using fnv1a = detail::fnv1a<HashType>;

	// String hash
	//TODO: Make this custom
	using StringHasher = fnv1a;

	constexpr HashType stringHash(const char* str)
	{
		return StringHasher::hash(str);
	}

	// Hash literal operator for convenience
	constexpr HashType operator"" _hash(const char* str, std::size_t)
	{
		return stringHash(str);
	}
}

#endif