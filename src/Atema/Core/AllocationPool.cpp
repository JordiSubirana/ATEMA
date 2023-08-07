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

#include <Atema/Core/AllocationPool.hpp>
#include <Atema/Core/Utils.hpp>

using namespace at;

// Allocation
Allocation::Allocation(size_t page, size_t offset, size_t size) :
	m_page(page),
	m_offset(offset),
	m_size(size)
{
}

Allocation::~Allocation()
{
	onDestroy();
}

size_t Allocation::getPage() const noexcept
{
	return m_page;
}

size_t Allocation::getOffset() const noexcept
{
	return m_offset;
}

size_t Allocation::getSize() const noexcept
{
	return m_size;
}

// AllocationPage::Range
AllocationPage::Range::Range() :
	Range(0, 0)
{
}

AllocationPage::Range::Range(size_t offset, size_t size) :
	offset(offset),
	size(size)
{
}

// AllocationPage
AllocationPage::AllocationPage(size_t size, size_t alignment, bool releaseOnClear) :
	m_size(size),
	m_alignment(alignment),
	m_releaseOnClear(releaseOnClear),
	m_currentOffset(0),
	m_remainingSize(size),
	m_allocationCount(0)
{
	Range range(m_currentOffset, m_remainingSize);
	m_availableRanges[range.size] = std::move(range);
}

size_t AllocationPage::allocate(size_t size)
{
	// Align the size
	size = nextMultiplePowerOfTwo(size, m_alignment);

	if (m_releaseOnClear)
	{
		if (m_remainingSize >= size)
		{
			const size_t offset = m_currentOffset;

			m_currentOffset += size;
			m_remainingSize -= size;

			m_allocationCount++;

			return offset;
		}
	}
	else
	{
		for (auto it = m_availableRanges.begin(); it != m_availableRanges.end(); it++)
		{
			auto& availableRange = it->second;

			if (availableRange.size >= size)
			{
				const auto offset = availableRange.offset;

				if (availableRange.size == size)
				{
					m_availableRanges.erase(it);
				}
				else
				{
					availableRange.size -= size;
					availableRange.offset += size;
				}

				m_allocationCount++;

				return offset;
			}
		}
	}

	return InvalidOffset;
}

void AllocationPage::release(size_t offset, size_t size)
{
	m_allocationCount--;

	if (!m_releaseOnClear)
		return;

	// Align the size
	size = nextMultiplePowerOfTwo(size, m_alignment);

	Range newRange(offset, size);

	// Merge with previous range if it exists
	auto prevIt = m_availableRanges.find(offset);
	if (prevIt != m_availableRanges.end())
	{
		newRange.offset = prevIt->second.offset;
		newRange.size += prevIt->second.size;
		m_availableRanges.erase(prevIt);
	}

	const auto lastAddress = newRange.offset + newRange.size;

	// Merge with next range if it exists
	auto nextIt = m_availableRanges.lower_bound(lastAddress);
	if (nextIt != m_availableRanges.end() && nextIt->second.offset == lastAddress)
	{
		auto& nextRange = nextIt->second;
		nextRange.offset = newRange.offset;
		nextRange.size += newRange.size;
	}
	// If not, add a new one
	else
	{
		m_availableRanges.emplace(lastAddress, newRange);
	}
}

void AllocationPage::clear()
{
	m_currentOffset = 0;
	m_remainingSize = m_size;

	m_allocationCount = 0;

	m_availableRanges.clear();

	Range range(m_currentOffset, m_remainingSize);
	m_availableRanges[range.size] = std::move(range);
}

size_t AllocationPage::getAllocationCount() const noexcept
{
	return m_allocationCount;
}
