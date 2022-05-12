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

#ifndef ATEMA_CORE_SPARSESETUNION_HPP
#define ATEMA_CORE_SPARSESETUNION_HPP

#include <Atema/Core/Config.hpp>
#include <Atema/Core/SparseSet.hpp>

#include <tuple>
#include <vector>
#include <functional>

namespace at
{
	template <typename ... Args>
	class SparseSetUnion
	{
		static constexpr size_t Size = (sizeof...(Args));

		static_assert(Size > 0, "SparseSetUnion types size must be at least 1");
	
	public:
		using IndexIterator = typename std::vector<size_t>::iterator;
		using ConstIndexIterator = typename std::vector<size_t>::const_iterator;
		
		SparseSetUnion() = delete;
		SparseSetUnion(SparseSet<Args>&... args);
		~SparseSetUnion();

		template <typename T>
		T* get();
		template <typename T>
		const T* get() const;

		template <typename T>
		T& get(size_t index);
		template <typename T>
		const T& get(size_t index) const;

		size_t size() const;

		IndexIterator begin() noexcept;
		ConstIndexIterator begin() const noexcept;
		IndexIterator end() noexcept;
		ConstIndexIterator end() const noexcept;

	private:
		template <size_t I, typename T, typename First, typename ... Others>
		static constexpr size_t getTypeIndexImpl();

		template <typename T>
		static constexpr size_t getTypeIndex();

		template <typename T>
		void build(SparseSet<T>& set);

		template <typename First, typename ... Others>
		void build(SparseSet<First>& first, SparseSet<Others>&... others);

		void initialize();

		std::tuple<SparseSet<Args>*...> m_sets;
		std::vector<const size_t*> m_indices;
		std::vector<size_t> m_sizes;
		std::vector<size_t> m_commonIndices;
		std::vector<std::function<bool(size_t)>> m_contains;
	};
}

#include <Atema/Core/SparseSetUnion.inl>

#endif
