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

#include <Atema/VulkanRenderer/VulkanDevice.hpp>
#include <Atema/VulkanRenderer/VulkanInstance.hpp>
#include <Atema/VulkanRenderer/VulkanPhysicalDevice.hpp>

using namespace at;

VulkanDevice::VulkanDevice(const VulkanInstance& instance, const VulkanPhysicalDevice& physicalDevice, const VkDeviceCreateInfo& createInfo) :
	m_instance(instance),
	m_physicalDevice(physicalDevice),
	m_device(nullptr),
	m_version(physicalDevice.getProperties().apiVersion),
	m_graphicsQueueFamilyIndex(std::numeric_limits<uint32_t>::max()),
	m_transferQueueFamilyIndex(std::numeric_limits<uint32_t>::max())
{
	ATEMA_VK_CHECK(m_instance.vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device));
	
	for (uint32_t i = 0; i < createInfo.enabledExtensionCount; i++)
		m_extensions.emplace(createInfo.ppEnabledExtensionNames[i]);

	for (uint32_t i = 0; i < createInfo.enabledLayerCount; i++)
		m_layers.emplace(createInfo.ppEnabledLayerNames[i]);

	// Initialize all function pointers to nullptr
#define ATEMA_MACROLIST_VULKAN_DEVICE_FUNCTION(at_func) at_func = nullptr;
#include <Atema/VulkanRenderer/DeviceFunctionMacroList.hpp>

	// Load required functions
#define ATEMA_MACROLIST_VULKAN_DEVICE_FUNCTION(at_func) at_func = reinterpret_cast<PFN_ ## at_func>(getProcAddr(#at_func));
#define ATEMA_MACROLIST_VULKAN_DEVICE_FUNCTION_CORE_VERSION_BEGIN(at_ver) if (m_version >= (at_ver)) {
#define ATEMA_MACROLIST_VULKAN_DEVICE_FUNCTION_CORE_VERSION_END }
#define ATEMA_MACROLIST_VULKAN_DEVICE_FUNCTION_EXTENSION_BEGIN(at_ext) if (isExtensionLoaded(#at_ext)) {
#define ATEMA_MACROLIST_VULKAN_DEVICE_FUNCTION_EXTENSION_END }
#include <Atema/VulkanRenderer/DeviceFunctionMacroList.hpp>

	// Initialize queues
	uint32_t queueIndex = 0;
	for (auto& queueFamily : m_physicalDevice.getQueueFamilyProperties())
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			m_graphicsQueueFamilyIndex = queueIndex;
			break;
		}

		queueIndex++;
	}

	queueIndex = 0;
	for (auto& queueFamily : m_physicalDevice.getQueueFamilyProperties())
	{
		// Graphics and compute queues also allow transfers
		VkQueueFlags transferFlags = VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;

		if ((queueFamily.queueFlags & transferFlags) && queueIndex != m_graphicsQueueFamilyIndex)
		{
			m_transferQueueFamilyIndex = queueIndex;
			break;
		}

		queueIndex++;
	}

	// Fall back on graphics queue family if we didn't find another one
	if (m_transferQueueFamilyIndex == std::numeric_limits<uint32_t>::max())
		m_transferQueueFamilyIndex = m_graphicsQueueFamilyIndex;

	if (m_graphicsQueueFamilyIndex == std::numeric_limits<uint32_t>::max())
	{
		ATEMA_ERROR("Device does not support graphics operations");
	}
}

VulkanDevice::~VulkanDevice()
{
	if (m_device != VK_NULL_HANDLE)
	{
		vkDeviceWaitIdle(m_device);

		vkDestroyDevice(m_device, nullptr);

		m_device = VK_NULL_HANDLE;
	}
}

VkDevice VulkanDevice::getHandle() const noexcept
{
	return m_device;
}

VulkanDevice::operator VkDevice() const noexcept
{
	return getHandle();
}

const VulkanInstance& VulkanDevice::getInstance() const noexcept
{
	return m_instance;
}

const VulkanPhysicalDevice& VulkanDevice::getPhysicalDevice() const noexcept
{
	return m_physicalDevice;
}

uint32_t VulkanDevice::getDefaultGraphicsQueueFamilyIndex() const noexcept
{
	return m_graphicsQueueFamilyIndex;
}

uint32_t VulkanDevice::getDefaultTransferQueueFamilyIndex() const noexcept
{
	return m_transferQueueFamilyIndex;
}

void VulkanDevice::waitForIdle()
{
	vkDeviceWaitIdle(m_device);
}

bool VulkanDevice::isExtensionLoaded(const std::string& extensionName) const
{
	return m_extensions.count(extensionName) > 0;
}

bool VulkanDevice::isLayerLoaded(const std::string& layerName) const
{
	return m_layers.count(layerName) > 0;
}

PFN_vkVoidFunction VulkanDevice::getProcAddr(const char* name) const
{
	auto function = m_instance.vkGetDeviceProcAddr(m_device, name);

	if (function)
		return function;

	// Try to get the function from the instance
	return m_instance.getProcAddr(name);
}
