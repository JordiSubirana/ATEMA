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

#ifndef ATEMA_VULKANRENDERER_VULKANPHYSICALDEVICE_HPP
#define ATEMA_VULKANRENDERER_VULKANPHYSICALDEVICE_HPP

#include <Atema/VulkanRenderer/Config.hpp>
#include <Atema/VulkanRenderer/Vulkan.hpp>

#include <string>
#include <vector>
#include <unordered_set>

namespace at
{
	class VulkanInstance;

	class ATEMA_VULKANRENDERER_API VulkanPhysicalDevice
	{
	public:
		VulkanPhysicalDevice() = delete;
		VulkanPhysicalDevice(const VulkanInstance& instance, VkPhysicalDevice physicalDevice);
		~VulkanPhysicalDevice();

		VkPhysicalDevice getHandle() const noexcept;

		operator VkPhysicalDevice() const noexcept;

		const VkPhysicalDeviceProperties& getProperties() const noexcept;
		const VkPhysicalDeviceFeatures& getFeatures() const noexcept;
		const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const noexcept;
		const std::vector<VkQueueFamilyProperties>& getQueueFamilyProperties() const noexcept;
		const std::unordered_set<std::string>& getExtensions() const noexcept;

		bool hasExtension(const std::string& extension) const;

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

	private:
		VkPhysicalDevice m_device;
		VkPhysicalDeviceProperties m_deviceProperties{};
		VkPhysicalDeviceFeatures m_supportedFeatures{};
		VkPhysicalDeviceMemoryProperties m_memoryProperties{};
		std::vector<VkQueueFamilyProperties> m_queueFamilyProperties;
		std::unordered_set<std::string> m_extensions;
	};
}

#endif