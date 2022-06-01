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
#include <Atema/VulkanRenderer/VulkanImage.hpp>
#include <Atema/VulkanRenderer/VulkanSampler.hpp>
#include <Atema/VulkanRenderer/VulkanRenderPass.hpp>
#include <Atema/VulkanRenderer/VulkanFramebuffer.hpp>
#include <Atema/VulkanRenderer/VulkanShader.hpp>
#include <Atema/VulkanRenderer/VulkanDescriptorSetLayout.hpp>
#include <Atema/VulkanRenderer/VulkanDescriptorSet.hpp>
#include <Atema/VulkanRenderer/VulkanGraphicsPipeline.hpp>
#include <Atema/VulkanRenderer/VulkanCommandPool.hpp>
#include <Atema/VulkanRenderer/VulkanCommandBuffer.hpp>
#include <Atema/VulkanRenderer/VulkanFence.hpp>
#include <Atema/VulkanRenderer/VulkanSemaphore.hpp>
#include <Atema/VulkanRenderer/VulkanBuffer.hpp>
#include <Atema/VulkanRenderer/VulkanRenderWindow.hpp>
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

	const std::vector<const char*> instanceRequiredExtensions =
	{
#ifdef ATEMA_SYSTEM_WINDOWS
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif

		VK_KHR_SURFACE_EXTENSION_NAME
	};
}

VulkanRenderer::VulkanRenderer(const Renderer::Settings& settings) :
	Renderer(settings),
	m_physicalDevice(nullptr),
	m_maxSamples(ImageSamples::S1)
{
}

VulkanRenderer::~VulkanRenderer()
{
	waitForIdle();

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

	pickPhysicalDevice();

	createDevice();
}

void VulkanRenderer::waitForIdle()
{
	m_device->waitForIdle();
}

Ptr<CommandPool> VulkanRenderer::getCommandPool(QueueType queueType)
{
	return m_device->getDefaultCommandPool(queueType);
}

Ptr<CommandPool> VulkanRenderer::getCommandPool(QueueType queueType, size_t threadIndex)
{
	return m_device->getDefaultCommandPool(queueType, threadIndex);
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

Ptr<RenderWindow> VulkanRenderer::createRenderWindow(const RenderWindow::Settings& settings)
{
	auto object = std::make_shared<VulkanRenderWindow>(*m_device, settings);

	return std::static_pointer_cast<RenderWindow>(object);
}

void VulkanRenderer::submit(
	const std::vector<Ptr<CommandBuffer>>& commandBuffers,
	const std::vector<WaitCondition>& waitConditions,
	const std::vector<Ptr<Semaphore>>& signalSemaphores,
	Ptr<Fence> fence)
{
	m_device->submit(commandBuffers, waitConditions, signalSemaphores, fence);
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
	appInfo.apiVersion = ATEMA_VULKAN_VERSION;

	// Check for available extensions
	uint32_t availableExtensionCount = 0;
	Vulkan::instance().vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
	Vulkan::instance().vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

	// Required extensions (instance required + user required)
	// Check if the required extensions are available
	for (auto& requiredExtensionName : instanceRequiredExtensions)
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
	createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceRequiredExtensions.size());
	createInfo.ppEnabledExtensionNames = instanceRequiredExtensions.data();
	createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	createInfo.ppEnabledLayerNames = validationLayers.data();

	m_instance = std::make_unique<VulkanInstance>(createInfo);
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

int VulkanRenderer::getPhysicalDeviceScore(const VulkanPhysicalDevice& physicalDevice)
{
	//TODO: Make this function complete and custom

	int score = -1;

	// Ensure physical device can perform graphics operation
	bool supportGraphics = false;

	for (auto& queueFamilyProperties : physicalDevice.getQueueFamilyProperties())
	{
		if (queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			supportGraphics = true;
			break;
		}
	}

	if (!supportGraphics)
		return score;

	// Check for extension support
	if (!checkPhysicalDeviceExtensionSupport(physicalDevice))
		return score;

	//TODO: Make this an option (see createDevice())
	if (!physicalDevice.getFeatures().samplerAnisotropy)
		return score;

	// Set score depending on device type
	switch (physicalDevice.getProperties().deviceType)
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
		auto score = getPhysicalDeviceScore(physicalDevice);

		if (score > 0 && score > currentScore)
		{
			currentScore = score;
			m_physicalDevice = &physicalDevice;
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

	constexpr uint32_t invalidIndex = std::numeric_limits<uint32_t>::max();

	// Initialize other queues (try to find a unique queue family index for each requirement)
	const std::vector<QueueType> queueTypes =
	{
		QueueType::Graphics,
		QueueType::Compute,
		QueueType::Transfer
	};

	std::vector<uint32_t> queueFamilyIndices(3, invalidIndex);
	std::unordered_set<uint32_t> processedIndices;

	for (auto& queueType : queueTypes)
	{
		// Insert a default invalid index for the given command type
		auto& queueFamilyIndex = queueFamilyIndices[static_cast<size_t>(queueType)];

		// Initialize queue family indices
		uint32_t queueIndex = 0;
		for (auto& queueFamily : m_physicalDevice->getQueueFamilyProperties())
		{
			// Check for queue family compatibility
			const bool isCompatible = Vulkan::isQueueFamilyCompatible(queueType, queueFamily.queueFlags);

			// We want an unique index if possible
			const bool isUnique = (processedIndices.count(queueIndex) == 0);

			// Save at least one compatible index (fall back on it if we don't get a unique index)
			if (isCompatible)
			{
				queueFamilyIndex = queueIndex;

				// We got an unique index, perfect
				if (isUnique)
					break;
			}

			queueIndex++;
		}

		// If invalid, fall back to graphics family (can also manages compute & transfer operations)
		if (queueFamilyIndex == invalidIndex)
		{
			if (queueType == QueueType::Graphics)
			{
				ATEMA_ERROR("Current device does not support graphics operations");
			}

			queueFamilyIndex = queueFamilyIndices[0];
		}
		// If valid, create associated pools for each thread
		else
		{
			processedIndices.emplace(queueFamilyIndex);

			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back(queueCreateInfo);
		}
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

	m_device = std::make_unique<VulkanDevice>(*m_instance, *m_physicalDevice, createInfo, queueFamilyIndices[0], queueFamilyIndices[1], queueFamilyIndices[2]);
}
