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

#include <Atema/VulkanRenderer/VulkanBuffer.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>

using namespace at;

VulkanBuffer::VulkanBuffer(const VulkanDevice& device, const Buffer::Settings& settings) :
	Buffer(),
	m_device(device),
	m_buffer(VK_NULL_HANDLE),
	m_memory(VK_NULL_HANDLE),
	m_usage(settings.usage),
	m_mappable(settings.mappable)
{
	ATEMA_ASSERT(settings.byteSize != 0, "Invalid buffer size");

	// Buffer creation
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = static_cast<VkDeviceSize>(settings.byteSize);
	bufferInfo.usage = Vulkan::getBufferUsages(settings.usage);
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	ATEMA_VK_CHECK(m_device.vkCreateBuffer(m_device, &bufferInfo, nullptr, &m_buffer));

	// Memory allcoation
	VkMemoryRequirements memRequirements;
	m_device.vkGetBufferMemoryRequirements(m_device, m_buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : Ensure we can map the memory into CPU visible memory
	// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : Ensure the memory can be written at the time we specify it
	allocInfo.memoryTypeIndex = m_device.getPhysicalDevice().findMemoryType(memRequirements.memoryTypeBits, Vulkan::getMemoryProperties(settings.mappable));

	//TODO: Should not be called for individual buffers
	// Check physical device's maxMemoryAllocationCount
	// Use an allocator with offset parameters
	ATEMA_VK_CHECK(m_device.vkAllocateMemory(m_device, &allocInfo, nullptr, &m_memory));

	// Set the memory to the buffer
	ATEMA_VK_CHECK(m_device.vkBindBufferMemory(m_device, m_buffer, m_memory, 0));
}

VulkanBuffer::~VulkanBuffer()
{
	ATEMA_VK_DESTROY(m_device, vkDestroyBuffer, m_buffer);
	ATEMA_VK_DESTROY(m_device, vkFreeMemory, m_memory);
}

VkBuffer VulkanBuffer::getHandle() const noexcept
{
	return m_buffer;
}

void* VulkanBuffer::map(size_t byteOffset, size_t byteSize)
{
	ATEMA_ASSERT(m_mappable, "Only mappable buffers can be mapped");
	
	if (m_mappable)
	{
		void* data = nullptr;

		const VkDeviceSize offset = static_cast<VkDeviceSize>(byteOffset);
		const VkDeviceSize size = byteSize ? static_cast<VkDeviceSize>(byteSize) : VK_WHOLE_SIZE;

		ATEMA_VK_CHECK(m_device.vkMapMemory(m_device, m_memory, offset, size, 0, &data));

		return data;
	}

	return nullptr;
}

void VulkanBuffer::unmap()
{
	if (m_mappable)
	{
		m_device.vkUnmapMemory(m_device, m_memory);
	}
}
