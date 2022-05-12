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

#include <Atema/VulkanRenderer/VulkanInstance.hpp>
#include <Atema/VulkanRenderer/VulkanPhysicalDevice.hpp>

using namespace at;

VulkanInstance::VulkanInstance(const VkInstanceCreateInfo& createInfo) :
	m_instance(nullptr),
	m_apiVersion(0)
{
	auto& vulkan = Vulkan::instance();

	ATEMA_VK_CHECK(vulkan.vkCreateInstance(&createInfo, nullptr, &m_instance));

	m_apiVersion = createInfo.pApplicationInfo->apiVersion;

	for (uint32_t i = 0; i < createInfo.enabledExtensionCount; i++)
		m_extensions.emplace(createInfo.ppEnabledExtensionNames[i]);

	for (uint32_t i = 0; i < createInfo.enabledLayerCount; i++)
		m_layers.emplace(createInfo.ppEnabledLayerNames[i]);

	// Initialize all function pointers to nullptr
#define ATEMA_MACROLIST_VULKAN_INSTANCE_FUNCTION(at_func) at_func = nullptr;
#include <Atema/VulkanRenderer/InstanceFunctionMacroList.hpp>

	// Load required functions
#define ATEMA_MACROLIST_VULKAN_INSTANCE_FUNCTION(at_func) at_func = reinterpret_cast<PFN_ ## at_func>(getProcAddr(#at_func));
#define ATEMA_MACROLIST_VULKAN_INSTANCE_FUNCTION_CORE_VERSION_BEGIN(at_ver) if (m_apiVersion >= (at_ver)) {
#define ATEMA_MACROLIST_VULKAN_INSTANCE_FUNCTION_CORE_VERSION_END }
#define ATEMA_MACROLIST_VULKAN_INSTANCE_FUNCTION_EXTENSION_BEGIN(at_ext) if (isExtensionLoaded(#at_ext)) {
#define ATEMA_MACROLIST_VULKAN_INSTANCE_FUNCTION_EXTENSION_END }
#include <Atema/VulkanRenderer/InstanceFunctionMacroList.hpp>

	// Get physical devices
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

	if (deviceCount == 0)
		ATEMA_ERROR("Failed to find GPUs with Vulkan support");

	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, physicalDevices.data());

	for (auto& physicalDeviceHandle : physicalDevices)
	{
		m_physicalDevices.emplace_back(*this, physicalDeviceHandle);
	}
}

VulkanInstance::~VulkanInstance()
{
	if (m_instance != VK_NULL_HANDLE)
	{
		vkDestroyInstance(m_instance, nullptr);

		m_instance = VK_NULL_HANDLE;
	}
}

VkInstance VulkanInstance::getHandle() const noexcept
{
	return m_instance;
}

VulkanInstance::operator VkInstance() const noexcept
{
	return getHandle();
}

bool VulkanInstance::isExtensionLoaded(const std::string& extensionName) const
{
	return m_extensions.count(extensionName) > 0;
}

bool VulkanInstance::isLayerLoaded(const std::string& layerName) const
{
	return m_layers.count(layerName) > 0;
}

const std::vector<VulkanPhysicalDevice>& VulkanInstance::getPhysicalDevices() const noexcept
{
	return m_physicalDevices;
}

PFN_vkVoidFunction VulkanInstance::getProcAddr(const char* name) const
{
	auto function = Vulkan::instance().vkGetInstanceProcAddr(m_instance, name);

	if (!function)
	{
		ATEMA_ERROR("Failed to get " + std::string(name) + " address");
	}

	return function;
}
