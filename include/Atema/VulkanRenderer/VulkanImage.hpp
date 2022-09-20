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

#ifndef ATEMA_VULKANRENDERER_VULKANIMAGE_HPP
#define ATEMA_VULKANRENDERER_VULKANIMAGE_HPP

#include <Atema/VulkanRenderer/Config.hpp>
#include <Atema/Renderer/Image.hpp>
#include <Atema/VulkanRenderer/Vulkan.hpp>

#include <unordered_map>

namespace at
{
	class ATEMA_VULKANRENDERER_API VulkanImage final : public Image
	{
	public:
		VulkanImage() = delete;
		VulkanImage(const VulkanDevice& device, const Image::Settings& settings);
		VulkanImage(const VulkanDevice& device, VkImage imageHandle, const Image::Settings& settings);
		virtual ~VulkanImage();

		VkImage getHandle() const noexcept;

		Ptr<ImageView> getView(uint32_t baseLayer = 0, uint32_t layerCount = 0, uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 0) override;

		ImageFormat getFormat() const noexcept override;

		Vector2u getSize() const noexcept override;

		uint32_t getLayers() const noexcept override;

		uint32_t getMipLevels() const noexcept override;
		
	private:
		const VulkanDevice& m_device;
		bool m_ownsImage;
		VkImage m_image;
		VmaAllocation m_allocation;
		ImageFormat m_format;
		Vector2u m_size;
		uint32_t m_layers;
		uint32_t m_mipLevels;
		std::unordered_map<Hash, Ptr<ImageView>> m_views;
	};
}

#endif
