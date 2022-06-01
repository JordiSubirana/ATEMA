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

#include <Atema/VulkanRenderer/VulkanImage.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>

#include <vma/vk_mem_alloc.h>

using namespace at;

VulkanImage::VulkanImage(const VulkanDevice& device, const Image::Settings& settings) :
	Image(),
	m_device(device),
	m_ownsImage(true),
	m_image(VK_NULL_HANDLE),
	m_view(VK_NULL_HANDLE),
	m_allocation(VK_NULL_HANDLE),
	m_format(settings.format),
	m_size(settings.width, settings.height),
	m_mipLevels(settings.mipLevels)
{
	const auto format = Vulkan::getFormat(settings.format);

	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = settings.width;
	imageCreateInfo.extent.height = settings.height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = settings.mipLevels;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = format; // Use the same format than the buffer
	imageCreateInfo.tiling = Vulkan::getTiling(settings.tiling); // Optimal or linear if we want to change pixels client side
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = Vulkan::getUsages(settings.usages, Renderer::isDepthImageFormat(settings.format));
	//TODO: Make this custom
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Here used by only one queue
	imageCreateInfo.samples = Vulkan::getSamples(settings.samples);
	imageCreateInfo.flags = 0; // Optional

	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	allocCreateInfo.priority = 1.0f;

	ATEMA_VK_CHECK(vmaCreateImage(m_device.getVmaAllocator(), &imageCreateInfo, &allocCreateInfo, &m_image, &m_allocation, nullptr));

	// Create view
	createView(format, Vulkan::getAspect(settings.format), settings.mipLevels);
}

VulkanImage::VulkanImage(const VulkanDevice& device, VkImage imageHandle, VkFormat format, VkImageAspectFlags aspect, uint32_t mipLevels) :
	Image(),
	m_device(device),
	m_ownsImage(false),
	m_image(imageHandle),
	m_view(VK_NULL_HANDLE),
	m_allocation(VK_NULL_HANDLE),
	m_format(Vulkan::getFormat(format)),
	m_size(0, 0),
	m_mipLevels(1)
{
	createView(format, aspect, mipLevels);
}

VulkanImage::~VulkanImage()
{
	ATEMA_VK_DESTROY(m_device, vkDestroyImageView, m_view);

	if (m_ownsImage)
		vmaDestroyImage(m_device.getVmaAllocator(), m_image, m_allocation);
}

VkImage VulkanImage::getImageHandle() const noexcept
{
	return m_image;
}

VkImageView VulkanImage::getViewHandle() const noexcept
{
	return m_view;
}

ImageFormat VulkanImage::getFormat() const noexcept
{
	return m_format;
}

Vector2u VulkanImage::getSize() const noexcept
{
	return m_size;
}

uint32_t VulkanImage::getMipLevels() const noexcept
{
	return m_mipLevels;
}

void VulkanImage::createView(VkFormat format, VkImageAspectFlags aspect, uint32_t mipLevels)
{
	// Create view
	//TODO: Check if this is the right place
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;

		// Describe what the image's purpose is and which part of the image should be accessed
		// Here, no mipmaps, no multiple layers (for stereographic 3D with R/L eyes for example)
		viewInfo.subresourceRange.aspectMask = aspect;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		ATEMA_VK_CHECK(m_device.vkCreateImageView(m_device, &viewInfo, nullptr, &m_view));
	}
}
