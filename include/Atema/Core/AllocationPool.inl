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
#include <Atema/Core/Error.hpp>
#include <Atema/Core/Utils.hpp>

namespace at
{
	template <typename AllocationType, typename PageResources>
	AllocationPool<AllocationType, PageResources>::AllocationPool(size_t pageSize, bool releaseOnClear) :
		m_pageSize(pageSize),
		m_pageAlignment(1),
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
		Ptr<AllocationType> allocation;

		// Find a page with enough remaining memory
		for (size_t pageIndex = 0; pageIndex < m_pages.size(); pageIndex++)
		{
			const size_t offset = m_pages[pageIndex]->allocate(size);

			if (offset != AllocationPage::InvalidOffset)
			{
				allocation = createAllocation(*m_pageResources[pageIndex], pageIndex, offset, size);

				break;
			}
		}

		// No page found, create a new one capable of storing enough data
		if (!allocation)
		{
			const auto pageSize = std::max(m_pageSize, size);

			auto& page = m_pages.emplace_back(std::make_unique<AllocationPage>(pageSize, m_pageAlignment, m_releaseOnClear));
			auto& pageResources = m_pageResources.emplace_back(createPageResources(pageSize));

			const size_t offset = page->allocate(size);

			allocation = createAllocation(*pageResources, m_pages.size() - 1, offset, size);
		}

		// Automatically release allocation if we are not on global release on clear
		if (!m_releaseOnClear)
		{
			AllocationType* allocationPtr = allocation.get();

			m_connectionGuard.connect(allocation->onDestroy, [this, allocationPtr]()
				{
					release(*allocationPtr);
				});
		}

		return allocation;
	}

	template <typename AllocationType, typename PageResources>
	size_t AllocationPool<AllocationType, PageResources>::getAllocationCount(size_t pageIndex) const
	{
		ATEMA_ASSERT(pageIndex < m_pages.size(), "Invalid page index");

		return m_pages[pageIndex]->getAllocationCount();
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

		// We don't want previous allocations to free the current content
		m_connectionGuard.disconnect();
	}

	template <typename AllocationType, typename PageResources>
	void AllocationPool<AllocationType, PageResources>::initialize(size_t pageAlignment)
	{
		ATEMA_ASSERT(pageAlignment > 1, "Page alignment must be a at least one");
		ATEMA_ASSERT(isPowerOfTwo(pageAlignment), "Page alignment must be a power of two");

		m_pageAlignment = pageAlignment;
	}
}

#endif
