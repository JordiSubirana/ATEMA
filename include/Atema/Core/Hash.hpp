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
#include <Atema/Core/Traits.hpp>

#include <utility>
#include <string>

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
				StdHash hash = 0; \
				at::DefaultStdHasher::hashCombine(hash, __VA_ARGS__); \
				return hash; \
			} \
		}; \
	}

namespace at
{
	// 16-bits length hash type
	using Hash16 = uint16_t;

	// 32-bits length hash type
	using Hash32 = uint32_t;

	// 64-bits length hash type
	using Hash64 = uint64_t;

	// Default std hash type
	using StdHash = size_t;

#if ATEMA_FORCE_HASH16
	// Default hash used by the engine (forced to Hash16)
	// Can be forced defining ATEMA_FORCE_HASH16 / ATEMA_FORCE_HASH32 / ATEMA_FORCE_HASH64
	using Hash = Hash16;
#elif ATEMA_FORCE_HASH32
	// Default hash used by the engine (forced to Hash32)
	// Can be forced defining ATEMA_FORCE_HASH16 / ATEMA_FORCE_HASH32 / ATEMA_FORCE_HASH64
	using Hash = Hash32;
#elif ATEMA_FORCE_HASH64
	// Default hash used by the engine (forced to Hash64)
	// Can be forced defining ATEMA_FORCE_HASH16 / ATEMA_FORCE_HASH32 / ATEMA_FORCE_HASH64
	using Hash = Hash64;
#else
	// Default hash used by the engine (StdHash)
	// Can be forced defining ATEMA_FORCE_HASH16 / ATEMA_FORCE_HASH32 / ATEMA_FORCE_HASH64
	using Hash = StdHash;
#endif

	// FNV-1a hash function
	template <typename HashType>
	struct FNV1a
	{
		template <typename T, typename U = T>
		static constexpr std::enable_if_t<sizeof(U) && std::is_integral_v<T>, HashType> hash(const T* data, size_t size);
	};

	template <>
	struct FNV1a<Hash16>
	{
		template <typename T, typename U = T>
		static constexpr std::enable_if_t<sizeof(U) && std::is_integral_v<T>, Hash16> hash(const T* data, size_t size);
	};

	/*
	Defines the hash behaviour for a given type.
	The default behaviour is to hash the object bytes.
	To override this behaviour, specialize this struct as follows :
	template <>
	struct HashOverload<MyType>
	{
		template <typename Hasher>
		static constexpr auto hash(const MyType& object)
		{
			// For example here we are combining 2 members
			typename Hasher::HashType hash = 0;

			Hasher::hashCombine(hash, object.myIntMember);
			Hasher::hashCombine(hash, object.myStrMember);
			
			return hash;
		}
	};
	*/
	template <typename T>
	struct HashOverload
	{
		template <typename Hasher>
		static constexpr auto hash(const T& object)
		{
			return Hasher::hash(static_cast<const void*>(&object), sizeof(T));
		}
	};

	// Hasher using a hash function.
	// Given HashType being an integral type, HashFunction must be a struct defining :
	//
	// template <typename T, typename U = T>
	// static constexpr std::enable_if_t<sizeof(U) && std::is_integral_v<T>, HashType> hash(const T* data, size_t size);
	template <typename HashFunction>
	struct Hasher
	{
		// Return type of HashFunction::hash(const T* data, size_t size)
		using HashType = decltype(std::declval<HashFunction>().hash(static_cast<const int*>(nullptr), 0));

		// Hash overload for integral C-style arrays
		template <typename T, typename U = T>
		static constexpr std::enable_if_t<sizeof(U) && std::is_integral_v<T>, HashType> hash(const T* data, size_t size);

		// Hash overload for C-style strings
		template <typename T, typename U = T>
		static constexpr std::enable_if_t<sizeof(U) && IsChar<T>::value, HashType> hash(const T* str);

		// Hash overload for abstract data, considered as a C-style byte array
		static constexpr HashType hash(const void* data, size_t byteSize);

		// Hash overload for any object, the hash is generated from hashing the bytes of the object
		template <typename T>
		static constexpr HashType hash(const T& object);

		// Fallback for hashCombine
		static constexpr void hashCombine(HashType& seed);

		// Hashes multiple objects and combine their hash to a seed
		template <typename T, typename... Rest>
		static constexpr void hashCombine(HashType& seed, const T& value, Rest... rest);

		// Convenience non-static method to be used for std containers
		template <typename T>
		constexpr HashType operator()(const T& object) const;

		// Convenience non-static method
		template <typename... Args>
		constexpr HashType operator()(Args... args) const;
	};

	template <typename T>
	using DefaultHashFunction = FNV1a<T>;

	using DefaultHasher = Hasher<DefaultHashFunction<Hash>>;

	using DefaultStdHasher = Hasher<DefaultHashFunction<StdHash>>;

	// Hash literal operator for convenience
	constexpr Hash operator""_hash(const char* str, size_t)
	{
		return DefaultHasher::hash(str);
	}

	// Override some hashes

	// std::string hash overload
	template<class CharT, class Traits, class Allocator>
	struct HashOverload<std::basic_string<CharT, Traits, Allocator>>
	{
		template <typename Hasher>
		static constexpr auto hash(const std::basic_string<CharT, Traits, Allocator>& str)
		{
			return Hasher::hash(str.c_str(), str.size());
		}
	};

	// std::string_view hash overload
	template<class CharT, class Traits>
	struct HashOverload<std::basic_string_view<CharT, Traits>>
	{
		template <typename Hasher>
		static constexpr auto hash(const std::basic_string_view<CharT, Traits>& str)
		{
			return Hasher::hash(str.data(), str.size());
		}
	};
}

#include <Atema/Core/Hash.inl>

#endif