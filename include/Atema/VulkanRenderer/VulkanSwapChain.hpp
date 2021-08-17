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

#ifndef ATEMA_VULKANRENDERER_VULKANSWAPCHAIN_HPP
#define ATEMA_VULKANRENDERER_VULKANSWAPCHAIN_HPP

#include <Atema/VulkanRenderer/Config.hpp>
#include <Atema/Renderer/SwapChain.hpp>
#include <Atema/VulkanRenderer/Vulkan.hpp>

#include <vector>

namespace at
{
	class ATEMA_VULKANRENDERER_API VulkanSwapChain final : public SwapChain
	{
	public:
		struct SupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};
		
		VulkanSwapChain() = delete;
		VulkanSwapChain(const SwapChain::Settings& settings);
		virtual ~VulkanSwapChain();

		static SupportDetails getSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface);

		VkSwapchainKHR getHandle() const noexcept;
		
		std::vector<Ptr<Image>>& getImages() noexcept override;
		const std::vector<Ptr<Image>>& getImages() const noexcept override;

		SwapChainResult acquireNextImage(uint32_t& imageIndex, const Ptr<Fence>& fence) override;
		SwapChainResult acquireNextImage(uint32_t& imageIndex, const Ptr<Semaphore>& semaphore) override;
		
	private:
		SwapChainResult acquireNextImage(uint32_t& imageIndex, VkSemaphore semaphore, VkFence fence);
		
		VkDevice m_device;
		VkSurfaceKHR m_surface;
		VkSwapchainKHR m_swapChain;
		std::vector<Ptr<Image>> m_images;
		VkExtent2D m_extent;
		VkFormat m_format;
	};
}

#endif
