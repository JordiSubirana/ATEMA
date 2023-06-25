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

#ifndef ATEMA_RENDERER_BUFFERPOOL_HPP
#define ATEMA_RENDERER_BUFFERPOOL_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Renderer/Buffer.hpp>
#include <Atema/Core/AllocationPool.hpp>

namespace at
{
	class ATEMA_RENDERER_API BufferAllocation : public Allocation
	{
	public:
		BufferAllocation() = delete;
		BufferAllocation(Buffer& buffer, size_t page, size_t offset, size_t size);
		BufferAllocation(const BufferAllocation& other) = delete;
		BufferAllocation(BufferAllocation&& other) noexcept = delete;
		~BufferAllocation() = default;

		Buffer& getBuffer() noexcept;
		const Buffer& getBuffer() const noexcept;

		// Only valid for buffer created with BufferUsage::Map
		void* map();

		BufferAllocation& operator=(const BufferAllocation& other) = delete;
		BufferAllocation& operator=(BufferAllocation&& other) noexcept = delete;

	private:
		Buffer* m_buffer;
	};

	class ATEMA_RENDERER_API BufferPageResources
	{
	public:
		BufferPageResources() = delete;
		BufferPageResources(Flags<BufferUsage> usages, size_t size);
		BufferPageResources(const BufferPageResources& other) = delete;
		BufferPageResources(BufferPageResources&& other) noexcept = delete;
		~BufferPageResources() = default;

		Buffer& getBuffer() noexcept;
		const Buffer& getBuffer() const noexcept;

		BufferPageResources& operator=(const BufferPageResources& other) = delete;
		BufferPageResources& operator=(BufferPageResources&& other) noexcept = delete;

	private:
		Ptr<Buffer> m_buffer;
	};

	class ATEMA_RENDERER_API BufferPool : public AllocationPool<BufferAllocation, BufferPageResources>
	{
	public:
		BufferPool() = delete;
		BufferPool(Flags<BufferUsage> usages, size_t pageSize, bool releaseOnClear);
		BufferPool(const BufferPool& other) = delete;
		BufferPool(BufferPool&& other) noexcept = delete;
		~BufferPool() = default;

		BufferPool& operator=(const BufferPool& other) = delete;
		BufferPool& operator=(BufferPool&& other) noexcept = delete;

	protected:
		UPtr<BufferPageResources> createPageResources(size_t pageSize) override;
		Ptr<BufferAllocation> createAllocation(BufferPageResources& pageResources, size_t page, size_t offset, size_t size) override;
		void releaseResources(BufferPageResources& pageResources, size_t offset, size_t size) override;
		void clearResources(BufferPageResources& pageResources) override;

	private:
		Flags<BufferUsage> m_usages;
	};
}

#endif