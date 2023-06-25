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

using namespace at;

// BufferAllocation
BufferAllocation::BufferAllocation(Buffer& buffer, size_t page, size_t offset, size_t size) :
	Allocation(page, offset, size),
	m_buffer(&buffer)
{
}

Buffer& BufferAllocation::getBuffer() noexcept
{
	return *m_buffer;
}

const Buffer& BufferAllocation::getBuffer() const noexcept
{
	return *m_buffer;
}

void* BufferAllocation::map()
{
	return m_buffer->map(getOffset(), getSize());
}

// BufferPageResources
BufferPageResources::BufferPageResources(Flags<BufferUsage> usages, size_t size)
{
	m_buffer = Buffer::create({ usages, size });
}

Buffer& BufferPageResources::getBuffer() noexcept
{
	return *m_buffer;
}

const Buffer& BufferPageResources::getBuffer() const noexcept
{
	return *m_buffer;
}

// BufferPool
BufferPool::BufferPool(Flags<BufferUsage> usages, size_t pageSize, bool releaseOnClear) :
	AllocationPool(pageSize, releaseOnClear),
	m_usages(usages)
{
}

UPtr<BufferPageResources> BufferPool::createPageResources(size_t pageSize)
{
	return std::make_unique<BufferPageResources>(m_usages, pageSize);
}

Ptr<BufferAllocation> BufferPool::createAllocation(BufferPageResources& pageResources, size_t page, size_t offset, size_t size)
{
	return std::make_shared<BufferAllocation>(pageResources.getBuffer(), page, offset, size);
}

void BufferPool::releaseResources(BufferPageResources& pageResources, size_t offset, size_t size)
{
}

void BufferPool::clearResources(BufferPageResources& pageResources)
{
}
