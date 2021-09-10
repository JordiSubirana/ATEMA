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

#ifndef ATEMA_CORE_SPARSESETUNION_INL
#define ATEMA_CORE_SPARSESETUNION_INL

#include <Atema/Core/SparseSetUnion.hpp>

namespace at
{
	template <typename ... Args>
	SparseSetUnion<Args...>::SparseSetUnion(SparseSet<Args>&... args)
	{
		m_sets = std::make_tuple((&args)...);

		build(args...);

		initialize();
	}

	template <typename ... Args>
	SparseSetUnion<Args...>::~SparseSetUnion()
	{
	}

	template <typename ... Args>
	template <typename T>
	T* SparseSetUnion<Args...>::get()
	{
		return std::get<SparseSet<T>*>(m_sets)->data();
	}

	template <typename ... Args>
	template <typename T>
	const T* SparseSetUnion<Args...>::get() const
	{
		return std::get<SparseSet<T>*>(m_sets)->data();
	}

	template <typename ... Args>
	template <typename T>
	T& SparseSetUnion<Args...>::get(size_t index)
	{
		return std::get<SparseSet<T>*>(m_sets)->operator[](index);
	}

	template <typename ... Args>
	template <typename T>
	const T& SparseSetUnion<Args...>::get(size_t index) const
	{
		return std::get<SparseSet<T>*>(m_sets)->operator[](index);
	}

	template <typename ... Args>
	size_t SparseSetUnion<Args...>::size() const
	{
		return m_commonIndices.size();
	}

	template <typename ... Args>
	typename SparseSetUnion<Args...>::IndexIterator SparseSetUnion<Args...>::begin() noexcept
	{
		return m_commonIndices.begin();
	}

	template <typename ... Args>
	typename SparseSetUnion<Args...>::ConstIndexIterator SparseSetUnion<Args...>::begin() const noexcept
	{
		return m_commonIndices.begin();
	}

	template <typename ... Args>
	typename SparseSetUnion<Args...>::IndexIterator SparseSetUnion<Args...>::end() noexcept
	{
		return m_commonIndices.end();
	}

	template <typename ... Args>
	typename SparseSetUnion<Args...>::ConstIndexIterator SparseSetUnion<Args...>::end() const noexcept
	{
		return m_commonIndices.end();
	}

	template <typename ... Args>
	template <size_t I, typename T, typename First, typename ... Others>
	constexpr size_t SparseSetUnion<Args...>::getTypeIndexImpl()
	{
		if constexpr (std::is_same<T, First>::value)
			return I;
		else if constexpr (sizeof...(Others))
			return getTypeIndexImpl<I + 1, T, Others...>();

		return -1;
	}

	template <typename ... Args>
	template <typename T>
	constexpr size_t SparseSetUnion<Args...>::getTypeIndex()
	{
		constexpr auto index = getTypeIndexImpl<0, T, Args...>();

		static_assert(index >= 0, "Unknown type requested");

		return index;
	}

	template <typename ... Args>
	template <typename T>
	void SparseSetUnion<Args...>::build(SparseSet<T>& set)
	{
		m_indices.push_back(set.getIndices().data());
		m_sizes.push_back(set.size());
		m_contains.push_back([&set](size_t index)
		{
			return set.contains(index);
		});
	}

	template <typename ... Args>
	template <typename First, typename ... Others>
	void SparseSetUnion<Args...>::build(SparseSet<First>& first, SparseSet<Others>&... others)
	{
		build(first);

		build(others...);
	}

	template <typename ... Args>
	void SparseSetUnion<Args...>::initialize()
	{
		size_t minSize = std::numeric_limits<size_t>::max();
		const size_t** indices = nullptr;

		for (size_t i = 0; i < Size; i++)
		{
			if (m_sizes[i] < minSize)
			{
				minSize = m_sizes[i];
				indices = &m_indices[i];
			}
		}

		m_commonIndices.assign(*indices, *indices + minSize);

		std::vector<size_t> commonIndicesTmp;
		commonIndicesTmp.reserve(minSize);

		for (size_t i = 0; i < Size; i++)
		{
			const auto& otherIndices = m_indices[i];
			const auto& contains = m_contains[i];

			if (*indices == otherIndices)
				continue;

			for (auto& index : m_commonIndices)
			{
				if (contains(index))
					commonIndicesTmp.push_back(index);
			}

			std::swap(commonIndicesTmp, m_commonIndices);

			commonIndicesTmp.clear();
		}
	}
}

#endif
