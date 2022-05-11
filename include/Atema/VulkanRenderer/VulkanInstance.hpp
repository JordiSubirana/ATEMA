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

#ifndef ATEMA_VULKANRENDERER_VULKANINSTANCE_HPP
#define ATEMA_VULKANRENDERER_VULKANINSTANCE_HPP

#include <Atema/VulkanRenderer/Config.hpp>
#include <Atema/VulkanRenderer/Vulkan.hpp>

#include <unordered_set>

namespace at
{
	class VulkanPhysicalDevice;

	class ATEMA_VULKANRENDERER_API VulkanInstance
	{
	public:
		VulkanInstance() = delete;
		VulkanInstance(const VkInstanceCreateInfo& createInfo);
		~VulkanInstance();

		VkInstance getHandle() const noexcept;

		operator VkInstance() const noexcept;

		bool isExtensionLoaded(const std::string& extensionName) const;
		bool isLayerLoaded(const std::string& layerName) const;

		const std::vector<VulkanPhysicalDevice>& getPhysicalDevices() const noexcept;

		PFN_vkVoidFunction getProcAddr(const char* name) const;

#define ATEMA_MACROLIST_VULKAN_INSTANCE_FUNCTION(at_func) PFN_ ## at_func at_func;
#include <Atema/VulkanRenderer/InstanceFunctionMacroList.hpp>

	private:
		VkInstance m_instance;
		uint32_t m_apiVersion;
		std::unordered_set<std::string> m_extensions;
		std::unordered_set<std::string> m_layers;
		std::vector<VulkanPhysicalDevice> m_physicalDevices;
	};
}

#endif