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

#include <Atema/Core/TaskManager.hpp>
#include <Atema/VulkanRenderer/VulkanCommandBuffer.hpp>
#include <Atema/VulkanRenderer/VulkanCommandPool.hpp>
#include <Atema/VulkanRenderer/VulkanDevice.hpp>
#include <Atema/VulkanRenderer/VulkanFence.hpp>
#include <Atema/VulkanRenderer/VulkanInstance.hpp>
#include <Atema/VulkanRenderer/VulkanPhysicalDevice.hpp>
#include <Atema/VulkanRenderer/VulkanSemaphore.hpp>

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include <vma/vk_mem_alloc.h>

using namespace at;

VulkanDevice::VulkanDevice(
	const VulkanInstance& instance,
	const VulkanPhysicalDevice& physicalDevice,
	const VkDeviceCreateInfo& createInfo,
	uint32_t graphicsFamilyIndex,
	uint32_t computeFamilyIndex,
	uint32_t transferFamilyIndex) :
	m_instance(instance),
	m_physicalDevice(physicalDevice),
	m_device(nullptr),
	m_version(physicalDevice.getProperties().apiVersion),
	m_vmaAllocator(nullptr)
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
	const auto threadCount = TaskManager::instance().getSize();
	constexpr uint32_t invalidIndex = std::numeric_limits<uint32_t>::max();

	const std::vector<uint32_t> queueFamilyIndices =
	{
		graphicsFamilyIndex,
		computeFamilyIndex,
		transferFamilyIndex
	};

	// Initialize other queues (try to find a unique queue family index for each requirement)
	const std::vector<QueueType> queueTypes =
	{
		QueueType::Graphics,
		QueueType::Compute,
		QueueType::Transfer
	};

	for (size_t i = 0; i < queueFamilyIndices.size(); i++)
	{
		auto& queueFamilyIndex = queueFamilyIndices[i];
		auto& queueType = queueTypes[i];

		// Create unique queue family, queue and command pools
		if (i == 0 || queueFamilyIndex != queueFamilyIndices[0])
		{
			QueueData queueData;

			queueData.familyIndex = queueFamilyIndex;

			vkGetDeviceQueue(m_device, queueFamilyIndex, 0, &queueData.queue);

			queueData.commandPools.resize(threadCount + 1);

			CommandPool::Settings commandPoolSettings;
			commandPoolSettings.queueType = queueType;

			for (auto& commandPool : queueData.commandPools)
				commandPool = std::make_shared<VulkanCommandPool>(*this, queueFamilyIndex, commandPoolSettings);

			m_queueDatas.emplace_back(std::move(queueData));
		}
		// Fall back on graphics family data
		else
		{
			m_queueDatas.emplace_back(m_queueDatas[0]);
		}
	}

	// Initialize VMA
	VmaVulkanFunctions vulkanFunctions = {};
	vulkanFunctions.vkGetInstanceProcAddr = Vulkan::instance().vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr = m_instance.vkGetDeviceProcAddr;
	vulkanFunctions.vkGetPhysicalDeviceProperties = m_instance.vkGetPhysicalDeviceProperties;
	vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = m_instance.vkGetPhysicalDeviceMemoryProperties;
	vulkanFunctions.vkAllocateMemory = vkAllocateMemory;
	vulkanFunctions.vkFreeMemory = vkFreeMemory;
	vulkanFunctions.vkMapMemory = vkMapMemory;
	vulkanFunctions.vkUnmapMemory = vkUnmapMemory;
	vulkanFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
	vulkanFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
	vulkanFunctions.vkBindBufferMemory = vkBindBufferMemory;
	vulkanFunctions.vkBindImageMemory = vkBindImageMemory;
	vulkanFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
	vulkanFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
	vulkanFunctions.vkCreateBuffer = vkCreateBuffer;
	vulkanFunctions.vkDestroyBuffer = vkDestroyBuffer;
	vulkanFunctions.vkCreateImage = vkCreateImage;
	vulkanFunctions.vkDestroyImage = vkDestroyImage;
	vulkanFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;
#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
	vulkanFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2;
	vulkanFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2;
#endif
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
	vulkanFunctions.vkBindBufferMemory2KHR = vkBindBufferMemory2;
	vulkanFunctions.vkBindImageMemory2KHR = vkBindImageMemory2;
#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
	vulkanFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = m_instance.vkGetPhysicalDeviceMemoryProperties2;
#endif
#if VMA_VULKAN_VERSION >= 1003000
	vulkanFunctions.vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements;
	vulkanFunctions.vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements;
#endif

	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.vulkanApiVersion = ATEMA_VULKAN_VERSION;
	allocatorCreateInfo.physicalDevice = physicalDevice;
	allocatorCreateInfo.device = m_device;
	allocatorCreateInfo.instance = instance;
	allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

	ATEMA_VK_CHECK(vmaCreateAllocator(&allocatorCreateInfo, &m_vmaAllocator));
}

VulkanDevice::~VulkanDevice()
{
	waitForIdle();

	m_queueDatas.clear();

	if (m_vmaAllocator)
		vmaDestroyAllocator(m_vmaAllocator);

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

VmaAllocator VulkanDevice::getVmaAllocator() const noexcept
{
	return m_vmaAllocator;
}

uint32_t VulkanDevice::getQueueFamilyIndex(QueueType queueType) const
{
	return getQueueData(queueType).familyIndex;
}

VkQueue VulkanDevice::getQueue(QueueType queueType) const
{
	return getQueueData(queueType).queue;
}

Ptr<CommandPool> VulkanDevice::getDefaultCommandPool(QueueType queueType) const
{
	return getQueueData(queueType).commandPools.back();
}

Ptr<CommandPool> VulkanDevice::getDefaultCommandPool(QueueType queueType, size_t threadIndex) const
{
	return getQueueData(queueType).commandPools[threadIndex];
}

void VulkanDevice::submit(
	const std::vector<Ptr<CommandBuffer>>& commandBuffers,
	const std::vector<WaitCondition>& waitConditions,
	const std::vector<Ptr<Semaphore>>& signalSemaphores,
	Ptr<Fence> fence)
{
	ATEMA_ASSERT(!commandBuffers.empty(), "At least one command buffer must be submitted");
	
	const auto queueType = commandBuffers[0]->getQueueType();
	
	// Command buffers
	std::vector<VkCommandBuffer> vkCommandBuffers;
	vkCommandBuffers.reserve(commandBuffers.size());
	for (auto& commandBuffer : commandBuffers)
	{
		if (commandBuffer->getQueueType() != queueType)
		{
			ATEMA_ERROR("All CommandBuffers submitted in a batch must share their QueueType");
		}

		vkCommandBuffers.emplace_back(static_cast<VulkanCommandBuffer&>(*commandBuffer).getHandle());
	}

	// Wait conditions (wait semaphores + pipeline stages)
	std::vector<VkSemaphore> vkWaitSemaphores;
	std::vector<VkPipelineStageFlags> vkWaitStages;
	vkWaitSemaphores.reserve(waitConditions.size());
	vkWaitStages.reserve(waitConditions.size());
	for (auto& waitCondition : waitConditions)
	{
		vkWaitSemaphores.emplace_back(static_cast<VulkanSemaphore&>(*waitCondition.semaphore).getHandle());
		vkWaitStages.emplace_back(Vulkan::getPipelineStages(waitCondition.pipelineStages));
	}

	// Signal semaphores
	std::vector<VkSemaphore> vkSignalSemaphores;
	vkSignalSemaphores.reserve(signalSemaphores.size());
	for (auto& semaphore : signalSemaphores)
	{
		vkSignalSemaphores.emplace_back(static_cast<VulkanSemaphore&>(*semaphore).getHandle());
	}

	// Submission
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

	ATEMA_VK_CHECK(vkQueueSubmit(getQueue(queueType), 1, &submitInfo, vkFence));
}

void VulkanDevice::waitForIdle() const
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

const VulkanDevice::QueueData& VulkanDevice::getQueueData(QueueType queueType) const
{
	return m_queueDatas[static_cast<size_t>(queueType)];
}
