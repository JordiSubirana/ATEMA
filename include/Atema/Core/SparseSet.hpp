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

#ifndef ATEMA_CORE_SPARSESET_HPP
#define ATEMA_CORE_SPARSESET_HPP

#include <Atema/Core/Config.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Core/Traits.hpp>

#include <vector>
#include <array>

#ifndef ATEMA_SPARSESET_PAGE_SIZE
#define ATEMA_SPARSESET_PAGE_SIZE 4096
#endif

namespace at
{
	template <typename T>
	class SparseSet
	{
	public:
		static constexpr size_t InvalidIndex = std::numeric_limits<size_t>::max() - 1;

		using Iterator = typename std::vector<T>::iterator;
		using ConstIterator = typename std::vector<T>::const_iterator;
		
		SparseSet();
		~SparseSet();

		T* data() noexcept;
		T* getData() noexcept;
		const T* data() const noexcept;
		const T* getData() const noexcept;
		
		const std::vector<size_t>& getIndices() const noexcept;

		size_t size() const;
		size_t getSize() const;
		size_t capacity() const;
		size_t getCapacity() const;
		
		void reserve(size_t size);
		
		bool contains(size_t index) const;

		T& operator[](size_t index);
		const T& operator[](size_t index) const;

		Iterator begin() noexcept;
		ConstIterator begin() const noexcept;
		Iterator end() noexcept;
		ConstIterator end() const noexcept;

		T& emplace(size_t index);
		T& insert(size_t index, const T& value);

		void erase(size_t index);

		void clear();

	private:
		static constexpr size_t PageSize = ATEMA_SPARSESET_PAGE_SIZE;

		static_assert(IsPowerOfTwo<PageSize>::value, "Page size must be power of 2");
		
		struct Page
		{
			Page() : size(0)
			{
				for (auto& index : indices)
					index = InvalidIndex;
			}
			
			std::array<size_t, PageSize> indices;
			size_t size;
		};

		size_t getPageIndex(size_t index) const;
		size_t getOffset(size_t index) const;
		size_t getPackedIndex(size_t index) const;

		Page& checkPage(size_t index);
		T& checkElement(size_t index);
		
		std::vector<T> m_data;
		std::vector<size_t> m_indices;
		std::vector<Ptr<Page>> m_pages;
	};
}

#include <Atema/Core/SparseSet.inl>

#endif