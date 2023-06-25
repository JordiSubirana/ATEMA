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

#ifndef ATEMA_CORE_ALLOCATIONPOOL_HPP
#define ATEMA_CORE_ALLOCATIONPOOL_HPP

#include <Atema/Core/Config.hpp>
#include <Atema/Core/Pointer.hpp>

#include <map>
#include <vector>

namespace at
{
	class ATEMA_CORE_API Allocation
	{
	public:
		Allocation() = delete;
		Allocation(size_t page, size_t offset, size_t size);
		Allocation(const Allocation& other) = default;
		Allocation(Allocation&& other) noexcept = default;
		~Allocation() = default;

		size_t getPage() const noexcept;
		size_t getOffset() const noexcept;
		size_t getSize() const noexcept;

		Allocation& operator=(const Allocation& other) = default;
		Allocation& operator=(Allocation&& other) noexcept = default;

	private:
		size_t m_page;
		size_t m_offset;
		size_t m_size;
	};

	class ATEMA_CORE_API AllocationPage
	{
	public:
		static constexpr size_t InvalidOffset = std::numeric_limits<size_t>::max();

		AllocationPage() = delete;
		AllocationPage(size_t size, bool releaseOnClear);
		AllocationPage(const AllocationPage& other) = default;
		AllocationPage(AllocationPage&& other) noexcept = default;
		~AllocationPage() = default;

		// Tries to allocate a block and returns its offset on success, or InvalidOffset on failure
		size_t allocate(size_t size);

		void release(size_t offset, size_t size);

		void clear();

		AllocationPage& operator=(const AllocationPage& other) = default;
		AllocationPage& operator=(AllocationPage&& other) noexcept = default;

	private:
		struct Range
		{
			Range();
			Range(size_t offset, size_t size);

			size_t offset;
			size_t size;
		};

		size_t m_size;
		bool m_releaseOnClear;

		size_t m_currentOffset;
		size_t m_remainingSize;

		// Key is the range last address
		std::map<size_t, Range> m_availableRanges;
	};

	template <typename AllocationType, typename PageResources>
	class AllocationPool
	{
	public:
		static_assert(std::is_base_of_v<Allocation, AllocationType>, "AllocationType must be derived from Allocation");

		AllocationPool() = delete;
		// releaseOnClear : if true, allocations will only be released when the user calls clear method
		// allocate & clear methods will be faster but release won't make the range available until the next clear
		AllocationPool(size_t pageSize, bool releaseOnClear);
		AllocationPool(const AllocationPool& other) = delete;
		AllocationPool(AllocationPool&& other) noexcept = delete;
		virtual ~AllocationPool() = default;

		bool releaseOnClear() const noexcept;

		Ptr<AllocationType> allocate(size_t size);

		// Makes the memory available again if releaseOnClear was set to false
		// If releaseOnClear was set to true, the memory will be available on next clear
		// The released memory is considered not to be used anymore
		void release(const Allocation& allocation);

		// All previously made allocations are considered to be unused and the memory is made available again
		void clear();

		AllocationPool& operator=(const AllocationPool& other) = delete;
		AllocationPool& operator=(AllocationPool&& other) noexcept = delete;

	protected:
		virtual UPtr<PageResources> createPageResources(size_t pageSize) = 0;
		virtual Ptr<AllocationType> createAllocation(PageResources& pageResources, size_t page, size_t offset, size_t size) = 0;
		virtual void releaseResources(PageResources& pageResources, size_t offset, size_t size) = 0;
		virtual void clearResources(PageResources& pageResources) = 0;

	private:
		size_t m_pageSize;
		bool m_releaseOnClear;

		std::vector<UPtr<AllocationPage>> m_pages;
		std::vector<UPtr<PageResources>> m_pageResources;
	};
}

#include <Atema/Core/AllocationPool.inl>

#endif
