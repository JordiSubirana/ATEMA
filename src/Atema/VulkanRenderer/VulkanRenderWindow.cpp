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

#include <Atema/VulkanRenderer/VulkanRenderWindow.hpp>
#include <Atema/VulkanRenderer/VulkanDevice.hpp>
#include <Atema/VulkanRenderer/VulkanFramebuffer.hpp>
#include <Atema/VulkanRenderer/VulkanImage.hpp>
#include <Atema/VulkanRenderer/VulkanInstance.hpp>
#include <Atema/VulkanRenderer/VulkanPhysicalDevice.hpp>
#include <Atema/VulkanRenderer/VulkanRenderFrame.hpp>
#include <Atema/VulkanRenderer/VulkanRenderPass.hpp>
#include <Atema/VulkanRenderer/VulkanSwapChain.hpp>

using namespace at;

VulkanRenderWindow::VulkanRenderWindow(VulkanDevice& device, const RenderWindow::Settings& settings) :
	RenderWindow(settings),
	m_device(device),
	m_colorFormat(settings.colorFormat),
	m_depthFormat(settings.depthFormat),
	m_surface(VK_NULL_HANDLE),
	m_currentSwapChainImage(0),
	m_recreateSwapChain(false),
	m_currentFrameIndex(0),
	m_frameCount(settings.maxFramesInFlight)
{
	createSurface();

	initializePresentQueue();

	createRenderPass();

	createSwapChainResources();
}

VulkanRenderWindow::~VulkanRenderWindow()
{
	destroySwapChainResources();

	destroySurface();
}

VulkanDevice& VulkanRenderWindow::getDevice() noexcept
{
	return m_device;
}

ImageFormat VulkanRenderWindow::getColorFormat() const noexcept
{
	return m_colorFormat;
}

ImageFormat VulkanRenderWindow::getDepthFormat() const noexcept
{
	return m_depthFormat;
}

size_t VulkanRenderWindow::getMaxFramesInFlight() const noexcept
{
	return m_frameCount;
}

RenderFrame& VulkanRenderWindow::acquireFrame()
{
	if (m_recreateSwapChain)
	{
		createSwapChainResources();

		m_recreateSwapChain = false;
	}

	auto& renderFrame = *m_renderFrames[m_currentFrameIndex];

	renderFrame.wait();

	const auto acquireResult = m_swapChain->acquireNextImage(m_currentSwapChainImage, renderFrame.getImageAvailableSemaphoreHandle(), VK_NULL_HANDLE);

	// Out of date : recreate swapchain right now
	if (acquireResult == SwapChainResult::OutOfDate)
	{
		m_recreateSwapChain = true;

		return acquireFrame();
	}
	// Suboptimal : recreate next frame, we can still use it
	else if (acquireResult == SwapChainResult::Suboptimal)
	{
		m_recreateSwapChain = true;
	}
	// Impossible to acquire
	else if (acquireResult != SwapChainResult::Success)
	{
		ATEMA_ERROR("Failed to acquire a valid swapchain image");
	}

	// Check if a previous frame is using this image (i.e. there is its fence to wait on)
	if (m_imageFences[m_currentSwapChainImage])
	{
		m_imageFences[m_currentSwapChainImage]->wait();
	}

	// Mark the image as now being in use by this frame
	m_imageFences[m_currentSwapChainImage] = renderFrame.getFence();

	renderFrame.setImageIndex(m_currentSwapChainImage);

	m_currentFrameIndex = (m_currentFrameIndex + 1) % m_frameCount;

	return renderFrame;
}

VkSurfaceKHR VulkanRenderWindow::getSurface() const noexcept
{
	return m_surface;
}

uint32_t VulkanRenderWindow::getPresentQueueFamilyIndex() const noexcept
{
	return m_presentQueueFamilyIndex;
}

Ptr<RenderPass> VulkanRenderWindow::getRenderPass() const noexcept
{
	return m_renderPass;
}

Ptr<Framebuffer> VulkanRenderWindow::getFramebuffer(size_t imageIndex) const noexcept
{
	return m_framebuffers[imageIndex];
}

void VulkanRenderWindow::present(const VulkanRenderFrame& renderFrame)
{
	auto vkSemaphore = std::static_pointer_cast<VulkanSemaphore>(renderFrame.getRenderFinishedSemaphore())->getHandle();

	std::vector<VkSemaphore> vkSemaphores = { vkSemaphore };

	VkPresentInfoKHR presentInfo{};

	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	// Which semaphores to wait on before presentation can happen
	presentInfo.waitSemaphoreCount = static_cast<uint32_t>(vkSemaphores.size());
	presentInfo.pWaitSemaphores = vkSemaphores.data();

	auto vkSwapChain = std::static_pointer_cast<VulkanSwapChain>(m_swapChain);

	VkSwapchainKHR swapChains[] = { vkSwapChain->getHandle() };
	auto imageIndex = renderFrame.getImageIndex();

	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	// Array of VkResult values to check for every individual swap chain if presentation was successful
	//presentInfo.pResults = nullptr; // Optional

	const auto result = m_device.vkQueuePresentKHR(m_presentQueue, &presentInfo);

	const auto swapChainResult = Vulkan::getSwapChainResult(result);

	if (swapChainResult == SwapChainResult::OutOfDate || swapChainResult == SwapChainResult::Suboptimal)
	{
		m_recreateSwapChain = true;
	}
	else if (swapChainResult != SwapChainResult::Success)
	{
		ATEMA_ERROR("SwapChain presentation failed");
	}
}

void VulkanRenderWindow::onResize(const Vector2u& newSize)
{
	m_recreateSwapChain = true;

	RenderWindow::onResize(newSize);
}

void VulkanRenderWindow::createSurface()
{
	const auto& instance = m_device.getInstance();

	// Ensure the surface is destroyed
	destroySurface();

	// Create surface
#ifdef ATEMA_SYSTEM_WINDOWS
	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = static_cast<HWND>(getHandle());
	createInfo.hinstance = GetModuleHandle(nullptr);

	ATEMA_VK_CHECK(instance.vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &m_surface));
#else
#error VulkanRenderWindow is not available on this OS
#endif
}

void VulkanRenderWindow::destroySurface()
{
	if (m_surface != VK_NULL_HANDLE)
	{
		const auto& instance = m_device.getInstance();

		instance.vkDestroySurfaceKHR(instance, m_surface, nullptr);

		m_surface = VK_NULL_HANDLE;
	}
}

void VulkanRenderWindow::initializePresentQueue()
{
	const auto& instance = m_device.getInstance();
	const auto& physicalDevice = m_device.getPhysicalDevice();

	constexpr uint32_t invalidIndex = std::numeric_limits<uint32_t>::max();

	const std::unordered_map<uint32_t, VkQueue> queues =
	{
		{ m_device.getQueueFamilyIndex(QueueType::Graphics), m_device.getQueue(QueueType::Graphics) },
		{ m_device.getQueueFamilyIndex(QueueType::Compute), m_device.getQueue(QueueType::Compute) },
		{ m_device.getQueueFamilyIndex(QueueType::Transfer), m_device.getQueue(QueueType::Transfer) }
	};

	// Present queue families
	m_presentQueueFamilyIndex = invalidIndex;

	uint32_t queueFamilyIndex = 0;
	for (auto& queueFamily : physicalDevice.getQueueFamilyProperties())
	{
		auto queueIt = queues.find(queueFamilyIndex);

		// Only check on enabled queue families
		if (queueIt == queues.end())
			continue;

		VkBool32 presentationSupported = false;
		instance.vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, m_surface, &presentationSupported);

		if (presentationSupported)
		{
			m_presentQueueFamilyIndex = queueFamilyIndex;
			m_presentQueue = queueIt->second;
			break;
		}

		queueFamilyIndex++;
	}

	if (m_presentQueueFamilyIndex == invalidIndex)
	{
		ATEMA_ERROR("Current device does not support presentation on this window");
	}
}

void VulkanRenderWindow::createRenderPass()
{
	// RenderPass
	RenderPass::Settings renderPassSettings;
	renderPassSettings.attachments.resize(2);
	renderPassSettings.attachments[0].format = m_colorFormat;
	renderPassSettings.attachments[0].finalLayout = ImageLayout::Present;
	renderPassSettings.attachments[1].format = m_depthFormat;

	renderPassSettings.subpasses.resize(1);
	renderPassSettings.subpasses[0].color = { 0 };
	renderPassSettings.subpasses[0].depthStencil = 1;

	m_renderPass = std::static_pointer_cast<RenderPass>(std::make_shared<VulkanRenderPass>(m_device, renderPassSettings));
}

void VulkanRenderWindow::createSwapChainResources()
{
	const auto windowSize = getSize();

	// Ensure the resources are deleted
	destroySwapChainResources();

	// Swapchain creation
	VulkanSwapChain::Settings swapChainSettings;
	swapChainSettings.imageFormat = m_colorFormat;

	m_swapChain = std::make_shared<VulkanSwapChain>(*this, swapChainSettings);

	// Depth image
	Image::Settings depthSettings;
	depthSettings.width = windowSize.x;
	depthSettings.height = windowSize.y;
	depthSettings.format = m_depthFormat;
	depthSettings.usages = ImageUsage::RenderTarget;

	m_depthImage = std::static_pointer_cast<Image>(std::make_shared<VulkanImage>(m_device, depthSettings));

	// Framebuffers
	Framebuffer::Settings framebufferSettings;
	framebufferSettings.renderPass = m_renderPass;
	framebufferSettings.width = windowSize.x;
	framebufferSettings.height = windowSize.y;

	const auto& swapChainImages = m_swapChain->getImages();

	m_framebuffers.reserve(swapChainImages.size());

	for (auto& swapChainImage : swapChainImages)
	{
		framebufferSettings.images =
		{
			swapChainImage,
			m_depthImage
		};

		auto framebuffer = std::make_shared<VulkanFramebuffer>(m_device, framebufferSettings);

		m_framebuffers.emplace_back(std::static_pointer_cast<Framebuffer>(framebuffer));
	}

	// RenderFrames
	m_renderFrames.resize(m_frameCount);

	size_t frameIndex = 0;
	for (auto& renderFrame : m_renderFrames)
	{
		renderFrame = std::make_shared<VulkanRenderFrame>(*this, frameIndex++);
	}

	// Image fences
	m_imageFences.resize(swapChainImages.size());

	m_currentFrameIndex = 0;
	m_currentSwapChainImage = 0;
}

void VulkanRenderWindow::destroySwapChainResources()
{
	// Ensure no resource are currently in use
	m_device.waitForIdle();

	m_imageFences.clear();

	m_renderFrames.clear();

	m_framebuffers.clear();

	m_depthImage.reset();

	m_swapChain.reset();
}
