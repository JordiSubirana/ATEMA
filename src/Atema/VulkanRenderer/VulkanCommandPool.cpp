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

#include <Atema/VulkanRenderer/VulkanCommandBuffer.hpp>
#include <Atema/VulkanRenderer/VulkanCommandPool.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>

using namespace at;

VulkanCommandPool::VulkanCommandPool(const VulkanDevice& device, const CommandPool::Settings& settings) :
	VulkanCommandPool(device, device.getQueueFamilyIndex(settings.queueType), settings)
{
	
}

VulkanCommandPool::VulkanCommandPool(const VulkanDevice& device, uint32_t queueFamilyIndex, const CommandPool::Settings& settings) :
	CommandPool(),
	m_device(device),
	m_queueFamilyIndex(queueFamilyIndex),
	m_queueType(settings.queueType),
	m_commandPool(VK_NULL_HANDLE)
{
	//TODO: Make this custom
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = m_queueFamilyIndex;
	// VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : Command buffers may be rerecorded with new commands very often (can optimize memory allocations)
	// VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
	poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

	ATEMA_VK_CHECK(m_device.vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool));
}

VulkanCommandPool::~VulkanCommandPool()
{
	ATEMA_VK_DESTROY(m_device, vkDestroyCommandPool, m_commandPool);
}

VkCommandPool VulkanCommandPool::getHandle() const noexcept
{
	return m_commandPool;
}

Ptr<CommandBuffer> VulkanCommandPool::createBuffer(const CommandBuffer::Settings& settings)
{
	auto commandBuffer = std::make_shared<VulkanCommandBuffer>(m_commandPool, m_queueType, m_queueFamilyIndex, settings);

	return std::static_pointer_cast<CommandBuffer>(commandBuffer);
}

uint32_t VulkanCommandPool::getQueueFamilyIndex() const noexcept
{
	return m_queueFamilyIndex;
}
