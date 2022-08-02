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

#ifndef ATEMA_CORE_HASH_INL
#define ATEMA_CORE_HASH_INL

#include <Atema/Core/Hash.hpp>

namespace at
{
	//----- FNV-1a -----//
	// Constants (depending on ID size)
	namespace detail
	{
		template <typename HashType>
		struct FNV1aData;

		template <>
		struct FNV1aData<uint32_t>
		{
			static constexpr uint32_t prime = 16777619ul;
			static constexpr uint32_t offsetBasis = 2166136261ul;
		};

		template <>
		struct FNV1aData<uint64_t>
		{
			static constexpr uint64_t prime = 1099511628211ull;
			static constexpr uint64_t offsetBasis = 14695981039346656037ull;
		};
	}

	template <typename HashType>
	constexpr HashType FNV1a<HashType>::hash(const char* c)
	{
		auto value = detail::FNV1aData<HashType>::offsetBasis;

		while (*c != 0)
			value = (value ^ static_cast<HashType>(*(c++))) * detail::FNV1aData<HashType>::prime;

		return value;
	}

	template <typename HashType>
	constexpr HashType FNV1a<HashType>::hash(const uint8_t* data, size_t byteSize)
	{
		auto value = detail::FNV1aData<HashType>::offsetBasis;

		for (size_t i = 0; i < byteSize; i++)
			value = (value ^ static_cast<HashType>(data[i])) * detail::FNV1aData<HashType>::prime;

		return value;
	}

	//----- HASHER -----//
	// Hash combine constants
	namespace detail
	{
		template <typename HashType>
		struct HashCombineMagicNumber;

		template <>
		struct HashCombineMagicNumber<uint32_t>
		{
			static constexpr uint32_t value = 0x9e3779b9ul;
		};

		template <>
		struct HashCombineMagicNumber<uint64_t>
		{
			static constexpr uint64_t value = 0x9E3779B97F4A7C15ull;
		};
	}

	template <typename HashFunction>
	constexpr typename Hasher<HashFunction>::HashType Hasher<HashFunction>::hashBytes(const void* data, size_t byteSize)
	{
		return HashFunction::hash(static_cast<const uint8_t*>(data), byteSize);
	}

	template <typename HashFunction>
	constexpr typename Hasher<HashFunction>::HashType Hasher<HashFunction>::hash(const char* c)
	{
		return HashFunction::hash(c);
	}

	template <typename HashFunction>
	template <typename T>
	constexpr typename Hasher<HashFunction>::HashType Hasher<HashFunction>::hash(const T& object)
	{
		return hashBytes(static_cast<const void*>(&object), sizeof(T));
	}

	template <typename HashFunction>
	constexpr void Hasher<HashFunction>::hashCombine(HashType& seed)
	{
		// Nothing to do
	}

	template <typename HashFunction>
	template <typename T, typename ... Rest>
	constexpr void Hasher<HashFunction>::hashCombine(HashType& seed, const T& value, Rest... rest)
	{
		seed ^= hash(value) + detail::HashCombineMagicNumber<HashType>::value + (seed << 6) + (seed >> 2);
		hashCombine(seed, rest...);
	}

	template <typename HashFunction>
	template <typename ... Args>
	constexpr typename Hasher<HashFunction>::HashType Hasher<HashFunction>::hash(Args... args)
	{
		HashType seed = 0;

		hashCombine(seed, args...);

		return seed;
	}
}

#endif
