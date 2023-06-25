/*
	Copyright 2023 Jordi SUBIRANA

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

#ifndef ATEMA_CORE_ALLOCATIONPOOL_INL
#define ATEMA_CORE_ALLOCATIONPOOL_INL

#include <Atema/Core/AllocationPool.hpp>

namespace at
{
	template <typename AllocationType, typename PageResources>
	AllocationPool<AllocationType, PageResources>::AllocationPool(size_t pageSize, bool releaseOnClear) :
		m_pageSize(pageSize),
		m_releaseOnClear(releaseOnClear)
	{
	}

	template <typename AllocationType, typename PageResources>
	bool AllocationPool<AllocationType, PageResources>::releaseOnClear() const noexcept
	{
		return m_releaseOnClear;
	}

	template <typename AllocationType, typename PageResources>
	Ptr<AllocationType> AllocationPool<AllocationType, PageResources>::allocate(size_t size)
	{
		// Find a page with enough remaining memory
		for (size_t pageIndex = 0; pageIndex < m_pages.size(); pageIndex++)
		{
			auto& page = m_pages[pageIndex];

			const auto offset = page->allocate(size);

			if (offset != AllocationPage::InvalidOffset)
				return createAllocation(*m_pageResources[pageIndex], pageIndex, offset, size);
		}

		// No page found, create a new one capable of storing enough data
		const auto pageSize = std::max(m_pageSize, size);

		auto& page = m_pages.emplace_back(std::make_unique<AllocationPage>(pageSize, m_releaseOnClear));
		auto& pageResources = m_pageResources.emplace_back(createPageResources(pageSize));

		const auto offset = page->allocate(size);

		return createAllocation(*pageResources, m_pages.size() - 1, offset, size);
	}

	template <typename AllocationType, typename PageResources>
	void AllocationPool<AllocationType, PageResources>::release(const Allocation& allocation)
	{
		const auto pageIndex = allocation.getPage();

		if (m_pages.size() < pageIndex)
		{
			m_pages[pageIndex]->release(allocation.getOffset(), allocation.getSize());

			releaseResources(*m_pageResources[pageIndex], allocation.getOffset(), allocation.getSize());
		}
	}

	template <typename AllocationType, typename PageResources>
	void AllocationPool<AllocationType, PageResources>::clear()
	{
		for (size_t pageIndex = 0; pageIndex < m_pages.size(); pageIndex++)
		{
			m_pages[pageIndex]->clear();

			clearResources(*m_pageResources[pageIndex]);
		}
	}
}

#endif
