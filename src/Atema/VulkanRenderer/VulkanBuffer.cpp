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

#include <vma/vk_mem_alloc.h>

using namespace at;

VulkanBuffer::VulkanBuffer(const VulkanDevice& device, const Buffer::Settings& settings) :
	Buffer(),
	m_device(device),
	m_buffer(VK_NULL_HANDLE),
	m_allocation(VK_NULL_HANDLE),
	m_usage(settings.usage),
	m_mappable(settings.mappable),
	m_mappedData(nullptr)
{
	ATEMA_ASSERT(settings.byteSize != 0, "Invalid buffer size");

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = static_cast<VkDeviceSize>(settings.byteSize);
	bufferCreateInfo.usage = Vulkan::getBufferUsages(settings.usage);
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

	VmaAllocationInfo allocInfo;

	// Persistent mapped buffer
	if (settings.mappable)
		allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

	ATEMA_VK_CHECK(vmaCreateBuffer(m_device.getVmaAllocator(), &bufferCreateInfo, &allocCreateInfo, &m_buffer, &m_allocation, &allocInfo));

	if (settings.mappable)
		m_mappedData = allocInfo.pMappedData;
}

VulkanBuffer::~VulkanBuffer()
{
	vmaDestroyBuffer(m_device.getVmaAllocator(), m_buffer, m_allocation);
}

VkBuffer VulkanBuffer::getHandle() const noexcept
{
	return m_buffer;
}

void* VulkanBuffer::map(size_t byteOffset, size_t byteSize)
{
	ATEMA_ASSERT(m_mappable, "Only mappable buffers can be mapped");
	
	return m_mappedData;
}

void VulkanBuffer::unmap()
{
}
