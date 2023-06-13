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

#include <Atema/Renderer/BufferPool.hpp>
#include <Atema/Core/Error.hpp>

using namespace at;

BufferPool::BufferPool(Flags<BufferUsage> usages, size_t blockSize) :
	m_usages(usages),
	m_blockSize(blockSize)
{
	ATEMA_ASSERT(usages, "Invalid buffer usages");
	ATEMA_ASSERT(blockSize > 0, "Invalid block size");
}

BufferRange BufferPool::create(size_t byteSize)
{
	for (auto& block : m_blocks)
	{
		for (auto it = block.freeRanges.begin(); it != block.freeRanges.end(); it++)
		{
			auto& freeRange = it->second;

			if (freeRange.size >= byteSize)
			{
				BufferRange bufferRange;
				bufferRange.buffer = block.buffer.get();
				bufferRange.size = byteSize;
				bufferRange.offset = freeRange.offset;
				
				if (freeRange.size == byteSize)
				{
					block.freeRanges.erase(it);
				}
				else
				{
					freeRange.size -= byteSize;
					freeRange.offset += byteSize;
				}

				return bufferRange;
			}
		}
	}

	// No block currently available : create a new one
	auto& block = m_blocks.emplace_back();

	Buffer::Settings bufferSettings;
	bufferSettings.usages = m_usages;
	bufferSettings.byteSize = std::max(m_blockSize, byteSize);

	block.buffer = Buffer::create(bufferSettings);

	if (byteSize < bufferSettings.byteSize)
	{
		auto& freeRange = block.freeRanges[bufferSettings.byteSize];
		freeRange.buffer = block.buffer.get();
		freeRange.offset = byteSize;
		freeRange.size = bufferSettings.byteSize - byteSize;
	}

	m_bufferToBlock[block.buffer.get()] = m_blocks.size() - 1;
	
	return BufferRange(*block.buffer.get(), 0, byteSize);
}

void BufferPool::release(const BufferRange& range)
{
	if (!range.buffer)
		return;

	auto it = m_bufferToBlock.find(range.buffer);

	if (it == m_bufferToBlock.end())
		return;

	auto& block = m_blocks[it->second];

	BufferRange newRange(range);
	// Merge with previous range if it exists
	auto prevIt = block.freeRanges.find(range.offset);
	if (prevIt != block.freeRanges.end())
	{
		newRange.offset = prevIt->second.offset;
		newRange.size += prevIt->second.size;
		block.freeRanges.erase(prevIt);
	}

	const auto lastAddress = newRange.offset + newRange.size;
	
	// Merge with next range if it exists
	auto nextIt = block.freeRanges.lower_bound(lastAddress);
	if (nextIt != block.freeRanges.end() && nextIt->second.offset == lastAddress)
	{
		auto& nextRange = nextIt->second;
		nextRange.offset = newRange.offset;
		nextRange.size += newRange.size;
	}
	// If not, add a new one
	else
	{
		block.freeRanges.emplace(lastAddress, newRange);
	}
}

void BufferPool::clear()
{
	for (auto& block : m_blocks)
	{
		BufferRange range;
		range.buffer = block.buffer.get();
		range.offset = 0;
		range.size = range.buffer->getByteSize();

		block.freeRanges.clear();
		block.freeRanges[range.size] = std::move(range);
	}
}
