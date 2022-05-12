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

#include <Atema/VulkanRenderer/VulkanPhysicalDevice.hpp>
#include <Atema/VulkanRenderer/VulkanInstance.hpp>

using namespace at;

VulkanPhysicalDevice::VulkanPhysicalDevice(const VulkanInstance& instance, VkPhysicalDevice physicalDevice) :
	m_device(physicalDevice)
{
	// Properties
	instance.vkGetPhysicalDeviceProperties(m_device, &m_deviceProperties);

	// Features
	instance.vkGetPhysicalDeviceFeatures(m_device, &m_supportedFeatures);

	// Memory properties
	instance.vkGetPhysicalDeviceMemoryProperties(m_device, &m_memoryProperties);

	// Queue family properties
	uint32_t queueFamilyCount = 0;
	instance.vkGetPhysicalDeviceQueueFamilyProperties(m_device, &queueFamilyCount, nullptr);

	m_queueFamilyProperties.resize(queueFamilyCount);
	instance.vkGetPhysicalDeviceQueueFamilyProperties(m_device, &queueFamilyCount, m_queueFamilyProperties.data());

	// Extensions
	uint32_t extensionCount;
	instance.vkEnumerateDeviceExtensionProperties(m_device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extensionCount);
	instance.vkEnumerateDeviceExtensionProperties(m_device, nullptr, &extensionCount, extensions.data());

	for (auto& extension : extensions)
		m_extensions.emplace(extension.extensionName);
}

VulkanPhysicalDevice::~VulkanPhysicalDevice()
{
}

VkPhysicalDevice VulkanPhysicalDevice::getHandle() const noexcept
{
	return m_device;
}

VulkanPhysicalDevice::operator VkPhysicalDevice_T*() const noexcept
{
	return getHandle();
}

const VkPhysicalDeviceProperties& VulkanPhysicalDevice::getProperties() const noexcept
{
	return m_deviceProperties;
}

const VkPhysicalDeviceFeatures& VulkanPhysicalDevice::getFeatures() const noexcept
{
	return m_supportedFeatures;
}

const VkPhysicalDeviceMemoryProperties& VulkanPhysicalDevice::getMemoryProperties() const noexcept
{
	return m_memoryProperties;
}

const std::vector<VkQueueFamilyProperties>& VulkanPhysicalDevice::getQueueFamilyProperties() const noexcept
{
	return m_queueFamilyProperties;
}

const std::unordered_set<std::string>& VulkanPhysicalDevice::getExtensions() const noexcept
{
	return m_extensions;
}

bool VulkanPhysicalDevice::hasExtension(const std::string& extension) const
{
	return m_extensions.count(extension) > 0;
}

uint32_t VulkanPhysicalDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
{
	for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) &&
			(m_memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	ATEMA_ERROR("Failed to find suitable memory type");

	return 0;
}
