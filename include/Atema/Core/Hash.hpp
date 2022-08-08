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

#ifndef ATEMA_CORE_HASH_HPP
#define ATEMA_CORE_HASH_HPP

#include <Atema/Core/Config.hpp>

#include <utility>
#include <vector>
#include <array>

#ifndef ATEMA_HASH_SIZE
#define ATEMA_HASH_SIZE 32
#endif

// Use Hasher::hash to generate the hash
#define ATEMA_DECLARE_STD_HASH(type) \
	namespace std \
	{ \
		template<> \
		struct hash<type> \
		{ \
			std::size_t operator()(const type& object) const \
			{ \
				return at::DefaultStdHasher::hash(object); \
			} \
		}; \
	}

// Use hashCombine to generate the hash
// This macro can be used like this : ATEMA_DECLARE_STD_HASH_COMBINE(MyType, object.myVar1, object.myVar2, ...)
#define ATEMA_DECLARE_STD_HASH_COMBINE(type, ...) \
	namespace std \
	{ \
		template<> \
		struct hash<type> \
		{ \
			std::size_t operator()(const type& object) const \
			{ \
				return at::DefaultStdHasher::hash(__VA_ARGS__); \
			} \
		}; \
	}

namespace at
{
	using Hash32 = uint32_t;
	using Hash64 = uint64_t;

#if ATEMA_HASH_SIZE == 32
	using Hash = Hash32;
#elif ATEMA_HASH_SIZE == 64
	using Hash = Hash64;
#else
#error Hash size must be 32 or 64
#endif

	template <typename HashType>
	struct FNV1a
	{
		static constexpr HashType hash(const char* c);

		static constexpr HashType hash(const uint8_t* data, size_t byteSize);
	};

	template <typename HashFunction>
	struct Hasher
	{
		using HashType = decltype(std::declval<HashFunction>().hash(nullptr));

		static constexpr HashType hashBytes(const void* data, size_t byteSize);

		static constexpr HashType hash(const char* c);

		template <typename T>
		static constexpr HashType hash(const T& object);

		static constexpr void hashCombine(HashType& seed);

		template <typename T, typename... Rest>
		static constexpr void hashCombine(HashType& seed, const T& value, Rest... rest);

		template <typename... Args>
		static constexpr HashType hash(Args... args);
	};

	template <typename T>
	using DefaultHashFunction = FNV1a<T>;

	using DefaultHasher = Hasher<DefaultHashFunction<Hash>>;

	using DefaultStdHasher = Hasher<DefaultHashFunction<std::size_t>>;

	// Hash literal operator for convenience
	constexpr Hash operator""_hash(const char* str, size_t)
	{
		return DefaultHasher::hash(str);
	}
}

// Override some std hashes
namespace std
{
	template <typename T>
	struct hash<vector<T>>
	{
		std::size_t operator()(const vector<T>& object) const
		{
			return at::DefaultStdHasher::hashBytes(object.data(), object.size() * sizeof(T));
		}
	};

	template <typename T, std::size_t N>
	struct hash<array<T, N>>
	{
		std::size_t operator()(const array<T, N>& object) const
		{
			return at::DefaultStdHasher::hashBytes(object.data(), N * sizeof(T));
		}
	};
}

#include <Atema/Core/Hash.inl>

#endif