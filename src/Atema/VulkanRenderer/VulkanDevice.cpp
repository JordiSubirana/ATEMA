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

#include <Atema/VulkanRenderer/VulkanDevice.hpp>
#include <Atema/VulkanRenderer/VulkanInstance.hpp>
#include <Atema/VulkanRenderer/VulkanPhysicalDevice.hpp>

using namespace at;

namespace
{
	bool isQueueFamilyCompatible(Flags<QueueType> flags, VkQueueFlags vkFlags)
	{
		// Check graphics bit if needed
		if ((flags & QueueType::Graphics) && !(vkFlags & VK_QUEUE_GRAPHICS_BIT))
			return false;

		// Check compute bit if needed
		if ((flags & QueueType::Compute) && !(vkFlags & VK_QUEUE_COMPUTE_BIT))
			return false;

		// Check transfer capability if needed (graphics & compute queues support transfers)
		if (flags & QueueType::Transfer)
			return vkFlags & (VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

		return true;
	}
}

VulkanDevice::VulkanDevice(const VulkanInstance& instance, const VulkanPhysicalDevice& physicalDevice, const VkDeviceCreateInfo& createInfo) :
	m_instance(instance),
	m_physicalDevice(physicalDevice),
	m_device(nullptr),
	m_version(physicalDevice.getProperties().apiVersion)
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

	// Initialize queues (try to find a unique queue family index for each requirement)
	const std::vector<Flags<QueueType>> queueTypes =
	{
		QueueType::Graphics | QueueType::Compute,
		QueueType::Graphics,
		QueueType::Compute,
		QueueType::Transfer
	};

	std::unordered_set<uint32_t> processedIndices;

	for (auto& queueType : queueTypes)
	{
		uint32_t defaultIndex = std::numeric_limits<uint32_t>::max();

		uint32_t queueIndex = 0;
		for (auto& queueFamily : m_physicalDevice.getQueueFamilyProperties())
		{
			// Check if the queue is compatible
			if (isQueueFamilyCompatible(queueType, queueFamily.queueFlags))
			{
				// Save at least one compatible index (fall back on it if we don't get a unique index)
				defaultIndex = queueIndex;

				// Check if we already processed this family (we want a unique index if possible)
				if (processedIndices.count(defaultIndex) == 0)
					break;
			}

			queueIndex++;
		}

		m_defaultQueueFamilyIndices[static_cast<size_t>(queueType.getValue())] = defaultIndex;
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

uint32_t VulkanDevice::getDefaultQueueFamilyIndex(Flags<QueueType> queueTypes) const
{
	ATEMA_ASSERT(queueTypes, "No queue types specified");

	// If the requirements is not ONLY transfer, remove the bit (every other case implicitly supports it)
	if (queueTypes != QueueType::Transfer)
		queueTypes &= ~(static_cast<int>(QueueType::Transfer));

	return m_defaultQueueFamilyIndices[static_cast<size_t>(queueTypes.getValue())];
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
