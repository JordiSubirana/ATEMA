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
#include <Atema/VulkanRenderer/VulkanImageView.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>

#include <vma/vk_mem_alloc.h>

using namespace at;

VulkanImage::VulkanImage(const VulkanDevice& device, const Image::Settings& settings) :
	Image(),
	m_device(device),
	m_ownsImage(true),
	m_image(VK_NULL_HANDLE),
	m_allocation(VK_NULL_HANDLE),
	m_format(settings.format),
	m_size(settings.width, settings.height),
	m_layers(settings.layers),
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
	imageCreateInfo.arrayLayers = settings.layers;
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
}

VulkanImage::VulkanImage(const VulkanDevice& device, VkImage imageHandle, const Image::Settings& settings) :
	Image(),
	m_device(device),
	m_ownsImage(false),
	m_image(imageHandle),
	m_allocation(VK_NULL_HANDLE),
	m_format(settings.format),
	m_size(settings.width, settings.height),
	m_layers(settings.layers),
	m_mipLevels(settings.mipLevels)
{
}

VulkanImage::~VulkanImage()
{
	m_views.clear();

	if (m_ownsImage)
		vmaDestroyImage(m_device.getVmaAllocator(), m_image, m_allocation);
}

VkImage VulkanImage::getHandle() const noexcept
{
	return m_image;
}

Ptr<ImageView> VulkanImage::getView(uint32_t baseLayer, uint32_t layerCount, uint32_t baseMipLevel, uint32_t mipLevelCount)
{
	Hash hash = 0;
	DefaultHasher::hashCombine(hash, baseLayer, layerCount, baseMipLevel, mipLevelCount);

	const auto it = m_views.find(hash);

	if (it != m_views.end())
		return it->second;

	auto imageView = std::make_shared<VulkanImageView>(m_device, *this, baseLayer, layerCount, baseMipLevel, mipLevelCount);

	m_views[hash] = imageView;

	return imageView;
}

ImageFormat VulkanImage::getFormat() const noexcept
{
	return m_format;
}

Vector2u VulkanImage::getSize() const noexcept
{
	return m_size;
}

uint32_t VulkanImage::getLayers() const noexcept
{
	return m_layers;
}

uint32_t VulkanImage::getMipLevels() const noexcept
{
	return m_mipLevels;
}
