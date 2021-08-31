/*
	Copyright 2021 Jordi SUBIRANA

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

using namespace at;

VulkanImage::VulkanImage(const Image::Settings& settings) :
	Image(),
	m_ownsImage(true),
	m_image(VK_NULL_HANDLE),
	m_view(VK_NULL_HANDLE),
	m_memory(VK_NULL_HANDLE),
	m_format(settings.format),
	m_layouts(settings.mipLevels, VK_IMAGE_LAYOUT_UNDEFINED),
	m_size(settings.width, settings.height),
	m_mipLevels(settings.mipLevels)
{
	auto& renderer = VulkanRenderer::instance();
	m_device = renderer.getLogicalDeviceHandle();

	auto format = Vulkan::getFormat(settings.format);

	// Create image
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = settings.width;
		imageInfo.extent.height = settings.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = settings.mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format; // Use the same format than the buffer
		imageInfo.tiling = Vulkan::getTiling(settings.tiling); // Optimal or linear if we want to change pixels client side
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = Vulkan::getUsages(settings.usages, hasDepth(settings.format));
		//TODO: Make this custom
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Here used by only one queue
		imageInfo.samples = Vulkan::getSamples(settings.samples);
		imageInfo.flags = 0; // Optional

		ATEMA_VK_CHECK(vkCreateImage(m_device, &imageInfo, nullptr, &m_image));
	}

	// Allocate image memory
	{
		//TODO: Make this custom
		VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(m_device, m_image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = renderer.findMemoryType(memRequirements.memoryTypeBits, memoryProperties);

		ATEMA_VK_CHECK(vkAllocateMemory(m_device, &allocInfo, nullptr, &m_memory));

		vkBindImageMemory(m_device, m_image, m_memory, 0);
	}

	// Create view
	createView(format, Vulkan::getAspect(settings.format), settings.mipLevels);
}

VulkanImage::VulkanImage(VkImage imageHandle, VkFormat format, VkImageAspectFlags aspect, uint32_t mipLevels) :
	Image(),
	m_ownsImage(false),
	m_image(imageHandle),
	m_view(VK_NULL_HANDLE),
	m_memory(VK_NULL_HANDLE),
	m_format(Vulkan::getFormat(format)),
	m_layouts(1, VK_IMAGE_LAYOUT_UNDEFINED),
	m_size(0, 0),
	m_mipLevels(1)
{
	auto& renderer = VulkanRenderer::instance();
	m_device = renderer.getLogicalDeviceHandle();
	
	createView(format, aspect, mipLevels);
}

VulkanImage::~VulkanImage()
{
	ATEMA_VK_DESTROY(m_device, vkDestroyImageView, m_view);
	if (m_ownsImage)
	{
		ATEMA_VK_DESTROY(m_device, vkDestroyImage, m_image);
		ATEMA_VK_DESTROY(m_device, vkFreeMemory, m_memory);
	}
}

VkImage VulkanImage::getImageHandle() const noexcept
{
	return m_image;
}

VkImageView VulkanImage::getViewHandle() const noexcept
{
	return m_view;
}

std::vector<VkImageLayout>& VulkanImage::getLayouts() noexcept
{
	return m_layouts;
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

		ATEMA_VK_CHECK(vkCreateImageView(m_device, &viewInfo, nullptr, &m_view));
	}
}
