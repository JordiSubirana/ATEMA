/*
	Copyright 2022 Jordi SUBIRANA

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

#include <Atema/VulkanRenderer/VulkanFence.hpp>
#include <Atema/VulkanRenderer/VulkanRenderFrame.hpp>
#include <Atema/VulkanRenderer/VulkanRenderWindow.hpp>
#include <Atema/Core/TaskManager.hpp>
#include <Atema/VulkanRenderer/VulkanCommandBuffer.hpp>
#include <Atema/VulkanRenderer/VulkanCommandPool.hpp>
#include <Atema/VulkanRenderer/VulkanDevice.hpp>

using namespace at;

VulkanRenderFrame::VulkanRenderFrame(VulkanRenderWindow& renderWindow, size_t frameIndex) :
	m_device(renderWindow.getDevice()),
	m_renderWindow(renderWindow),
	m_imageIndex(std::numeric_limits<uint32_t>::max()),
	m_frameIndex(frameIndex)
{
	m_fence = std::make_unique<VulkanFence>(m_device, VulkanFence::Settings({ true }));

	m_imageAvailableSemaphore = std::make_unique<VulkanSemaphore>(m_device);
	m_renderFinishedSemaphore = std::make_unique<VulkanSemaphore>(m_device);

	// Initialize command pools
	const auto threadCount = TaskManager::instance().getSize();
	constexpr uint32_t invalidIndex = std::numeric_limits<uint32_t>::max();

	const std::vector<uint32_t> queueFamilyIndices =
	{
		m_device.getQueueFamilyIndex(QueueType::Graphics),
		m_device.getQueueFamilyIndex(QueueType::Compute),
		m_device.getQueueFamilyIndex(QueueType::Transfer)
	};

	// Initialize other queues (try to find a unique queue family index for each requirement)
	const std::vector<QueueType> queueTypes =
	{
		QueueType::Graphics,
		QueueType::Compute,
		QueueType::Transfer
	};

	m_commandPools.resize(queueFamilyIndices.size());

	for (size_t i = 0; i < queueFamilyIndices.size(); i++)
	{
		auto& queueFamilyIndex = queueFamilyIndices[i];
		auto& queueType = queueTypes[i];
		auto& commandPools = m_commandPools[i];

		// Create unique queue family, queue and command pools
		if (i == 0 || queueFamilyIndex != queueFamilyIndices[0])
		{
			commandPools.resize(threadCount + 1);

			CommandPool::Settings commandPoolSettings;
			commandPoolSettings.queueType = queueType;

			for (auto& commandPool : commandPools)
				commandPool = std::make_shared<VulkanCommandPool>(m_device, queueFamilyIndex, commandPoolSettings);
		}
		// Fall back on graphics family data
		else
		{
			commandPools = m_commandPools[0];
		}
	}
}

VulkanRenderFrame::~VulkanRenderFrame()
{
}

void VulkanRenderFrame::wait()
{
	m_fence->wait();
}

void VulkanRenderFrame::setImageIndex(uint32_t imageIndex)
{
	m_imageIndex = imageIndex;
}

uint32_t VulkanRenderFrame::getImageIndex() const noexcept
{
	return m_imageIndex;
}

size_t VulkanRenderFrame::getFrameIndex() const noexcept
{
	return m_frameIndex;
}

Ptr<CommandBuffer> VulkanRenderFrame::createCommandBuffer(const CommandBuffer::Settings& settings, QueueType queueType)
{
	return getCommandPools(queueType).back()->createBuffer(settings);
}

Ptr<CommandBuffer> VulkanRenderFrame::createCommandBuffer(const CommandBuffer::Settings& settings, QueueType queueType, size_t threadIndex)
{
	const auto& commandPool = getCommandPools(queueType);

	ATEMA_ASSERT(threadIndex < commandPool.size() - 1, "Invalid thread index");

	return commandPool[threadIndex]->createBuffer(settings);
}

Ptr<RenderPass> VulkanRenderFrame::getRenderPass() const noexcept
{
	return m_renderWindow.getRenderPass();
}

Ptr<Framebuffer> VulkanRenderFrame::getFramebuffer() const noexcept
{
	return m_renderWindow.getFramebuffer(m_imageIndex);
}

Ptr<Semaphore> VulkanRenderFrame::getImageAvailableSemaphore() const noexcept
{
	return std::static_pointer_cast<Semaphore>(m_imageAvailableSemaphore);
}

Ptr<Semaphore> VulkanRenderFrame::getRenderFinishedSemaphore() const noexcept
{
	return std::static_pointer_cast<Semaphore>(m_renderFinishedSemaphore);
}

Ptr<Fence> VulkanRenderFrame::getFence() const noexcept
{
	return m_fence;
}

VkSemaphore VulkanRenderFrame::getImageAvailableSemaphoreHandle() const
{
	return m_imageAvailableSemaphore->getHandle();
}

VkSemaphore VulkanRenderFrame::getRenderFinishedSemaphoreHandle() const
{
	return m_renderFinishedSemaphore->getHandle();
}

void VulkanRenderFrame::submit(
	const std::vector<Ptr<CommandBuffer>>& commandBuffers,
	const std::vector<WaitCondition>& waitConditions,
	const std::vector<Ptr<Semaphore>>& signalSemaphores,
	Ptr<Fence> fence)
{
	m_device.submit(commandBuffers, waitConditions, signalSemaphores, fence);
}

void VulkanRenderFrame::present()
{
	m_renderWindow.present(*this);
}

std::vector<Ptr<CommandPool>>& VulkanRenderFrame::getCommandPools(QueueType queueType)
{
	return m_commandPools[static_cast<size_t>(queueType)];
}
