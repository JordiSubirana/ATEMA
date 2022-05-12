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

#ifndef ATEMA_CORE_SPARSESET_INL
#define ATEMA_CORE_SPARSESET_INL

#include <Atema/Core/SparseSet.hpp>

namespace at
{
	template<typename T>
	SparseSet<T>::SparseSet()
	{
	}
	template<typename T>
	SparseSet<T>::~SparseSet()
	{
	}

	template <typename T>
	T* SparseSet<T>::data() noexcept
	{
		return getData();
	}

	template <typename T>
	T* SparseSet<T>::getData() noexcept
	{
		return m_data.data();
	}

	template <typename T>
	const T* SparseSet<T>::data() const noexcept
	{
		return getData();
	}

	template <typename T>
	const T* SparseSet<T>::getData() const noexcept
	{
		return m_data.data();
	}

	template <typename T>
	const std::vector<size_t>& SparseSet<T>::getIndices() const noexcept
	{
		return m_indices;
	}

	template <typename T>
	size_t SparseSet<T>::size() const
	{
		return getSize();
	}

	template <typename T>
	size_t SparseSet<T>::getSize() const
	{
		return m_data.size();
	}

	template <typename T>
	size_t SparseSet<T>::capacity() const
	{
		return getCapacity();
	}

	template <typename T>
	size_t SparseSet<T>::getCapacity() const
	{
		return m_data.capacity();
	}

	template <typename T>
	void SparseSet<T>::reserve(size_t size)
	{
		m_data.reserve(size);
	}

	template <typename T>
	bool SparseSet<T>::contains(size_t index) const
	{
		const auto pageIndex = getPageIndex(index);
		const auto offset = getOffset(index);
		
		return m_pages.size() > pageIndex && m_pages[pageIndex] && m_pages[pageIndex]->indices[offset] != InvalidIndex;
	}

	template <typename T>
	T& SparseSet<T>::operator[](size_t index)
	{
		return m_data[getPackedIndex(index)];
	}

	template <typename T>
	const T& SparseSet<T>::operator[](size_t index) const
	{
		return m_data[getPackedIndex(index)];
	}

	template <typename T>
	typename SparseSet<T>::Iterator SparseSet<T>::begin() noexcept
	{
		return m_data.begin();
	}

	template <typename T>
	typename SparseSet<T>::ConstIterator SparseSet<T>::begin() const noexcept
	{
		return m_data.begin();
	}

	template <typename T>
	typename SparseSet<T>::Iterator SparseSet<T>::end() noexcept
	{
		return m_data.end();
	}

	template <typename T>
	typename SparseSet<T>::ConstIterator SparseSet<T>::end() const noexcept
	{
		return m_data.end();
	}

	template <typename T>
	T& SparseSet<T>::emplace(size_t index)
	{
		return checkElement(index);
	}

	template <typename T>
	T& SparseSet<T>::insert(size_t index, const T& value)
	{
		auto& element = checkElement(index);

		element = value;

		return element;
	}

	template <typename T>
	void SparseSet<T>::erase(size_t index)
	{
		const auto pageIndex = getPageIndex(index);
		const auto offset = getOffset(index);

		// Ensure index validity
		if (pageIndex < m_pages.size() && m_pages[pageIndex])
		{
			auto& page = m_pages[pageIndex];

			const auto currentIndex = page->indices[offset];

			// Ensure index validity
			if (currentIndex != InvalidIndex)
			{
				// Get indices
				const auto lastIndex = m_indices.back();

				// Replace current element by last element then pop (except if this is the last, just pop)
				if (index != lastIndex)
				{
					// Swap data
					m_data[currentIndex] = m_data.back();
					m_indices[currentIndex] = lastIndex;

					// Update page index (we know the page exists)
					auto& lastElementPage = m_pages[getPageIndex(lastIndex)];
					lastElementPage->indices[getOffset(lastIndex)] = currentIndex;
				}

				m_data.pop_back();
				m_indices.pop_back();

				// Don't forget to invalidate erased page index
				page->indices[offset] = InvalidIndex;

				// We just removed an element to this page
				page->size--;

				// If the page does not track anything anymore, remove it
				if (page->size == 0)
				{
					page.reset();
				}
			}
		}
	}

	template <typename T>
	void SparseSet<T>::clear()
	{
		m_data.clear();
		m_indices.clear();
		m_pages.clear();
	}

	template <typename T>
	size_t SparseSet<T>::getPageIndex(size_t index) const
	{
		return index / PageSize;
	}

	template <typename T>
	size_t SparseSet<T>::getOffset(size_t index) const
	{
		return index & (PageSize - 1);
	}

	template <typename T>
	size_t SparseSet<T>::getPackedIndex(size_t index) const
	{
		return m_pages[getPageIndex(index)]->indices[getOffset(index)];
	}

	template <typename T>
	typename SparseSet<T>::Page& SparseSet<T>::checkPage(size_t index)
	{
		// Ensure there is a valid slot for the page
		if (m_pages.size() <= index)
		{
			m_pages.resize(index + 1);
		}

		// Ensure the page is initialized
		if (!m_pages[index])
		{
			m_pages[index] = std::make_unique<Page>();
		}

		return *(m_pages[index]);
	}

	template <typename T>
	T& SparseSet<T>::checkElement(size_t index)
	{
		const auto pageIndex = getPageIndex(index);
		const auto offset = getOffset(index);

		auto& page = checkPage(pageIndex);

		// Ensure the element is created
		if (page.indices[offset] == InvalidIndex)
		{
			m_data.resize(m_data.size() + 1);
			m_indices.push_back(index);

			page.indices[offset] = m_data.size() - 1;
			page.size++;

			return m_data.back();
		}

		return m_data[page.indices[offset]];
	}
}

#endif