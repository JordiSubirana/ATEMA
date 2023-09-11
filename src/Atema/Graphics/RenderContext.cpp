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

#include <Atema/Graphics/RenderContext.hpp>
#include <Atema/Core/TaskManager.hpp>
#include <Atema/Renderer/CommandPool.hpp>
#include <Atema/Renderer/Enums.hpp>

using namespace at;

namespace
{
	// 1MB by default
	constexpr size_t StagingBufferBlockSize = 1048576;
}

RenderContext::RenderContext() :
	m_stagingBufferPool(BufferUsage::Map | BufferUsage::TransferSrc, StagingBufferBlockSize, true)
{
	const auto threadCount = TaskManager::instance().getSize();

	const std::array<QueueType, 3> queueTypes =
	{
		QueueType::Graphics,
		QueueType::Compute,
		QueueType::Transfer
	};

	m_commandPools.resize(queueTypes.size());

	for (size_t i = 0; i < queueTypes.size(); i++)
	{
		auto& queueType = queueTypes[i];
		auto& commandPools = m_commandPools[i];

		commandPools.resize(threadCount + 1);

		CommandPool::Settings commandPoolSettings;
		commandPoolSettings.queueType = queueType;

		for (auto& commandPool : commandPools)
			commandPool = CommandPool::create(commandPoolSettings);
	}
}

Ptr<CommandBuffer> RenderContext::createCommandBuffer(const CommandBuffer::Settings& settings, QueueType queueType)
{
	const auto& commandPools = getCommandPools(queueType);

	return commandPools.back()->createBuffer(settings);
}

Ptr<CommandBuffer> RenderContext::createCommandBuffer(const CommandBuffer::Settings& settings, QueueType queueType, size_t threadIndex)
{
	const auto& commandPools = getCommandPools(queueType);

	ATEMA_ASSERT(threadIndex < commandPools.size() - 1, "Invalid thread index");

	return commandPools[threadIndex]->createBuffer(settings);
}

Ptr<BufferAllocation> RenderContext::createStagingBuffer(size_t byteSize)
{
	return m_stagingBufferPool.allocate(byteSize);
}

void RenderContext::destroyPendingResources()
{
	std::lock_guard lockGuard(m_resourceMutex);

	m_resources.clear();

	m_stagingBufferPool.clear();
}

std::vector<Ptr<CommandPool>>& RenderContext::getCommandPools(QueueType queueType)
{
	return m_commandPools[static_cast<size_t>(queueType)];
}
