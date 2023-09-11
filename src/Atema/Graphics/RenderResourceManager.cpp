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

#include <Atema/Core/Benchmark.hpp>
#include <Atema/Graphics/RenderResourceManager.hpp>
#include <Atema/Core/Error.hpp>
#include <Atema/Renderer/Buffer.hpp>
#include <Atema/Renderer/RenderFrame.hpp>

using namespace at;

namespace
{
	// Minimum : 256B
	constexpr size_t MinimumBufferPageSize = 256;
	// Default : 1MB
	constexpr size_t DefaultBufferPageSize = 1048576;
}

// Range
RenderResourceManager::Range::Range() :
	Range(0, 0)
{
}

RenderResourceManager::Range::Range(size_t offset, size_t size) :
	offset(offset),
	size(size)
{
}

// RenderResourceManager
RenderResourceManager::RenderResourceManager() :
	RenderResourceManager(DefaultBufferPageSize)
{
}

RenderResourceManager::RenderResourceManager(size_t bufferPoolPageSize) :
	m_commandBuffer(nullptr),
	m_renderContext(nullptr),
	m_bufferPageSize(std::max(bufferPoolPageSize, MinimumBufferPageSize))
{
}

Ptr<BufferAllocation> RenderResourceManager::createBuffer(const Buffer::Settings& settings)
{
	return getBufferPool(settings.usages).allocate(settings.byteSize);
}

void RenderResourceManager::beginTransfer(CommandBuffer& commandBuffer, RenderContext& renderContext)
{
	m_commandBuffer = &commandBuffer;
	m_renderContext = &renderContext;
}

void RenderResourceManager::endTransfer()
{
	updateResources();

	destroyPendingResources();

	m_commandBuffer = nullptr;
	m_renderContext = nullptr;
}

CommandBuffer& RenderResourceManager::getCommandBuffer() const
{
	ATEMA_ASSERT(m_commandBuffer, "RenderResourceManager::getCommandBuffer must only be called between beginFrame & endFrame");

	return *m_commandBuffer;
}

RenderContext& RenderResourceManager::getRenderContext() const
{
	ATEMA_ASSERT(m_renderContext, "RenderResourceManager::getRenderContext must only be called between beginFrame & endFrame");

	return *m_renderContext;
}

void* RenderResourceManager::mapBuffer(BufferAllocation& bufferAllocation)
{
	Buffer& buffer = bufferAllocation.getBuffer();
	const size_t offset = bufferAllocation.getOffset();
	const size_t size = bufferAllocation.getSize();

	if (buffer.getUsages() & BufferUsage::Map)
		return buffer.map(offset, size);

	return prepareTransfer(getBufferStagingData(bufferAllocation), offset, size);
}

void* RenderResourceManager::mapBuffer(Buffer& buffer, size_t offset, size_t size)
{
	if (buffer.getUsages() & BufferUsage::Map)
		return buffer.map(offset, size);

	return prepareTransfer(getBufferStagingData(buffer), offset, size);
}

BufferPool& RenderResourceManager::getBufferPool(const Flags<BufferUsage>& usages)
{
	const auto it = m_bufferPools.find(usages);

	if (it != m_bufferPools.end())
		return *it->second;

	UPtr<BufferPool> bufferPool = std::make_unique<BufferPool>(usages, m_bufferPageSize, false);

	BufferPool* bufferPoolPtr = bufferPool.get();

	m_bufferPools.emplace(usages, std::move(bufferPool));

	return *bufferPoolPtr;
}

RenderResourceManager::StagingData& RenderResourceManager::createBufferStagingData(BufferAllocation& bufferAllocation)
{
	Buffer& buffer = bufferAllocation.getBuffer();
	const size_t allocationCount = getBufferPool(buffer.getUsages()).getAllocationCount(bufferAllocation.getPage());

	StagingData& stagingData = createBufferStagingData(buffer);
	stagingData.ranges.reserve(allocationCount);

	return stagingData;
}

RenderResourceManager::StagingData& RenderResourceManager::createBufferStagingData(Buffer& buffer)
{
	UPtr<StagingData> stagingData = std::make_unique<StagingData>();
	stagingData->stagingBuffer = m_renderContext->createStagingBuffer(buffer.getByteSize());

	StagingData* stagingDataPtr = stagingData.get();

	m_bufferStagingData.emplace(&buffer, std::move(stagingData));

	return *stagingDataPtr;
}

RenderResourceManager::StagingData& RenderResourceManager::getBufferStagingData(BufferAllocation& bufferAllocation)
{
	Buffer& buffer = bufferAllocation.getBuffer();

	const auto it = m_bufferStagingData.find(&buffer);

	if (it != m_bufferStagingData.end())
		return *it->second;

	return createBufferStagingData(bufferAllocation);
}

RenderResourceManager::StagingData& RenderResourceManager::getBufferStagingData(Buffer& buffer)
{
	const auto it = m_bufferStagingData.find(&buffer);

	if (it != m_bufferStagingData.end())
		return *it->second;

	return createBufferStagingData(buffer);
}

void* RenderResourceManager::prepareTransfer(StagingData& stagingData, size_t offset, size_t size)
{
	stagingData.ranges.emplace_back(offset, size);

	void* data = stagingData.stagingBuffer->map();

	return static_cast<void*>(static_cast<uint8_t*>(data) + offset);
}

void RenderResourceManager::updateResources()
{
	auto& commandBuffer = getCommandBuffer();
	
	for (auto& [dstBuffer, stagingData] : m_bufferStagingData)
	{
		BufferAllocation& stagingBuffer = *stagingData->stagingBuffer;

		auto& ranges = stagingData->ranges;
		std::sort(ranges.begin(), ranges.end(), [](const Range& r1, const Range& r2)
			{
				return r1.offset < r2.offset;
			});

		// Copy all ranges from staging buffer to dstBuffer
		for (auto it = ranges.begin(); it != ranges.end(); it++)
		{
			size_t offset = it->offset;
			size_t lastAddress = offset + it->size;

			// Group multiple copy ranges if possible
			for (auto it2 = std::next(it); it2 != ranges.end(); it2++)
			{
				const size_t nextOffset = it2->offset;
				const size_t nextSize = it2->size;

				if (nextOffset <= lastAddress)
				{
					lastAddress = nextOffset + nextSize;

					it = it2;
				}
				else
				{
					break;
				}
			}
			const size_t size = lastAddress - offset;

			commandBuffer.copyBuffer(stagingBuffer.getBuffer(), *dstBuffer, size, offset, offset);
		}
	}
}

void RenderResourceManager::destroyPendingResources()
{
	m_bufferStagingData.clear();
}
