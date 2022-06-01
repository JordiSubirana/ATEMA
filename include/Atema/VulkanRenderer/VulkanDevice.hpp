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

#ifndef ATEMA_VULKANRENDERER_VULKANDEVICE_HPP
#define ATEMA_VULKANRENDERER_VULKANDEVICE_HPP

#include <Atema/VulkanRenderer/Config.hpp>
#include <Atema/VulkanRenderer/Vulkan.hpp>
#include <Atema/Core/SparseSet.hpp>
#include <Atema/Renderer/Semaphore.hpp>

#include <unordered_set>

namespace at
{
	class Fence;
	class CommandBuffer;
	class CommandPool;
	class VulkanInstance;
	class VulkanPhysicalDevice;

	class ATEMA_VULKANRENDERER_API VulkanDevice
	{
	public:
		VulkanDevice() = delete;
		VulkanDevice(
			const VulkanInstance& instance,
			const VulkanPhysicalDevice& physicalDevice,
			const VkDeviceCreateInfo& createInfo,
			uint32_t graphicsFamilyIndex,
			uint32_t computeFamilyIndex,
			uint32_t transferFamilyIndex);
		~VulkanDevice();

		VkDevice getHandle() const noexcept;

		operator VkDevice() const noexcept;

		const VulkanInstance& getInstance() const noexcept;
		const VulkanPhysicalDevice& getPhysicalDevice() const noexcept;

		VmaAllocator getVmaAllocator() const noexcept;

		uint32_t getQueueFamilyIndex(QueueType queueType) const;

		VkQueue getQueue(QueueType queueType) const;

		Ptr<CommandPool> getDefaultCommandPool(QueueType queueType) const;
		Ptr<CommandPool> getDefaultCommandPool(QueueType queueType, size_t threadIndex) const;

		void submit(const std::vector<Ptr<CommandBuffer>>& commandBuffers, const std::vector<WaitCondition>& waitConditions, const std::vector<Ptr<Semaphore>>& signalSemaphores, Ptr<Fence> fence = nullptr);

		void waitForIdle() const;

		bool isExtensionLoaded(const std::string& extensionName) const;
		bool isLayerLoaded(const std::string& layerName) const;

		PFN_vkVoidFunction getProcAddr(const char* name) const;

#define ATEMA_MACROLIST_VULKAN_DEVICE_FUNCTION(at_func) PFN_ ## at_func at_func;
#include <Atema/VulkanRenderer/DeviceFunctionMacroList.hpp>

	private:
		struct QueueData
		{
			uint32_t familyIndex;
			VkQueue queue;
			std::vector<Ptr<CommandPool>> commandPools;
		};

		const QueueData& getQueueData(QueueType queueType) const;

		const VulkanInstance& m_instance;
		const VulkanPhysicalDevice& m_physicalDevice;
		VkDevice m_device;
		uint32_t m_version;
		std::unordered_set<std::string> m_extensions;
		std::unordered_set<std::string> m_layers;
		std::vector<QueueData> m_queueDatas;
		VmaAllocator m_vmaAllocator;
	};
}

#endif