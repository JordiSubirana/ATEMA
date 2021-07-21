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

#include <Atema/Core/Window.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>
#include <Atema/Core/Matrix.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

#include <set>
#include <array>
#include <chrono>
#include <fstream>
#include <unordered_map>

#define ATEMA_VK_DESTROY(deleterFunc, resource) \
	{ \
		if (resource != VK_NULL_HANDLE) \
		{ \
			deleterFunc(m_device, resource, nullptr); \
			resource = VK_NULL_HANDLE; \
		} \
	}

using namespace at;

namespace std
{
	template<> struct hash<Vec3f>
	{
		size_t operator()(Vec3f const& vertex) const noexcept
		{
			size_t h1 = std::hash<double>()(vertex.x);
			size_t h2 = std::hash<double>()(vertex.y);
			size_t h3 = std::hash<double>()(vertex.z);
			return (h1 ^ (h2 << 1)) ^ h3;
		}
	};

	template<> struct hash<Vec2f>
	{
		size_t operator()(Vec2f const& vertex) const noexcept
		{
			return std::hash<Vec3f>()({ vertex.x, vertex.y, 0.0f });
		}
	};

	template<> struct hash<BaseVertex>
	{
		size_t operator()(BaseVertex const& vertex) const noexcept
		{
			return ((hash<Vec3f>()(vertex.pos) ^
				(hash<Vec3f>()(vertex.color) << 1)) >> 1) ^
				(hash<Vec2f>()(vertex.texCoord) << 1);
		}
	};
}

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

	static std::vector<char> readFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			ATEMA_ERROR("Failed to open file : " + filename);
		}

		size_t fileSize = static_cast<size_t>(file.tellg());

		std::vector<char> buffer(fileSize);

		file.seekg(0);

		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	const std::string rsc_path = "../../examples/Sandbox/Resources/";
	const std::string model_path = rsc_path + "Models/LampPost.obj";
	const std::string model_texture_path = rsc_path + "Textures/LampPost_Color.png";
	
	struct UniformBufferObject
	{
		Matrix4f model;
		Matrix4f view;
		Matrix4f proj;
	};

	float toRadians(float degrees)
	{
		return degrees * 3.14159f / 180.0f;
	}

	float toDegrees(float radians)
	{
		return radians * 180.0f / 3.14159f;
	}
}

VulkanRenderer::VulkanRenderer(const Renderer::Settings& settings) :
	Renderer(settings),
	m_currentFrame(0),
	m_framebufferResized(false),
	m_instance(VK_NULL_HANDLE),
	m_surface(VK_NULL_HANDLE),
	m_physicalDevice(VK_NULL_HANDLE),
	m_device(VK_NULL_HANDLE),
	m_graphicsQueue(VK_NULL_HANDLE),
	m_presentQueue(VK_NULL_HANDLE),
	m_swapChain(VK_NULL_HANDLE),
	m_swapChainImageFormat(),
	m_swapChainExtent(),
	m_renderPass(VK_NULL_HANDLE),
	m_descriptorSetLayout(VK_NULL_HANDLE),
	m_pipelineLayout(VK_NULL_HANDLE),
	m_graphicsPipeline(VK_NULL_HANDLE),
	m_commandPool(VK_NULL_HANDLE),
	m_vertexBuffer(VK_NULL_HANDLE),
	m_vertexBufferMemory(VK_NULL_HANDLE),
	m_indexBuffer(VK_NULL_HANDLE),
	m_indexBufferMemory(VK_NULL_HANDLE),
	m_descriptorPool(VK_NULL_HANDLE),
	m_textureMipLevels(0),
	m_textureImage(VK_NULL_HANDLE),
	m_textureImageMemory(VK_NULL_HANDLE),
	m_textureImageView(VK_NULL_HANDLE),
	m_textureSampler(VK_NULL_HANDLE),
	m_msaaSamples(VK_SAMPLE_COUNT_1_BIT),
	m_colorImage(VK_NULL_HANDLE),
	m_colorImageMemory(VK_NULL_HANDLE),
	m_colorImageView(VK_NULL_HANDLE),
	m_depthImage(VK_NULL_HANDLE),
	m_depthImageMemory(VK_NULL_HANDLE),
	m_depthImageView(VK_NULL_HANDLE)
{
}

VulkanRenderer::~VulkanRenderer()
{
	destroy();
}

void VulkanRenderer::initialize()
{
	createInstance();

	createSurface();

	getPhysicalDevice();

	createDevice();

	createSwapChain();

	createImageViews();

	createRenderPass();

	createDescriptorSetLayout();

	createGraphicsPipeline();

	createCommandPool();

	createColorResources();

	createDepthResources();

	createFramebuffers();

	createTextureImage();

	createTextureImageView();

	createTextureSampler();

	loadModel();

	createVertexBuffer();

	createIndexBuffer();

	createUniformBuffers();

	createDescriptorPool();

	createDescriptorSets();

	createCommandBuffers();

	createSemaphores();

	createFences();
}

bool VulkanRenderer::checkValidationLayerSupport()
{
	// Used for debugging purpose
	if (!validationLayers.empty())
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

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
	vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

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

	ATEMA_VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_instance));

	m_vulkan = std::make_unique<Vulkan>(m_instance);
}

void VulkanRenderer::createSurface()
{
#ifdef ATEMA_SYSTEM_WINDOWS
	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = static_cast<HWND>(getMainWindow()->getHandle());
	createInfo.hinstance = GetModuleHandle(nullptr);

	ATEMA_VK_CHECK(vkCreateWin32SurfaceKHR(m_instance, &createInfo, nullptr, &m_surface));
#else
#error VulkanRenderer is not available on this OS
#endif
}

VulkanRenderer::QueueFamilyData VulkanRenderer::getQueueFamilyData(VkPhysicalDevice device)
{
	QueueFamilyData queueFamilyData;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int index = 0;

	for (auto& queueFamily : queueFamilies)
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
			vkGetPhysicalDeviceSurfaceSupportKHR(device, index, m_surface, &presentSupport);

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

VulkanRenderer::SwapChainSupportDetails VulkanRenderer::getSwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails swapChainSupportDetails;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &swapChainSupportDetails.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		swapChainSupportDetails.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, swapChainSupportDetails.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		swapChainSupportDetails.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, swapChainSupportDetails.presentModes.data());
	}

	return swapChainSupportDetails;
}

bool VulkanRenderer::checkPhysicalDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

VkSampleCountFlagBits VulkanRenderer::getMaxUsableSampleCount(VkPhysicalDevice device)
{
	// Check for max sample count for both color and depth
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

	if (counts & VK_SAMPLE_COUNT_64_BIT)
		return VK_SAMPLE_COUNT_64_BIT;

	if (counts & VK_SAMPLE_COUNT_32_BIT)
		return VK_SAMPLE_COUNT_32_BIT;

	if (counts & VK_SAMPLE_COUNT_16_BIT)
		return VK_SAMPLE_COUNT_16_BIT;

	if (counts & VK_SAMPLE_COUNT_8_BIT)
		return VK_SAMPLE_COUNT_8_BIT;

	if (counts & VK_SAMPLE_COUNT_4_BIT)
		return VK_SAMPLE_COUNT_4_BIT;

	if (counts & VK_SAMPLE_COUNT_2_BIT)
		return VK_SAMPLE_COUNT_2_BIT;

	return VK_SAMPLE_COUNT_1_BIT;
}

int VulkanRenderer::getPhysicalDeviceScore(VkPhysicalDevice device, const QueueFamilyData& queueFamilyData)
{
	//TODO: Make this function complete and custom

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

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
	if (!supportedFeatures.samplerAnisotropy)
		return score;

	switch (deviceProperties.deviceType)
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

void VulkanRenderer::getPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

	if (deviceCount == 0)
		ATEMA_ERROR("Failed to find GPUs with Vulkan support");

	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, physicalDevices.data());

	int currentScore = -1;

	for (auto& physicalDevice : physicalDevices)
	{
		auto queueFamilyData = getQueueFamilyData(physicalDevice);

		auto score = getPhysicalDeviceScore(physicalDevice, queueFamilyData);

		if (score > 0 && score > currentScore)
		{
			currentScore = score;
			m_physicalDevice = physicalDevice;
			m_queueFamilyData = queueFamilyData;
			m_msaaSamples = getMaxUsableSampleCount(physicalDevice);
		}
	}

	if (m_physicalDevice == VK_NULL_HANDLE)
		ATEMA_ERROR("Failed to find a valid physical device");
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

	ATEMA_VK_CHECK(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device));

	// Get required queues
	vkGetDeviceQueue(m_device, m_queueFamilyData.graphicsIndex, 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_device, m_queueFamilyData.presentIndex, 0, &m_presentQueue);
}

VkSurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	//TODO: Make this custom

	// Use SRGB if it is available, because it results in more accurate perceived colors
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	// Use first format by default
	return availableFormats[0];
}

VkPresentModeKHR VulkanRenderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
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

VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
		return capabilities.currentExtent;

	const auto size = getMainWindow()->getSize();
	
	VkExtent2D actualExtent =
	{
		static_cast<uint32_t>(size.x),
		static_cast<uint32_t>(size.y)
	};

	// Clamp extent between min & max extent values supported
	actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
	actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

	return actualExtent;
}

void VulkanRenderer::createSwapChain()
{
	//TODO: Make this custom

	SwapChainSupportDetails swapChainSupport = getSwapChainSupport(m_physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount;
	if (swapChainSupport.capabilities.maxImageCount > 0 &&
		imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // To render directly to it
	//createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT; // To copy from some image (for post-processing)

	uint32_t queueFamilyIndices[] = { m_queueFamilyData.graphicsIndex, m_queueFamilyData.presentIndex };

	if (m_queueFamilyData.graphicsIndex != m_queueFamilyData.presentIndex)
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

	// Get images created within the swapchain (will be cleaned up at swapchain's destruction)
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);

	m_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());

	// Save format & extent
	m_swapChainImageFormat = surfaceFormat.format;
	m_swapChainExtent = extent;
}

VkImageView VulkanRenderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;

	// Describe what the image's purpose is and which part of the image should be accessed
	// Here, no mipmaps, no multiple layers (for stereographic 3D with R/L eyes for example)
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;

	ATEMA_VK_CHECK(vkCreateImageView(m_device, &viewInfo, nullptr, &imageView));

	return imageView;
}

void VulkanRenderer::createImageViews()
{
	//TODO: Make this custom

	m_swapChainImageViews.resize(m_swapChainImages.size());

	for (size_t i = 0; i < m_swapChainImages.size(); i++)
	{
		m_swapChainImageViews[i] = createImageView(m_swapChainImages[i], m_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}

void VulkanRenderer::createRenderPass()
{
	//TODO: Make this custom

	// Define attachments and their properties for the render pass
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_swapChainImageFormat; // Match swapchain format because we're rendering to it
	colorAttachment.samples = m_msaaSamples; // Use with multisampling
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Load, clear, don't care
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Store, don't care
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // We don't care about previous layout because we'll clear the image
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Multisampled image can't be presented directly
	//colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // We'll present the image using the swapchain after rendering

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = m_msaaSamples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve{}; // For multisampling
	colorAttachmentResolve.format = m_swapChainImageFormat;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };

	// Create references to attachments previously defined, for each subpass
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0; // Attachment description index
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // The layout we want during the subpass

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentResolveRef{}; // For multisampling
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Create subpasses
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef; // Index 0 = layout (location = 0) out vec4 in fragment shader
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.pResolveAttachments = &colorAttachmentResolveRef; // For multisampling

	// Subpass dependencies
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // Refers to implicit subpass before the render pass (or after if defined in dstSubpass)
	dependency.dstSubpass = 0; // Refers to the only subpass we have right now
	// Specify the operations to wait on and the stages in which these operations occur
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	// The operations that should wait on this are in the color attachment stage and involve the writing of the color attachment
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	// Create render pass
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	ATEMA_VK_CHECK(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass));
}

VkShaderModule VulkanRenderer::createShaderModule(const std::vector<char>& code)
{
	VkShaderModule shaderModule = VK_NULL_HANDLE;

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	ATEMA_VK_CHECK(vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule));

	return shaderModule;
}

void VulkanRenderer::createDescriptorSetLayout()
{
	// Uniform buffer binding
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

	// Sampler binding
	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	// Regroup all bindings
	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

	// Create layout
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	ATEMA_VK_CHECK(vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout));
}

void VulkanRenderer::createGraphicsPipeline()
{
	//TODO: Make this custom

	auto vertShaderCode = readFile(rsc_path + "Shaders/vert.spv");
	auto fragShaderCode = readFile(rsc_path + "Shaders/frag.spv");

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	// Assign shader modules to a specific pipeline stage
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";
	//vertShaderStageInfo.pSpecializationInfo = nullptr; // Used to specify constants and optimize stuff

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";
	//fragShaderStageInfo.pSpecializationInfo = nullptr; // Used to specify constants and optimize stuff

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	// Vertex input (format of vertex data passed to the shader)
	// Bindings : spacing bewteen data & per-vertex/per-instance
	// Attributes : types, binding to load them from, at which offset
	// TODO: Make custom vertex input
	auto bindingDescription = BaseVertex::getBindingDescription();
	auto attributeDescriptions = BaseVertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	//vertexInputInfo.vertexBindingDescriptionCount = 0;
	//vertexInputInfo.pVertexBindingDescriptions = nullptr;
	//vertexInputInfo.vertexAttributeDescriptionCount = 0;
	//vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	// Input assembly (what kind of geometry)
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewport (region of the framebuffer to draw to)
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_swapChainExtent.width);
	viewport.height = static_cast<float>(m_swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Scissors (use rasterizer to discard pixels)
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_swapChainExtent;

	// Possible to use multiple viewports & scissors but requires to enable a GPU feature (device creation)
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// Rasterizer
	// Takes geometry from vertex shader and create fragments using fragment shaders
	// Applies depth testing, face culling, scissors test
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	// We can clamp fragments outside near/far planes to those planes instead of discarding (requires GPU feature)
	rasterizer.depthClampEnable = VK_FALSE;
	// We can disable rasterization to prevent the geometry to pass to this stage (no output)
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	// Select the polygon mode : fill, point, line (point & line require GPU feature)
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f; // Thicker requires GPU feature
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // None, front, back, both
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // VK_FRONT_FACE_CLOCKWISE
	rasterizer.depthBiasEnable = VK_FALSE;
	//rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	//rasterizer.depthBiasClamp = 0.0f; // Optional
	//rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	// Multisampling (can be used to perform anti-aliasing but requires GPU feature)
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.rasterizationSamples = m_msaaSamples;
	if (getSettings().sampleShading)
	{
		multisampling.sampleShadingEnable = VK_TRUE; // Enable sample shading in the pipeline
		multisampling.minSampleShading = .2f; // Min fraction for sample shading (closer to 1 is smoother)
	}
	else
	{
		multisampling.sampleShadingEnable = VK_FALSE;
	}
	//multisampling.pSampleMask = nullptr; // Optional
	//multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	//multisampling.alphaToOneEnable = VK_FALSE; // Optional

	// Depth & stencil

	// Color blending (configuration per attached framebuffer)
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	// No blending
	///*
	colorBlendAttachment.blendEnable = VK_FALSE;
	//colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	//colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	//colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	//colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	//colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	//colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
	//*/
	// Alpha blending
	/*
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	//*/

	// Color blending (global configuration)
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE; // True to use bitwise combination blending, false to use previous method
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	// Enable depth testing (optionnal if we don't use any depth buffer)
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

	// Dynamic states (instead of being fixed at pipeline creation)
	/*
	VkDynamicState dynamicStates[] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;
	//*/

	// Pipeline layout (to use uniform variables & push constants)
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	//pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	ATEMA_VK_CHECK(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));

	// Pipeline creation
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = m_pipelineLayout;
	pipelineInfo.pDepthStencilState = &depthStencil; // Optional
	//pipelineInfo.pDynamicState = nullptr; // Optional

	pipelineInfo.renderPass = m_renderPass; // Can still be used with other render pass instances (if compatible)
	pipelineInfo.subpass = 0;

	// We can derivate pipelines, which can optimize creation & switch performance
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	//pipelineInfo.basePipelineIndex = -1; // Optional

	ATEMA_VK_CHECK(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline));

	// No need to keep shader modules once the pipeline is created
	vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
	vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
}

void VulkanRenderer::createFramebuffers()
{
	m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

	for (size_t i = 0; i < m_swapChainImageViews.size(); i++)
	{
		// The color attachments are different
		// But with the semaphores only one subpass can run at a time, so only one depth image can be used
		std::array<VkImageView, 3> attachments =
		{
			m_colorImageView,
			m_depthImageView,
			m_swapChainImageViews[i]
		};


		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = m_swapChainExtent.width;
		framebufferInfo.height = m_swapChainExtent.height;
		framebufferInfo.layers = 1;

		ATEMA_VK_CHECK(vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]));
	}
}

void VulkanRenderer::createCommandPool()
{
	//TODO: Make this custom

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = m_queueFamilyData.graphicsIndex;
	// VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : Command buffers may be rerecorded with new commands very often (can optimize memory allocations)
	// VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
	//poolInfo.flags = 0; // Optional

	ATEMA_VK_CHECK(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool));
}

void VulkanRenderer::createCommandBuffers()
{
	//TODO: Make this custom

	// One command buffer per swapchain image
	m_commandBuffers.resize(m_swapChainFramebuffers.size());

	// Create command buffers
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Primary, secondary
	allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

	ATEMA_VK_CHECK(vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data()));

	// Record command buffers
	for (size_t i = 0; i < m_commandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		// VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : Executed once then rerecorded
		// VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : Secondary command buffer that will be entirely within a single render pass
		// VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : Can be resubmitted while it is also already pending execution
		//beginInfo.flags = 0; // Optional
		//beginInfo.pInheritanceInfo = nullptr; // Optional (for secondary command buffers)

		// Start recording (and reset command buffer if it was already recorded)
		ATEMA_VK_CHECK(vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo));

		// Start render pass
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_renderPass;
		renderPassInfo.framebuffer = m_swapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_swapChainExtent;
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		// VK_SUBPASS_CONTENTS_INLINE : render pass in primary command buffer, no secondary buffer will be used
		// VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : render pass commands executed in secondary command buffer
		vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Bind pipeline
		vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

		//TODO: To improve memory management, we should allocate big buffers and use offsets in commands like vkCmdBindVertexBuffers

		// Bind vertex buffer
		VkBuffer vertexBuffers[] = { m_vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, vertexBuffers, offsets);

		// Bind index buffer
		vkCmdBindIndexBuffer(m_commandBuffers[i], m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		// Bind descriptor sets
		vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[i], 0, nullptr);

		// Draw (vertex count, instance count, first vertex, first instance)
		//vkCmdDraw(m_commandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);
		vkCmdDrawIndexed(m_commandBuffers[i], static_cast<uint32_t>(m_modelIndices.size()), 1, 0, 0, 0);

		// End render pass
		vkCmdEndRenderPass(m_commandBuffers[i]);

		// End recording
		ATEMA_VK_CHECK(vkEndCommandBuffer(m_commandBuffers[i]));
	}
}

void VulkanRenderer::createSemaphores()
{
	// We need semaphores to do GPU-GPU synchronization between rendering stages
	size_t semaphoreCount = getSettings().maxFramesInFlight;

	m_imageAvailableSemaphores.resize(semaphoreCount);
	m_renderFinishedSemaphores.resize(semaphoreCount);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for (size_t i = 0; i < semaphoreCount; i++)
	{
		ATEMA_VK_CHECK(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]));
		ATEMA_VK_CHECK(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]));
	}
}

void VulkanRenderer::createFences()
{
	// We need fences to do CPU-GPU synchronization and ensure we don't submit more frames than the max allowed
	size_t fenceCount = getSettings().maxFramesInFlight;

	m_inFlightFences.resize(fenceCount);

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Specify initial state

	for (size_t i = 0; i < fenceCount; i++)
	{
		ATEMA_VK_CHECK(vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]));
	}

	// We also need some more fences to prevent the case when we have more frames in flight than swapchain images
	// In this case the app could try to draw in a frame that is already in flight, so we want to prevent it
	// Here we'll store fences from the inFlightFences member so we just initialize the size from the swapchain
	m_imagesInFlight.resize(m_swapChainImages.size(), VK_NULL_HANDLE);
}

uint32_t VulkanRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	//TODO: Make this custom
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) &&
			(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	ATEMA_ERROR("Failed to find suitable memory type");

	return 0;
}

void VulkanRenderer::createBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
	// Buffer creation
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	ATEMA_VK_CHECK(vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer));

	// Memory allcoation
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : Ensure we can map the memory into CPU visible memory
	// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : Ensure the memory can be written at the time we specify it
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	//TODO: Should not be called for individual buffers
	// Check physical device's maxMemoryAllocationCount
	// Use an allocator with offset parameters
	ATEMA_VK_CHECK(vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory));

	// Set the memory to the buffer
	ATEMA_VK_CHECK(vkBindBufferMemory(m_device, buffer, bufferMemory, 0));
}

void VulkanRenderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(commandBuffer);
}

void VulkanRenderer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent =
	{
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	endSingleTimeCommands(commandBuffer);
}

void VulkanRenderer::createImage(VkImage& image, VkDeviceMemory& imageMemory, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits sampleCount, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
{
	// Create image
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format; // Use the same format than the buffer
	imageInfo.tiling = tiling; // Optimal or linear if we want to change pixels client side
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Here used by only one queue
	imageInfo.samples = sampleCount;
	imageInfo.flags = 0; // Optional

	ATEMA_VK_CHECK(vkCreateImage(m_device, &imageInfo, nullptr, &image));

	// Allocate image memory
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	ATEMA_VK_CHECK(vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory));

	vkBindImageMemory(m_device, image, imageMemory, 0);
}

void VulkanRenderer::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
{
	// Apply one shot command buffer
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	// Pipeline barriers are used to synchronize resources (ensure we are not writing & reading at the same time)
	// We can also change layout or change queue family ownership for a resource (when VK_SHARING_MODE_EXCLUSIVE is used)
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

	// Change layout
	barrier.oldLayout = oldLayout; // or VK_IMAGE_LAYOUT_UNDEFINED if we don't care about previous content
	barrier.newLayout = newLayout;

	// Change queue ownership (indices of the queue families if we want to use it)
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	// Specify image & subResourceRange (used for mipmaps or arrays)
	barrier.image = image;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (hasStencilComponent(format))
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	// Depends on what happens before and after the barrier
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	// Undefined to transfer destination : we don't need to wait on anything
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		// We don't need to wait : empty access mask
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		// We don't need to wait : earliest possible pipeline stage
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		// Not a real stage for graphics & compute pipelines
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	// Transfer destination to shader reading : shader reads should wait on transfer writes (here fragment shader)
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		// Earliest stage when we'll need to use the depth buffer (here to read)
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		ATEMA_ERROR("Unsupported layout transition");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		// Depends on what happens before and after the barrier
		sourceStage, // In which pipeline stage the operations occur (before the barrier)
		destinationStage, // The pipeline stage in which operations will wait on the barrier

		0,

		// All types of pipeline barriers can be used
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	endSingleTimeCommands(commandBuffer);
}

void VulkanRenderer::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
	//TODO: Use mipmaps in different files, to avoid generating it a runtime

	// Check if image format supports linear blitting (needed for vkCmdBlitImage)
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(m_physicalDevice, imageFormat, &formatProperties);

	// Here we check in optimal tiling features because our image is in optimal tiling
	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		//TODO: Instead of throwing an error, make a custom mipmap generation without using vkCmdBlitImage
		ATEMA_ERROR("Texture image format does not support linear blitting");
	}

	// Generate mip levels
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for (uint32_t i = 1; i < mipLevels; i++)
	{
		// Transition level i-1 to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
		// Will wait for this level to be filled by previous blit or vkCmdCopyBufferToImage
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		// Which regions will be used for the blit
		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;

		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		// Blit (must be submitted to a queue with graphics capability)
		vkCmdBlitImage(commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		// Transition level i-1 to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		// Will wait on the blit command to finish
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipWidth > 1)
			mipWidth /= 2;

		if (mipHeight > 1)
			mipHeight /= 2;
	}

	// Transition the last level to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	endSingleTimeCommands(commandBuffer);
}

VkFormat VulkanRenderer::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
{
	// Find the first available format in a list from most desirable to least desirable
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	ATEMA_ERROR("Failed to find any supported format");

	return VkFormat{};
}

VkFormat VulkanRenderer::findDepthFormat() const
{
	//TODO: Make this custom
	auto format = findSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);

	return format;
}

bool VulkanRenderer::hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VulkanRenderer::createColorResources()
{
	// Create color image for multisampling
	VkFormat colorFormat = m_swapChainImageFormat;

	createImage(
		m_colorImage,
		m_colorImageMemory,
		m_swapChainExtent.width,
		m_swapChainExtent.height,
		1,
		m_msaaSamples,
		colorFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	m_colorImageView = createImageView(m_colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void VulkanRenderer::createDepthResources()
{
	// Find a supported depth format
	//TODO: Make this custom
	VkFormat depthFormat = findDepthFormat();

	// Create image
	createImage(
		m_depthImage,
		m_depthImageMemory,
		m_swapChainExtent.width,
		m_swapChainExtent.height,
		1,
		m_msaaSamples,
		depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// Create image view
	m_depthImageView = createImageView(m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

	// Transition image layout
	// Here optionnal because we'll handle this in the render pass
	transitionImageLayout(
		m_depthImage,
		depthFormat,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		1);
}

void VulkanRenderer::createTextureImage()
{
	//TODO: Make this custom

	// Load the texture data
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(model_texture_path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	// std::max for the largest dimension, std::log2 to find how many times we can divide by 2
	// std::floor handles the case when the dimension is not power of 2, +1 to add a mip level to the original image
	m_textureMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	if (!pixels)
		ATEMA_ERROR("Failed to load texture image");

	// Staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(
		stagingBuffer,
		stagingBufferMemory,
		imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Fill staging buffer
	void* data;

	vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);

	memcpy(data, pixels, static_cast<size_t>(imageSize));

	vkUnmapMemory(m_device, stagingBufferMemory);

	// Free image data
	stbi_image_free(pixels);

	// Create image
	createImage(
		m_textureImage,
		m_textureImageMemory,
		texWidth,
		texHeight,
		m_textureMipLevels,
		VK_SAMPLE_COUNT_1_BIT,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		// VK_IMAGE_USAGE_TRANSFER_SRC_BIT for mipmap generations and vkCmdBlitImage which is a transfer operation
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// Transition image layout to be able to transfer data from the buffer
	transitionImageLayout(
		m_textureImage,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_UNDEFINED, // We don't care about previous content
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		m_textureMipLevels);

	// Copy staging buffer to image
	copyBufferToImage(
		stagingBuffer,
		m_textureImage,
		static_cast<uint32_t>(texWidth),
		static_cast<uint32_t>(texHeight));

	// Transition image layout to be able to read from the shader
	// Not used because the generateMipmaps function handles this
	/*
	transitionImageLayout(
		m_textureImage,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		m_textureMipLevels);
	//*/

	generateMipmaps(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, m_textureMipLevels);

	// Destroy staging buffer
	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void VulkanRenderer::createTextureImageView()
{
	m_textureImageView = createImageView(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_textureMipLevels);
}

void VulkanRenderer::createTextureSampler()
{
	//TODO: Make this custom

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);

	// Sampler creation
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

	// How to interpolate texels that are magnified/minified
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;

	// REPEAT / MIRRORED_REPEAT / CLAMP_TO_EDGE / MIRROR_CLAMP_TO_EDGE / CLAMP_TO_BORDER
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // Used with CLAMP_TO_BORDER

	// Avoid artifacts when sampling high frequency patterns (undersampling : more texels than fragments)
	// REQUIRES PHYSICAL DEVICE FEATURE (see createDevice())
	//TODO: Can be disabled to increase performance
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

	// VK_TRUE : [0, texWidth) & [0, texHeight) / VK_FALSE : [0, 1) & [0, 1)
	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	// If enabled, texels will first be compared to a value, and the result of that comparison is used in filtering operations
	// Can be used for percentage-closer filtering on shadow maps for example
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	// Mipmaps management
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f; // Optional
	samplerInfo.maxLod = static_cast<float>(m_textureMipLevels);
	samplerInfo.mipLodBias = 0.0f; // Optional

	ATEMA_VK_CHECK(vkCreateSampler(m_device, &samplerInfo, nullptr, &m_textureSampler));
}

void VulkanRenderer::loadModel()
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, model_path.c_str()))
	{
		ATEMA_ERROR(warn + err);
	}

	std::unordered_map<BaseVertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			BaseVertex vertex{};

			vertex.pos =
			{
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			//vertex.texCoord =
			//{
			//	attrib.texcoords[2 * index.texcoord_index + 0],
			//	attrib.texcoords[2 * index.texcoord_index + 1]
			//};

			vertex.texCoord =
			{
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1] // Flip vertical component of texcoord
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			// This method may duplicate vertices and we don't want that
			//m_modelVertices.push_back(vertex);
			//m_modelIndices.push_back(m_modelIndices.size());

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(m_modelVertices.size());
				m_modelVertices.push_back(vertex);
			}

			m_modelIndices.push_back(uniqueVertices[vertex]);
		}
	}
}

void VulkanRenderer::createVertexBuffer()
{
	//TODO: Make this custom

	VkDeviceSize bufferSize = sizeof(m_modelVertices[0]) * m_modelVertices.size();

	// Staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(
		stagingBuffer,
		stagingBufferMemory,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Fill the staging buffer
	void* data = nullptr;

	// Map the buffer memory into CPU visible memory
	ATEMA_VK_CHECK(vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data));

	memcpy(data, m_modelVertices.data(), static_cast<size_t>(bufferSize));

	vkUnmapMemory(m_device, stagingBufferMemory);

	// Vertex buffer
	createBuffer(
		m_vertexBuffer,
		m_vertexBufferMemory,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// Copy staging buffer to vertex buffer
	copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

	// Destroy staging buffer
	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void VulkanRenderer::createIndexBuffer()
{
	//TODO: Make this custom

	VkDeviceSize bufferSize = sizeof(m_modelIndices[0]) * m_modelIndices.size();

	// Staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(
		stagingBuffer,
		stagingBufferMemory,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Fill the staging buffer
	void* data = nullptr;

	// Map the buffer memory into CPU visible memory
	ATEMA_VK_CHECK(vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data));

	memcpy(data, m_modelIndices.data(), static_cast<size_t>(bufferSize));

	vkUnmapMemory(m_device, stagingBufferMemory);

	// Vertex buffer
	createBuffer(
		m_indexBuffer,
		m_indexBufferMemory,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// Copy staging buffer to vertex buffer
	copyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

	// Destroy staging buffer
	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void VulkanRenderer::createUniformBuffers()
{
	//TODO: Make this custom

	// We need to create multiple uniform buffers because the data inside may change every frame
	// And we can't change the content of a resource that is already in use
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	m_uniformBuffers.resize(m_swapChainImages.size());
	m_uniformBuffersMemory.resize(m_swapChainImages.size());

	for (size_t i = 0; i < m_swapChainImages.size(); i++)
	{
		createBuffer(
			m_uniformBuffers[i],
			m_uniformBuffersMemory[i],
			bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}
}

void VulkanRenderer::createDescriptorPool()
{
	//TODO: Make this custom

	// One descriptor per frame
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(m_swapChainImages.size());
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(m_swapChainImages.size());

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(m_swapChainImages.size());

	ATEMA_VK_CHECK(vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool));
}

void VulkanRenderer::createDescriptorSets()
{
	//TODO: Make this custom

	// For now, one descriptor set per frame, with the same layout (we need to copy the layout)
	// Create descriptor sets (will be destroyed when the pool is destroyed)
	std::vector<VkDescriptorSetLayout> layouts(m_swapChainImages.size(), m_descriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(m_swapChainImages.size());
	allocInfo.pSetLayouts = layouts.data();

	m_descriptorSets.resize(m_swapChainImages.size());

	ATEMA_VK_CHECK(vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data()));

	for (size_t i = 0; i < m_swapChainImages.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_textureImageView;
		imageInfo.sampler = m_textureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = m_descriptorSets[i];
		descriptorWrites[0].dstBinding = 0; // Binding in the shader
		descriptorWrites[0].dstArrayElement = 0; // First index we want to update (descriptors can be arrays)
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // Type of descriptor
		descriptorWrites[0].descriptorCount = 1; // How many elements we want to update
		descriptorWrites[0].pBufferInfo = &bufferInfo; // Used for descriptors that refer to buffer data
		descriptorWrites[0].pImageInfo = nullptr; // Used for descriptors that refer to image data
		descriptorWrites[0].pTexelBufferView = nullptr; // Used for descriptors that refer to buffer views

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = m_descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = nullptr;
		descriptorWrites[1].pImageInfo = &imageInfo;
		descriptorWrites[1].pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

VkCommandBuffer VulkanRenderer::beginSingleTimeCommands()
{
	// We could use a different command pool for those actions, to allow implementation to optimize allocations
	// In this case the command pool may be created with VK_COMMAND_POOL_CREATE_TRANSIENT_BIT flag because command buffers will not live long
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void VulkanRenderer::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	// We also could create another queue for transfers and use that one, and use VK_SHARING_MODE_CONCURRENT for resources (specifying queues)
	// OR an even better way : use memory barriers to transfer ownership from one queue to another one
	vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

	// Wait on the transfer to be done
	// We could use fences here, to allow multiple transfers simultaneously
	vkQueueWaitIdle(m_graphicsQueue);

	vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
}

void VulkanRenderer::recreateSwapChain()
{
	// We may need to recreate swapchain because it may be no longer compatible

	// If window is minimized, we just pause it until it shows again
	auto size = getMainWindow()->getSize();

	while (size.x == 0 || size.y == 0)
	{
		size = getMainWindow()->getSize();
		getMainWindow()->processEvents();
	}

	// Resources may be in use, wait until it's not the case anymore
	vkDeviceWaitIdle(m_device);

	// Ensure the resources which depends on swapchain are deleted
	destroySwapChainResources();

	// Some stuff depends on swapchain so we need to recreate it all
	createSwapChain();

	createImageViews();

	createRenderPass();

	// Pipeline layout doesn't need to be recreated because it doesn't depend on extent or format
	// We can avoid pipeline recreation if we use dynamic state for viewport/scissors
	createGraphicsPipeline();

	createColorResources();

	createDepthResources();

	createFramebuffers();

	createUniformBuffers();

	createDescriptorPool();

	createDescriptorSets();

	createCommandBuffers();
}

void VulkanRenderer::destroySwapChainResources()
{
	destroyDepthResources();

	destroyColorResources();

	destroyFramebuffers();

	destroyCommandBuffers();

	destroyGraphicsPipeline();

	destroyRenderPass();

	destroyImageViews();

	destroySwapChain();

	destroyUniformBuffers();

	destroyDescriptorPool();
}

void VulkanRenderer::updateUniformBuffer(uint32_t index)
{
	//To update data frequently, there is a more efficient way : push constants
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();

	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	Vector3f rotation;
	rotation.z = time * toRadians(45.0f);

	Matrix4f basisChange = rotation4f({toRadians(90.0f), 0.0f, 0.0f});

	UniformBufferObject ubo{};
	ubo.model = rotation4f(rotation) * basisChange;
	ubo.view = lookAt({ 80.0f, 0.0f, 80.0f }, { 0.0f, 0.0f, 15.0f }, { 0.0f, 0.0f, 1.0f });
	ubo.proj = perspective(toRadians(45.0f), static_cast<float>(m_swapChainExtent.width) / static_cast<float>(m_swapChainExtent.height), 0.1f, 1000.0f);

	ubo.proj[1][1] *= -1;

	void* data;
	vkMapMemory(m_device, m_uniformBuffersMemory[index], 0, sizeof(ubo), 0, &data);

	memcpy(data, &ubo, sizeof(ubo));

	vkUnmapMemory(m_device, m_uniformBuffersMemory[index]);
}

void VulkanRenderer::destroy()
{
	// Wait for asynchronous stuff to be done
	vkDeviceWaitIdle(m_device);

	destroySwapChainResources();

	destroyTextureSampler();

	destroyTextureImageView();

	destroyTextureImage();

	destroyDescriptorSetLayout();

	destroyIndexBuffer();

	destroyVertexBuffer();

	destroyFences();

	destroySemaphores();

	destroyCommandPool();

	destroyDevice();

	destroySurface();

	destroyInstance();
}

void VulkanRenderer::destroyInstance()
{
	if (m_instance != VK_NULL_HANDLE)
	{
		m_vulkan.reset();

		vkDestroyInstance(m_instance, nullptr);

		m_instance = VK_NULL_HANDLE;
	}
}

void VulkanRenderer::destroySurface()
{
	if (m_surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

		m_surface = VK_NULL_HANDLE;
	}
}

void VulkanRenderer::destroyDevice()
{
	if (m_device != VK_NULL_HANDLE)
	{
		vkDestroyDevice(m_device, nullptr);

		m_device = VK_NULL_HANDLE;
		m_graphicsQueue = VK_NULL_HANDLE;
		m_presentQueue = VK_NULL_HANDLE;
	}
}

void VulkanRenderer::destroySwapChain()
{
	ATEMA_VK_DESTROY(vkDestroySwapchainKHR, m_swapChain);

	m_swapChainImages.clear();
}

void VulkanRenderer::destroyImageViews()
{
	for (auto& imageView : m_swapChainImageViews)
		ATEMA_VK_DESTROY(vkDestroyImageView, imageView);

	m_swapChainImageViews.clear();
}

void VulkanRenderer::destroyRenderPass()
{
	ATEMA_VK_DESTROY(vkDestroyRenderPass, m_renderPass);
}

void VulkanRenderer::destroyDescriptorSetLayout()
{
	ATEMA_VK_DESTROY(vkDestroyDescriptorSetLayout, m_descriptorSetLayout);
}

void VulkanRenderer::destroyDescriptorPool()
{
	ATEMA_VK_DESTROY(vkDestroyDescriptorPool, m_descriptorPool);
}

void VulkanRenderer::destroyGraphicsPipeline()
{
	ATEMA_VK_DESTROY(vkDestroyPipeline, m_graphicsPipeline);
	ATEMA_VK_DESTROY(vkDestroyPipelineLayout, m_pipelineLayout);
}

void VulkanRenderer::destroyFramebuffers()
{
	for (auto& framebuffer : m_swapChainFramebuffers)
		ATEMA_VK_DESTROY(vkDestroyFramebuffer, framebuffer);

	m_swapChainFramebuffers.clear();
}

void VulkanRenderer::destroyCommandPool()
{
	ATEMA_VK_DESTROY(vkDestroyCommandPool, m_commandPool);
}

void VulkanRenderer::destroyCommandBuffers()
{
	if (!m_commandBuffers.empty())
	{
		vkFreeCommandBuffers(m_device, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());

		m_commandBuffers.clear();
	}
}

void VulkanRenderer::destroySemaphores()
{
	for (auto& semaphore : m_imageAvailableSemaphores)
		ATEMA_VK_DESTROY(vkDestroySemaphore, semaphore);

	m_imageAvailableSemaphores.clear();

	for (auto& semaphore : m_renderFinishedSemaphores)
		ATEMA_VK_DESTROY(vkDestroySemaphore, semaphore);

	m_renderFinishedSemaphores.clear();
}

void VulkanRenderer::destroyFences()
{
	for (auto& fence : m_inFlightFences)
		ATEMA_VK_DESTROY(vkDestroyFence, fence);

	m_inFlightFences.clear();
	m_imagesInFlight.clear();
}

void VulkanRenderer::destroyColorResources()
{
	ATEMA_VK_DESTROY(vkDestroyImageView, m_colorImageView);
	ATEMA_VK_DESTROY(vkDestroyImage, m_colorImage);
	ATEMA_VK_DESTROY(vkFreeMemory, m_colorImageMemory);
}

void VulkanRenderer::destroyDepthResources()
{
	ATEMA_VK_DESTROY(vkDestroyImageView, m_depthImageView);
	ATEMA_VK_DESTROY(vkDestroyImage, m_depthImage);
	ATEMA_VK_DESTROY(vkFreeMemory, m_depthImageMemory);
}

void VulkanRenderer::destroyTextureImage()
{
	ATEMA_VK_DESTROY(vkDestroyImage, m_textureImage);
	ATEMA_VK_DESTROY(vkFreeMemory, m_textureImageMemory);
}

void VulkanRenderer::destroyTextureImageView()
{
	ATEMA_VK_DESTROY(vkDestroyImageView, m_textureImageView);
}

void VulkanRenderer::destroyTextureSampler()
{
	ATEMA_VK_DESTROY(vkDestroySampler, m_textureSampler);
}

void VulkanRenderer::destroyVertexBuffer()
{
	ATEMA_VK_DESTROY(vkDestroyBuffer, m_vertexBuffer);
	ATEMA_VK_DESTROY(vkFreeMemory, m_vertexBufferMemory);
}

void VulkanRenderer::destroyIndexBuffer()
{
	ATEMA_VK_DESTROY(vkDestroyBuffer, m_indexBuffer);
	ATEMA_VK_DESTROY(vkFreeMemory, m_indexBufferMemory);
}

void VulkanRenderer::destroyUniformBuffers()
{
	for (auto& buffer : m_uniformBuffers)
		ATEMA_VK_DESTROY(vkDestroyBuffer, buffer);

	m_uniformBuffers.clear();

	for (auto& memory : m_uniformBuffersMemory)
		ATEMA_VK_DESTROY(vkFreeMemory, memory);

	m_uniformBuffersMemory.clear();
}


void VulkanRenderer::drawFrame()
{
	//TODO: Make this custom

	// Wait for fences to put the max frames in flight limit
	vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

	// Draw divided in 3 steps
	// #1 : Acquire an image from the swapchain
	// #2 : Execute the command buffer with that image as attachment in the framebuffer
	// #3 : Return the image to the swap chain for presentation
	// Those steps are executed asynchronously so we need to synchronize with fences or semaphores
	// Fence : Synchonize with app
	// Semaphore : Internal synchronization (better performance if we don't need to check the state)

	// #1 : Acquire an image from the swapchain
	uint32_t imageIndex;
	auto result = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

	// Swapchain can't be used for rendering anymore, we need to recreate it
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain();
		return;
	}
	// Check for errors
	// Suboptimal swapchains can still be used but we can change this to recrate it anyway
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		ATEMA_ERROR("Failed to acquire swapchain image");
	}

	// Check if a previous frame is using this image (i.e. there is its fence to wait on)
	if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
		vkWaitForFences(m_device, 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);

	// Mark the image as now being in use by this frame
	m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];

	// Update uniform data for current frame
	updateUniformBuffer(imageIndex);

	// #2 : Submitting the command buffer
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	// Which semaphores to wait, and on which pipeline stages
	// Here we wait until the image is available to draw on it (so during this time the driver can start vertex shading)
	// To ensure the render pass doesn't begin until we have the image, we could have waited on VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
	// or we can wait on color attachment to be available, and use subpass dependencies (this is done that way, check createRenderPass)
	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	// Command buffers to be submitted
	// Here the one corresponding to the swapchain image
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];

	// Specify what semaphore will be used as the signal that command buffers were executed
	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	// Reset fence & submit command buffers to the target queue (works with arrays for performance)
	vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);

	ATEMA_VK_CHECK(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]));

	// #3 : Presentation
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	// Which semaphores to wait on before presentation can happen
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	// Array of VkResult values to check for every individual swap chain if presentation was successful
	//presentInfo.pResults = nullptr; // Optional

	result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

	// Swapchain can't be used for rendering anymore, we need to recreate it
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized)
	{
		m_framebufferResized = false;
		recreateSwapChain();
	}
	// Check for errors
	else if (result != VK_SUCCESS)
	{
		ATEMA_ERROR("Failed to present swapchain image");
	}

	//vkQueueWaitIdle(m_presentQueue);

	m_currentFrame = (m_currentFrame + 1) % getSettings().maxFramesInFlight;
}