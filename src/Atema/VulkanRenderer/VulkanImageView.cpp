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

#include <Atema/VulkanRenderer/VulkanImageView.hpp>
#include <Atema/VulkanRenderer/VulkanDevice.hpp>
#include <Atema/VulkanRenderer/VulkanImage.hpp>

using namespace at;

VulkanImageView::VulkanImageView(const VulkanDevice& device, const VulkanImage& image, uint32_t baseLayer, uint32_t layerCount, uint32_t baseMipLevel, uint32_t mipLevelCount) :
	ImageView(image.getFormat(), baseLayer, layerCount, baseMipLevel, mipLevelCount),
	m_device(device),
	m_imageView(VK_NULL_HANDLE)
{
	const auto format = image.getFormat();

	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image.getHandle();
	viewInfo.format = Vulkan::getFormat(format);

	if (layerCount == 1)
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	else if (image.getType() == ImageType::CubeMap)
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	else
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;

	// Describe what the image's purpose is and which part of the image should be accessed
	viewInfo.subresourceRange.aspectMask = Vulkan::getAspect(format);
	viewInfo.subresourceRange.baseMipLevel = baseMipLevel;
	viewInfo.subresourceRange.levelCount = mipLevelCount;
	viewInfo.subresourceRange.baseArrayLayer = baseLayer;
	viewInfo.subresourceRange.layerCount = layerCount;

	ATEMA_VK_CHECK(m_device.vkCreateImageView(m_device, &viewInfo, nullptr, &m_imageView));
}

VulkanImageView::~VulkanImageView()
{
	ATEMA_VK_DESTROY(m_device, vkDestroyImageView, m_imageView);
}

VkImageView VulkanImageView::getHandle() const noexcept
{
	return m_imageView;
}
