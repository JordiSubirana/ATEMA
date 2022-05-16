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

#include <Atema/VulkanRenderer/VulkanRenderer.hpp>
#include <Atema/Window/Window.hpp>
#include <Atema/VulkanRenderer/VulkanImage.hpp>
#include <Atema/VulkanRenderer/VulkanSampler.hpp>
#include <Atema/VulkanRenderer/VulkanSwapChain.hpp>
#include <Atema/VulkanRenderer/VulkanRenderPass.hpp>
#include <Atema/VulkanRenderer/VulkanFramebuffer.hpp>
#include <Atema/VulkanRenderer/VulkanShader.hpp>
#include <Atema/VulkanRenderer/VulkanDescriptorPool.hpp>
#include <Atema/VulkanRenderer/VulkanDescriptorSet.hpp>
#include <Atema/VulkanRenderer/VulkanGraphicsPipeline.hpp>
#include <Atema/VulkanRenderer/VulkanCommandPool.hpp>
#include <Atema/VulkanRenderer/VulkanCommandBuffer.hpp>
#include <Atema/VulkanRenderer/VulkanFence.hpp>
#include <Atema/VulkanRenderer/VulkanSemaphore.hpp>
#include <Atema/VulkanRenderer/VulkanBuffer.hpp>
#include <Atema/Core/TaskManager.hpp>

#include <set>
#include <thread>

using namespace at;

namespace
{
	const std::vector<const char*> validationLayers =
	{
#ifdef ATEMA_DEBUG
		"VK_LAYER_KHRONOS_validation"
#endif
	};

	const std::vector<const char*> deviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
}

VulkanRenderer::VulkanRenderer(const Renderer::Settings& settings) :
	Renderer(settings),
	m_instance(VK_NULL_HANDLE),
	m_surface(VK_NULL_HANDLE),
	m_physicalDevice(VK_NULL_HANDLE),
	m_device(VK_NULL_HANDLE),
	m_graphicsQueue(VK_NULL_HANDLE),
	m_presentQueue(VK_NULL_HANDLE),
	m_maxSamples(ImageSamples::S1)
{
}

VulkanRenderer::~VulkanRenderer()
{
	waitForIdle();

	unregisterWindows();

	// Wait for asynchronous stuff to be done
	m_device->waitForIdle();

	m_threadCommandPools.clear();

	m_device.reset();

	m_instance.reset();
}

VulkanRenderer& VulkanRenderer::instance()
{
	return static_cast<VulkanRenderer&>(Renderer::instance());
}

const Renderer::Limits& VulkanRenderer::getLimits() const noexcept
{
	return m_limits;
}

void VulkanRenderer::initialize()
{
	createInstance();

	registerWindow(getMainWindow());
	createSurface();

	pickPhysicalDevice();

	createDevice();

	createThreadCommandPools();
}

void VulkanRenderer::waitForIdle()
{
	m_device->waitForIdle();
}

Ptr<CommandPool> VulkanRenderer::getDefaultCommandPool()
{
	return m_threadCommandPools.back();
}

Ptr<CommandPool> VulkanRenderer::getCommandPool(size_t threadIndex)
{
	ATEMA_ASSERT(threadIndex < m_threadCommandPools.size() - 1, "Thread index out of range");
	
	return m_threadCommandPools[threadIndex];
}

void VulkanRenderer::registerWindow(Ptr<Window> window)
{
	if (!window)
		return;

	// Create surface
	VkSurfaceKHR surface = VK_NULL_HANDLE;

#ifdef ATEMA_SYSTEM_WINDOWS
	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = static_cast<HWND>(window->getHandle());
	createInfo.hinstance = GetModuleHandle(nullptr);

	ATEMA_VK_CHECK(m_instance->vkCreateWin32SurfaceKHR(m_instance->getHandle(), &createInfo, nullptr, &surface));
#else
#error VulkanRenderer is not available on this OS
#endif

	m_windowSurfaces[window.get()] = surface;
}

void VulkanRenderer::unregisterWindow(Ptr<Window> window)
{
	auto it = m_windowSurfaces.find(window.get());

	if (it != m_windowSurfaces.end())
	{
		m_instance->vkDestroySurfaceKHR(m_instance->getHandle(), it->second, nullptr);

		m_windowSurfaces.erase(it);
	}
}

Ptr<Image> VulkanRenderer::createImage(const Image::Settings& settings)
{
	auto object = std::make_shared<VulkanImage>(*m_device, settings);

	return std::static_pointer_cast<Image>(object);
}

Ptr<Sampler> VulkanRenderer::createSampler(const Sampler::Settings& settings)
{
	auto object = std::make_shared<VulkanSampler>(*m_device, settings);

	return std::static_pointer_cast<Sampler>(object);
}

Ptr<SwapChain> VulkanRenderer::createSwapChain(const SwapChain::Settings& settings)
{
	auto object = std::make_shared<VulkanSwapChain>(*m_device, settings);

	return std::static_pointer_cast<SwapChain>(object);
}

Ptr<RenderPass> VulkanRenderer::createRenderPass(const RenderPass::Settings& settings)
{
	auto object = std::make_shared<VulkanRenderPass>(*m_device, settings);

	return std::static_pointer_cast<RenderPass>(object);
}

Ptr<Framebuffer> VulkanRenderer::createFramebuffer(const Framebuffer::Settings& settings)
{
	auto object = std::make_shared<VulkanFramebuffer>(*m_device, settings);

	return std::static_pointer_cast<Framebuffer>(object);
}

Ptr<Shader> VulkanRenderer::createShader(const Shader::Settings& settings)
{
	auto object = std::make_shared<VulkanShader>(*m_device, settings);

	return std::static_pointer_cast<Shader>(object);
}

Ptr<DescriptorSetLayout> VulkanRenderer::createDescriptorSetLayout(const DescriptorSetLayout::Settings& settings)
{
	auto object = std::make_shared<VulkanDescriptorSetLayout>(*m_device, settings);

	return std::static_pointer_cast<DescriptorSetLayout>(object);
}

Ptr<DescriptorPool> VulkanRenderer::createDescriptorPool(const DescriptorPool::Settings& settings)
{
	auto object = std::make_shared<VulkanDescriptorPool>(*m_device, settings);

	return std::static_pointer_cast<DescriptorPool>(object);
}

Ptr<GraphicsPipeline> VulkanRenderer::createGraphicsPipeline(const GraphicsPipeline::Settings& settings)
{
	auto object = std::make_shared<VulkanGraphicsPipeline>(*m_device, settings);

	return std::static_pointer_cast<GraphicsPipeline>(object);
}

Ptr<CommandPool> VulkanRenderer::createCommandPool(const CommandPool::Settings& settings)
{
	auto object = std::make_shared<VulkanCommandPool>(*m_device, settings);

	return std::static_pointer_cast<CommandPool>(object);
}

Ptr<Fence> VulkanRenderer::createFence(const Fence::Settings& settings)
{
	auto object = std::make_shared<VulkanFence>(*m_device, settings);

	return std::static_pointer_cast<Fence>(object);
}

Ptr<Semaphore> VulkanRenderer::createSemaphore()
{
	auto object = std::make_shared<VulkanSemaphore>(*m_device);

	return std::static_pointer_cast<Semaphore>(object);
}

Ptr<Buffer> VulkanRenderer::createBuffer(const Buffer::Settings& settings)
{
	auto object = std::make_shared<VulkanBuffer>(*m_device, settings);

	return std::static_pointer_cast<Buffer>(object);
}

void VulkanRenderer::submit(
	const std::vector<Ptr<CommandBuffer>>& commandBuffers,
	const std::vector<Ptr<Semaphore>>& waitSemaphores,
	const std::vector<Flags<PipelineStage>>& waitStages,
	const std::vector<Ptr<Semaphore>>& signalSemaphores,
	Ptr<Fence> fence)
{
	ATEMA_ASSERT(!commandBuffers.empty(), "At least one command buffer must be submitted");
	ATEMA_ASSERT(waitSemaphores.size() == waitStages.size(), "Each wait semaphore must have a corresponding wait stage");

	std::vector<VkCommandBuffer> vkCommandBuffers(commandBuffers.size());
	std::vector<VkSemaphore> vkWaitSemaphores(waitSemaphores.size());
	std::vector<VkPipelineStageFlags> vkWaitStages(waitStages.size());
	std::vector<VkSemaphore> vkSignalSemaphores(signalSemaphores.size());

	for (size_t i = 0; i < vkCommandBuffers.size(); i++)
	{
		auto vkCommandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(commandBuffers[i]);

		vkCommandBuffers[i] = vkCommandBuffer->getHandle();
	}

	for (size_t i = 0; i < vkWaitSemaphores.size(); i++)
	{
		auto vkSemaphore = std::static_pointer_cast<VulkanSemaphore>(waitSemaphores[i]);

		vkWaitSemaphores[i] = vkSemaphore->getHandle();
		vkWaitStages[i] = Vulkan::getPipelineStages(waitStages[i]);
	}

	for (size_t i = 0; i < vkSignalSemaphores.size(); i++)
	{
		auto vkSemaphore = std::static_pointer_cast<VulkanSemaphore>(signalSemaphores[i]);

		vkSignalSemaphores[i] = vkSemaphore->getHandle();
	}

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	// Which semaphores to wait, and on which pipeline stages
	submitInfo.waitSemaphoreCount = static_cast<uint32_t>(vkWaitSemaphores.size());
	submitInfo.pWaitSemaphores = vkWaitSemaphores.data();
	submitInfo.pWaitDstStageMask = vkWaitStages.data();

	// Command buffers to be submitted
	submitInfo.commandBufferCount = static_cast<uint32_t>(vkCommandBuffers.size());
	submitInfo.pCommandBuffers = vkCommandBuffers.data();

	// Specify what semaphore will be used as the signal that command buffers were executed
	submitInfo.signalSemaphoreCount = static_cast<uint32_t>(vkSignalSemaphores.size());
	submitInfo.pSignalSemaphores = vkSignalSemaphores.data();

	VkFence vkFence = VK_NULL_HANDLE;

	if (fence)
		vkFence = std::static_pointer_cast<VulkanFence>(fence)->getHandle();

	ATEMA_VK_CHECK(m_device->vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, vkFence));
}

SwapChainResult VulkanRenderer::present(
	const Ptr<SwapChain>& swapChain,
	uint32_t imageIndex,
	const std::vector<Ptr<Semaphore>>& waitSemaphores)
{
	ATEMA_ASSERT(swapChain, "Invalid SwapChain");

	std::vector<VkSemaphore> vkSemaphores(waitSemaphores.size());

	for (size_t i = 0; i < vkSemaphores.size(); i++)
	{
		auto vkSemaphore = std::static_pointer_cast<VulkanSemaphore>(waitSemaphores[i]);

		vkSemaphores[i] = vkSemaphore->getHandle();
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	// Which semaphores to wait on before presentation can happen
	presentInfo.waitSemaphoreCount = static_cast<uint32_t>(vkSemaphores.size());
	presentInfo.pWaitSemaphores = vkSemaphores.data();

	auto vkSwapChain = std::static_pointer_cast<VulkanSwapChain>(swapChain);

	VkSwapchainKHR swapChains[] = { vkSwapChain->getHandle() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	// Array of VkResult values to check for every individual swap chain if presentation was successful
	//presentInfo.pResults = nullptr; // Optional

	const auto result = m_device->vkQueuePresentKHR(m_presentQueue, &presentInfo);

	return Vulkan::getSwapChainResult(result);
}

VkSurfaceKHR VulkanRenderer::getWindowSurface(Ptr<Window> window) const
{
	auto it = m_windowSurfaces.find(window.get());

	if (it == m_windowSurfaces.end())
	{
		ATEMA_ERROR("Windows need to be registered in the Renderer in order to access their surface");

		return VK_NULL_HANDLE;
	}

	return it->second;
}

const VulkanInstance& VulkanRenderer::getInstance() const noexcept
{
	return *m_instance;
}

const VulkanPhysicalDevice& VulkanRenderer::getPhysicalDevice() const noexcept
{
	return *m_physicalDevice;
}

const VulkanDevice& VulkanRenderer::getDevice() const noexcept
{
	return *m_device;
}

uint32_t VulkanRenderer::getGraphicsQueueIndex() const noexcept
{
	return m_queueFamilyData.graphicsIndex;
}

uint32_t VulkanRenderer::getPresentQueueIndex() const noexcept
{
	return m_queueFamilyData.presentIndex;
}

bool VulkanRenderer::checkValidationLayerSupport()
{
	// Used for debugging purpose
	if (!validationLayers.empty())
	{
		uint32_t layerCount;
		Vulkan::instance().vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		Vulkan::instance().vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (auto& layerName : validationLayers)
		{
			bool layerFound = false;

			for (auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}
	}

	return true;
}

void VulkanRenderer::createInstance()
{
	// Check for requested validation layers support
	if (!checkValidationLayerSupport())
	{
		ATEMA_ERROR("Validation layers requested, but not available");
	}

	// Application info (optionnal)
	// Can give some freedom to the driver to optimize the app
	// TODO: Make this custom
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Application";
	appInfo.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
	appInfo.pEngineName = "Atema";
	appInfo.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;

	// Check for available extensions
	uint32_t availableExtensionCount = 0;
	Vulkan::instance().vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
	Vulkan::instance().vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

	// GLFW required extensions
	auto& glfwRequiredExtensionNames = Window::getVulkanExtensions();
	const auto glfwRequiredExtensionCount = glfwRequiredExtensionNames.size();

	//TODO: User required extensions
	std::vector<std::string> userRequiredExtensionNames;

	// Required extensions (glfw required + user required)
	std::vector<const char*> requiredExtensionNames;
	requiredExtensionNames.reserve(glfwRequiredExtensionCount + userRequiredExtensionNames.size());

	for (uint32_t i = 0; i < glfwRequiredExtensionCount; i++)
		requiredExtensionNames.push_back(glfwRequiredExtensionNames[i]);

	for (auto& extensionName : userRequiredExtensionNames)
		requiredExtensionNames.push_back(extensionName.c_str());

	// Check if the required extensions are available
	for (auto& requiredExtensionName : requiredExtensionNames)
	{
		bool found = false;

		for (auto& availableExtension : availableExtensions)
		{
			std::string availableExtensionName(availableExtension.extensionName);

			if (availableExtensionName == requiredExtensionName)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			ATEMA_ERROR("Required extension \"" + std::string(requiredExtensionName) + "\" not found");
		}
	}

	// Instance info
	// Tells the driver which global extensions and validation layers we want to use
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensionNames.size());
	createInfo.ppEnabledExtensionNames = requiredExtensionNames.data();
	createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	createInfo.ppEnabledLayerNames = validationLayers.data();

	m_instance = std::make_unique<VulkanInstance>(createInfo);
}

void VulkanRenderer::createSurface()
{
	m_surface = getWindowSurface(getMainWindow());
}

VulkanRenderer::QueueFamilyData VulkanRenderer::getQueueFamilyData(const VulkanPhysicalDevice& device) const
{
	QueueFamilyData queueFamilyData;

	int index = 0;

	for (auto& queueFamily : device.getQueueFamilyProperties())
	{
		// Graphics features
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				queueFamilyData.hasGraphics = true;
				queueFamilyData.graphicsIndex = index;
			}
		}

		// Presentation features
		{
			VkBool32 presentSupport = false;
			m_instance->vkGetPhysicalDeviceSurfaceSupportKHR(device.getHandle(), index, m_surface, &presentSupport);

			if (presentSupport)
			{
				queueFamilyData.hasPresent = true;
				queueFamilyData.presentIndex = index;
			}
		}

		index++;
	}

	return queueFamilyData;
}

VulkanRenderer::SwapChainSupportDetails VulkanRenderer::getSwapChainSupport(const VulkanPhysicalDevice& device) const
{
	SwapChainSupportDetails swapChainSupportDetails;

	m_instance->vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &swapChainSupportDetails.capabilities);

	uint32_t formatCount;
	m_instance->vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		swapChainSupportDetails.formats.resize(formatCount);
		m_instance->vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, swapChainSupportDetails.formats.data());
	}

	uint32_t presentModeCount;
	m_instance->vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		swapChainSupportDetails.presentModes.resize(presentModeCount);
		m_instance->vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, swapChainSupportDetails.presentModes.data());
	}

	return swapChainSupportDetails;
}

bool VulkanRenderer::checkPhysicalDeviceExtensionSupport(const VulkanPhysicalDevice& device)
{
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : device.getExtensions())
	{
		requiredExtensions.erase(extension);
	}

	return requiredExtensions.empty();
}

ImageSamples VulkanRenderer::getMaxUsableSampleCount(const VulkanPhysicalDevice& device)
{
	// Check for max sample count for both color and depth
	auto& properties = device.getProperties();

	const VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;

	if (counts & VK_SAMPLE_COUNT_64_BIT)
		return ImageSamples::S64;

	if (counts & VK_SAMPLE_COUNT_32_BIT)
		return ImageSamples::S32;

	if (counts & VK_SAMPLE_COUNT_16_BIT)
		return ImageSamples::S16;

	if (counts & VK_SAMPLE_COUNT_8_BIT)
		return ImageSamples::S8;

	if (counts & VK_SAMPLE_COUNT_4_BIT)
		return ImageSamples::S4;

	if (counts & VK_SAMPLE_COUNT_2_BIT)
		return ImageSamples::S2;

	return ImageSamples::S1;
}

int VulkanRenderer::getPhysicalDeviceScore(const VulkanPhysicalDevice& device, const QueueFamilyData& queueFamilyData)
{
	//TODO: Make this function complete and custom

	int score = -1;

	if (!queueFamilyData.isComplete())
		return score;

	bool swapChainAdequate = false;
	if (checkPhysicalDeviceExtensionSupport(device))
	{
		SwapChainSupportDetails swapChainSupport = getSwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	if (!swapChainAdequate)
		return score;

	//TODO: Make this an option (see createDevice())
	if (!device.getFeatures().samplerAnisotropy)
		return score;

	switch (device.getProperties().deviceType)
	{
		// CPU
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
		{
			score = 1;
			break;
		}
		// Virtual
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
		{
			score = 10;
			break;
		}
		// Integrated
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		{
			score = 100;
			break;
		}
		// Discrete
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		{
			score = 1000;
			break;
		}
		default:
		{
			break;
		}
	}

	return score;
}

void VulkanRenderer::pickPhysicalDevice()
{
	int currentScore = -1;

	for (auto& physicalDevice : m_instance->getPhysicalDevices())
	{
		auto queueFamilyData = getQueueFamilyData(physicalDevice);

		auto score = getPhysicalDeviceScore(physicalDevice, queueFamilyData);

		if (score > 0 && score > currentScore)
		{
			currentScore = score;
			m_physicalDevice = &physicalDevice;
			m_queueFamilyData = queueFamilyData;
			m_maxSamples = getMaxUsableSampleCount(physicalDevice);
		}
	}

	if (m_physicalDevice == VK_NULL_HANDLE)
		ATEMA_ERROR("Failed to find a valid physical device");

	// Save device limits
	auto& limits = m_physicalDevice->getProperties().limits;

	m_limits.maxImageDimension1D = limits.maxImageDimension1D;
	m_limits.maxImageDimension2D = limits.maxImageDimension2D;
	m_limits.maxImageDimension3D = limits.maxImageDimension3D;
	m_limits.maxImageDimensionCube = limits.maxImageDimensionCube;
	m_limits.maxImageArrayLayers = limits.maxImageArrayLayers;
	m_limits.maxTexelBufferElements = limits.maxTexelBufferElements;
	m_limits.maxUniformBufferRange = limits.maxUniformBufferRange;
	m_limits.maxStorageBufferRange = limits.maxStorageBufferRange;
	m_limits.maxPushConstantsSize = limits.maxPushConstantsSize;
	m_limits.maxMemoryAllocationCount = limits.maxMemoryAllocationCount;
	m_limits.maxSamplerAllocationCount = limits.maxSamplerAllocationCount;
	m_limits.bufferImageGranularity = limits.bufferImageGranularity;
	m_limits.sparseAddressSpaceSize = limits.sparseAddressSpaceSize;
	m_limits.maxBoundDescriptorSets = limits.maxBoundDescriptorSets;
	m_limits.maxPerStageDescriptorSamplers = limits.maxPerStageDescriptorSamplers;
	m_limits.maxPerStageDescriptorUniformBuffers = limits.maxPerStageDescriptorUniformBuffers;
	m_limits.maxPerStageDescriptorStorageBuffers = limits.maxPerStageDescriptorStorageBuffers;
	m_limits.maxPerStageDescriptorSampledImages = limits.maxPerStageDescriptorSampledImages;
	m_limits.maxPerStageDescriptorStorageImages = limits.maxPerStageDescriptorStorageImages;
	m_limits.maxPerStageDescriptorInputAttachments = limits.maxPerStageDescriptorInputAttachments;
	m_limits.maxPerStageResources = limits.maxPerStageResources;
	m_limits.maxDescriptorSetSamplers = limits.maxDescriptorSetSamplers;
	m_limits.maxDescriptorSetUniformBuffers = limits.maxDescriptorSetUniformBuffers;
	m_limits.maxDescriptorSetUniformBuffersDynamic = limits.maxDescriptorSetUniformBuffersDynamic;
	m_limits.maxDescriptorSetStorageBuffers = limits.maxDescriptorSetStorageBuffers;
	m_limits.maxDescriptorSetStorageBuffersDynamic = limits.maxDescriptorSetStorageBuffersDynamic;
	m_limits.maxDescriptorSetSampledImages = limits.maxDescriptorSetSampledImages;
	m_limits.maxDescriptorSetStorageImages = limits.maxDescriptorSetStorageImages;
	m_limits.maxDescriptorSetInputAttachments = limits.maxDescriptorSetInputAttachments;
	m_limits.maxVertexInputAttributes = limits.maxVertexInputAttributes;
	m_limits.maxVertexInputBindings = limits.maxVertexInputBindings;
	m_limits.maxVertexInputAttributeOffset = limits.maxVertexInputAttributeOffset;
	m_limits.maxVertexInputBindingStride = limits.maxVertexInputBindingStride;
	m_limits.maxVertexOutputComponents = limits.maxVertexOutputComponents;
	m_limits.maxTessellationGenerationLevel = limits.maxTessellationGenerationLevel;
	m_limits.maxTessellationPatchSize = limits.maxTessellationPatchSize;
	m_limits.maxTessellationControlPerVertexInputComponents = limits.maxTessellationControlPerVertexInputComponents;
	m_limits.maxTessellationControlPerVertexOutputComponents = limits.maxTessellationControlPerVertexOutputComponents;
	m_limits.maxTessellationControlPerPatchOutputComponents = limits.maxTessellationControlPerPatchOutputComponents;
	m_limits.maxTessellationControlTotalOutputComponents = limits.maxTessellationControlTotalOutputComponents;
	m_limits.maxTessellationEvaluationInputComponents = limits.maxTessellationEvaluationInputComponents;
	m_limits.maxTessellationEvaluationOutputComponents = limits.maxTessellationEvaluationOutputComponents;
	m_limits.maxGeometryShaderInvocations = limits.maxGeometryShaderInvocations;
	m_limits.maxGeometryInputComponents = limits.maxGeometryInputComponents;
	m_limits.maxGeometryOutputComponents = limits.maxGeometryOutputComponents;
	m_limits.maxGeometryOutputVertices = limits.maxGeometryOutputVertices;
	m_limits.maxGeometryTotalOutputComponents = limits.maxGeometryTotalOutputComponents;
	m_limits.maxFragmentInputComponents = limits.maxFragmentInputComponents;
	m_limits.maxFragmentOutputAttachments = limits.maxFragmentOutputAttachments;
	m_limits.maxFragmentDualSrcAttachments = limits.maxFragmentDualSrcAttachments;
	m_limits.maxFragmentCombinedOutputResources = limits.maxFragmentCombinedOutputResources;
	m_limits.maxComputeSharedMemorySize = limits.maxComputeSharedMemorySize;
	m_limits.maxComputeWorkGroupCount[0] = limits.maxComputeWorkGroupCount[0];
	m_limits.maxComputeWorkGroupCount[1] = limits.maxComputeWorkGroupCount[1];
	m_limits.maxComputeWorkGroupCount[2] = limits.maxComputeWorkGroupCount[2];
	m_limits.maxComputeWorkGroupInvocations = limits.maxComputeWorkGroupInvocations;
	m_limits.maxComputeWorkGroupSize[0] = limits.maxComputeWorkGroupSize[0];
	m_limits.maxComputeWorkGroupSize[1] = limits.maxComputeWorkGroupSize[1];
	m_limits.maxComputeWorkGroupSize[2] = limits.maxComputeWorkGroupSize[2];
	m_limits.subPixelPrecisionBits = limits.subPixelPrecisionBits;
	m_limits.subTexelPrecisionBits = limits.subTexelPrecisionBits;
	m_limits.mipmapPrecisionBits = limits.mipmapPrecisionBits;
	m_limits.maxDrawIndexedIndexValue = limits.maxDrawIndexedIndexValue;
	m_limits.maxDrawIndirectCount = limits.maxDrawIndirectCount;
	m_limits.maxSamplerLodBias = limits.maxSamplerLodBias;
	m_limits.maxSamplerAnisotropy = limits.maxSamplerAnisotropy;
	m_limits.maxViewports = limits.maxViewports;
	m_limits.maxViewportDimensions[0] = limits.maxViewportDimensions[0];
	m_limits.maxViewportDimensions[1] = limits.maxViewportDimensions[1];
	m_limits.viewportBoundsRange[0] = limits.viewportBoundsRange[0];
	m_limits.viewportBoundsRange[1] = limits.viewportBoundsRange[1];
	m_limits.viewportSubPixelBits = limits.viewportSubPixelBits;
	m_limits.minMemoryMapAlignment = limits.minMemoryMapAlignment;
	m_limits.minTexelBufferOffsetAlignment = limits.minTexelBufferOffsetAlignment;
	m_limits.minUniformBufferOffsetAlignment = limits.minUniformBufferOffsetAlignment;
	m_limits.minStorageBufferOffsetAlignment = limits.minStorageBufferOffsetAlignment;
	m_limits.minTexelOffset = limits.minTexelOffset;
	m_limits.maxTexelOffset = limits.maxTexelOffset;
	m_limits.minTexelGatherOffset = limits.minTexelGatherOffset;
	m_limits.maxTexelGatherOffset = limits.maxTexelGatherOffset;
	m_limits.minInterpolationOffset = limits.minInterpolationOffset;
	m_limits.maxInterpolationOffset = limits.maxInterpolationOffset;
	m_limits.subPixelInterpolationOffsetBits = limits.subPixelInterpolationOffsetBits;
	m_limits.maxFramebufferWidth = limits.maxFramebufferWidth;
	m_limits.maxFramebufferHeight = limits.maxFramebufferHeight;
	m_limits.maxFramebufferLayers = limits.maxFramebufferLayers;
	m_limits.framebufferColorSampleCounts = Vulkan::getSamples(limits.framebufferColorSampleCounts);
	m_limits.framebufferDepthSampleCounts = Vulkan::getSamples(limits.framebufferDepthSampleCounts);
	m_limits.framebufferStencilSampleCounts = Vulkan::getSamples(limits.framebufferStencilSampleCounts);
	m_limits.framebufferNoAttachmentsSampleCounts = Vulkan::getSamples(limits.framebufferNoAttachmentsSampleCounts);
	m_limits.maxColorAttachments = limits.maxColorAttachments;
	m_limits.sampledImageColorSampleCounts = Vulkan::getSamples(limits.sampledImageColorSampleCounts);
	m_limits.sampledImageIntegerSampleCounts = Vulkan::getSamples(limits.sampledImageIntegerSampleCounts);
	m_limits.sampledImageDepthSampleCounts = Vulkan::getSamples(limits.sampledImageDepthSampleCounts);
	m_limits.sampledImageStencilSampleCounts = Vulkan::getSamples(limits.sampledImageStencilSampleCounts);
	m_limits.storageImageSampleCounts = Vulkan::getSamples(limits.storageImageSampleCounts);
	m_limits.maxSampleMaskWords = limits.maxSampleMaskWords;
	m_limits.timestampComputeAndGraphics = limits.timestampComputeAndGraphics;
	m_limits.timestampPeriod = limits.timestampPeriod;
	m_limits.maxClipDistances = limits.maxClipDistances;
	m_limits.maxCullDistances = limits.maxCullDistances;
	m_limits.maxCombinedClipAndCullDistances = limits.maxCombinedClipAndCullDistances;
	m_limits.discreteQueuePriorities = limits.discreteQueuePriorities;
	m_limits.pointSizeRange[0] = limits.pointSizeRange[0];
	m_limits.pointSizeRange[1] = limits.pointSizeRange[1];
	m_limits.lineWidthRange[0] = limits.lineWidthRange[0];
	m_limits.lineWidthRange[1] = limits.lineWidthRange[1];
	m_limits.pointSizeGranularity = limits.pointSizeGranularity;
	m_limits.lineWidthGranularity = limits.lineWidthGranularity;
	m_limits.strictLines = limits.strictLines;
	m_limits.standardSampleLocations = limits.standardSampleLocations;
	m_limits.optimalBufferCopyOffsetAlignment = limits.optimalBufferCopyOffsetAlignment;
	m_limits.optimalBufferCopyRowPitchAlignment = limits.optimalBufferCopyRowPitchAlignment;
	m_limits.nonCoherentAtomSize = limits.nonCoherentAtomSize;
}

void VulkanRenderer::createDevice()
{
	//TODO: Create more queues, from more types, and add some features if needed

	// Graphics queue
	constexpr float queuePriority = 1.0f; // Between 0.0 & 1.0

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies =
	{
		m_queueFamilyData.graphicsIndex,
		m_queueFamilyData.presentIndex
	};

	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Require some features
	//TODO: Make options
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	// Sample shading : set a number of samples for fragment shading
	deviceFeatures.sampleRateShading = getSettings().sampleShading ? VK_TRUE : VK_FALSE;

	// Create the device
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	createInfo.ppEnabledLayerNames = validationLayers.data();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	m_device = std::make_unique<VulkanDevice>(*m_instance, *m_physicalDevice, createInfo);

	// Get required queues
	m_device->vkGetDeviceQueue(m_device->getHandle(), m_queueFamilyData.graphicsIndex, 0, &m_graphicsQueue);
	m_device->vkGetDeviceQueue(m_device->getHandle(), m_queueFamilyData.presentIndex, 0, &m_presentQueue);
}

void VulkanRenderer::createThreadCommandPools()
{
	const auto size = TaskManager::instance().getSize() + 1;
	m_threadCommandPools.reserve(size);

	for (size_t i = 0; i < size; i++)
	{
		m_threadCommandPools.push_back(CommandPool::create({QueueType::All}));
	}
}

void VulkanRenderer::unregisterWindows()
{
	for (auto& it : m_windowSurfaces)
	{
		m_instance->vkDestroySurfaceKHR(m_instance->getHandle(), it.second, nullptr);
	}

	m_windowSurfaces.clear();
}
