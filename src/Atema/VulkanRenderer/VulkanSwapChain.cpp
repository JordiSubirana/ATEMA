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

#include <Atema/VulkanRenderer/VulkanSwapChain.hpp>
#include <Atema/Core/Window.hpp>
#include <Atema/VulkanRenderer/VulkanFence.hpp>
#include <Atema/VulkanRenderer/VulkanImage.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>
#include <Atema/VulkanRenderer/VulkanSemaphore.hpp>

using namespace at;

namespace
{
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkFormat format, const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		//TODO: Make this custom

		// Use SRGB if it is available, because it results in more accurate perceived colors
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == format &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		// Use first format by default
		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		//TODO: Make this custom

		// VK_PRESENT_MODE_IMMEDIATE_KHR : images transmitted right away => tearing possible
		// VK_PRESENT_MODE_FIFO_KHR : fifo, if the queue is full, the program waits => VSync
		// VK_PRESENT_MODE_FIFO_RELAXED_KHR : same as fifo, but if the image comes late it's transferred right away => tearing possible
		// VK_PRESENT_MODE_MAILBOX_KHR : same as fifo but images are replaced if the queue is full => triple buffering
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		// This is the only one guaranteed to be available
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D chooseSwapExtent(VkExtent2D extent, const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
			return capabilities.currentExtent;

		// Clamp extent between min & max extent values supported
		extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, extent.width));
		extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, extent.height));

		return extent;
	}
}

VulkanSwapChain::VulkanSwapChain(const SwapChain::Settings& settings) :
	SwapChain(),
	m_device(VK_NULL_HANDLE),
	m_surface(VK_NULL_HANDLE),
	m_swapChain(VK_NULL_HANDLE),
	m_extent({0, 0}),
	m_format(VK_FORMAT_UNDEFINED)
{
	//TODO: Make this custom

	if (!settings.window)
	{
		ATEMA_ERROR("Invalid window");
	}
	
	auto& renderer = VulkanRenderer::getInstance();
	auto instance = renderer.getInstanceHandle();
	auto physicalDevice = renderer.getPhysicalDeviceHandle();
	m_device = renderer.getLogicalDeviceHandle();

	m_surface = renderer.getWindowSurface(settings.window);

	m_extent =
	{
		static_cast<uint32_t>(settings.window->getSize().x),
		static_cast<uint32_t>(settings.window->getSize().y)
	};

	SupportDetails swapChainSupport = getSupportDetails(physicalDevice, m_surface);

	auto format = Vulkan::getFormat(settings.format);
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(format, swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	m_extent = chooseSwapExtent(m_extent, swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount;
	if (swapChainSupport.capabilities.maxImageCount > 0 &&
		imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	m_format = surfaceFormat.format;

	// Create swap chain
	{
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = m_extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // To render directly to it
		//createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT; // To copy from some image (for post-processing)

		auto graphicsIndex = renderer.getGraphicsQueueIndex();
		auto presentIndex = renderer.getPresentQueueIndex();
		uint32_t queueFamilyIndices[] = { graphicsIndex, presentIndex };

		if (graphicsIndex != presentIndex)
		{
			// Lower performance but simpler to use if the queues are different
			// Images can be used across multiple queue families without explicit ownership transfers
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			// Best performance
			// An image is owned by one queue family at a time and ownership must be explicitly transferred before using it in another queue family
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform; // Can be used for rotations or flips
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Can be used to blend with other windows. Here we'll ignore it
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE; // Best performance but we won't be able to get pixels if another window is in front of this one
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		ATEMA_VK_CHECK(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain));
	}

	// Create images
	{
		// Get images created within the swapchain (will be cleaned up at swapchain's destruction)
		vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);

		std::vector<VkImage> swapChainImages(imageCount, VK_NULL_HANDLE);

		vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, swapChainImages.data());

		for (auto& swapChainImage : swapChainImages)
		{
			auto image = std::make_shared<VulkanImage>(swapChainImage, m_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);

			m_images.push_back(std::static_pointer_cast<Image>(image));
		}
	}
}

VulkanSwapChain::~VulkanSwapChain()
{
	// Ensure the images are deleted
	m_images.clear();
	
	ATEMA_VK_DESTROY(m_device, vkDestroySwapchainKHR, m_swapChain);
}

VulkanSwapChain::SupportDetails VulkanSwapChain::getSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	SupportDetails swapChainSupportDetails;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapChainSupportDetails.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		swapChainSupportDetails.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, swapChainSupportDetails.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		swapChainSupportDetails.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, swapChainSupportDetails.presentModes.data());
	}

	return swapChainSupportDetails;
}

VkSwapchainKHR VulkanSwapChain::getHandle() const noexcept
{
	return m_swapChain;
}

std::vector<Ptr<Image>>& VulkanSwapChain::getImages() noexcept
{
	return m_images;
}

const std::vector<Ptr<Image>>& VulkanSwapChain::getImages() const noexcept
{
	return m_images;
}

SwapChain::AcquireResult VulkanSwapChain::acquireNextImage(uint32_t& imageIndex, const Ptr<Fence>& fence)
{
	ATEMA_ASSERT(fence, "Invalid Fence");

	auto vkFence = std::static_pointer_cast<VulkanFence>(fence);
	
	return acquireNextImage(imageIndex, VK_NULL_HANDLE, vkFence->getHandle());
}

SwapChain::AcquireResult VulkanSwapChain::acquireNextImage(uint32_t& imageIndex, const Ptr<Semaphore>& semaphore)
{
	ATEMA_ASSERT(semaphore, "Invalid Semaphore");

	auto vkSemaphore = std::static_pointer_cast<VulkanSemaphore>(semaphore);

	return acquireNextImage(imageIndex, vkSemaphore->getHandle(), VK_NULL_HANDLE);
}

SwapChain::AcquireResult VulkanSwapChain::acquireNextImage(uint32_t& imageIndex, VkSemaphore semaphore, VkFence fence)
{
	auto result = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, semaphore, fence, &imageIndex);

	switch (result)
	{
		case VK_SUCCESS: return AcquireResult::Success;
		case VK_NOT_READY: return AcquireResult::NotReady;
		case VK_SUBOPTIMAL_KHR: return AcquireResult::Suboptimal;
		case VK_ERROR_OUT_OF_DATE_KHR:
		case VK_ERROR_SURFACE_LOST_KHR:
		{
			return AcquireResult::OutOfDate;
		}
		case VK_ERROR_OUT_OF_HOST_MEMORY:
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		case VK_ERROR_DEVICE_LOST:
		case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
		case VK_TIMEOUT:
		default:
		{
			break;
		}
	}

	return AcquireResult::Error;
}
