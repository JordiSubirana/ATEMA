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
		using size_type = typename std::vector<T>::size_type;
		static constexpr size_type InvalidIndex = std::numeric_limits<size_type>::max() - 1;

		using Iterator = typename std::vector<T>::iterator;
		using ConstIterator = typename std::vector<T>::const_iterator;
		
		SparseSet();
		~SparseSet();

		T* data() noexcept;
		const T* data() const noexcept;
		size_type* indices() noexcept;
		const size_type* indices() const noexcept;

		size_type size() const;
		size_type getSize() const;
		size_type capacity() const;
		size_type getCapacity() const;
		
		void reserve(size_type size);
		
		bool contains(size_type index) const;

		T& operator[](size_type index);
		const T& operator[](size_type index) const;

		Iterator begin() noexcept;
		ConstIterator begin() const noexcept;
		Iterator end() noexcept;
		ConstIterator end() const noexcept;

		T& emplace(size_type index);
		T& insert(size_type index, const T& value);

		void erase(size_type index);

		void clear();

	private:
		static constexpr size_type PageSize = ATEMA_SPARSESET_PAGE_SIZE;

		static_assert(IsPowerOfTwo<PageSize>::value, "Page size must be power of 2");
		
		struct Page
		{
			Page() : size(0)
			{
				for (auto& index : indices)
					index = InvalidIndex;
			}
			
			std::array<size_type, PageSize> indices;
			size_type size;
		};

		size_type getPageIndex(size_type index) const;
		size_type getOffset(size_type index) const;
		size_type getPackedIndex(size_type index) const;

		Page& checkPage(size_type index);
		T& checkElement(size_type index);
		
		std::vector<T> m_data;
		std::vector<size_t> m_indices;
		std::vector<Ptr<Page>> m_pages;
	};
}

#include <Atema/Core/SparseSet.inl>

#endif