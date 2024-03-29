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

#ifndef ATEMA_VULKANRENDERER_VULKAN_HPP
#define ATEMA_VULKANRENDERER_VULKAN_HPP

#include <Atema/VulkanRenderer/Config.hpp>
#include <Atema/Core/Error.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Renderer/Enums.hpp>
#include <Atema/Renderer/VertexInput.hpp>

#define VK_NO_PROTOTYPES
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

#define ATEMA_VK_ERROR(functionCall, vkResult) ATEMA_ERROR(std::string("Vulkan result invalid : ") + std::to_string(vkResult) + " (" #functionCall ")");

#define ATEMA_VK_CHECK(functionCall) \
	{ \
		auto result = (functionCall); \
		if (result != VK_SUCCESS) \
		{ \
			ATEMA_VK_ERROR(functionCall, result); \
		} \
	}

#define ATEMA_VK_DESTROY(device, deleterFunc, resource) \
	{ \
		if (resource != VK_NULL_HANDLE) \
		{ \
			device.deleterFunc(device, resource, nullptr); \
			resource = VK_NULL_HANDLE; \
		} \
	}

#ifdef ATEMA_SYSTEM_WINDOWS

#include <Atema/Core/Windows.hpp>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan_win32.h>
#endif

#define ATEMA_VULKAN_VERSION VK_API_VERSION_1_1
#define VMA_VULKAN_VERSION 1001000

VK_DEFINE_HANDLE(VmaAllocator)
VK_DEFINE_HANDLE(VmaAllocation)

namespace at
{
	class Library;
	class VulkanDevice;

	class ATEMA_VULKANRENDERER_API Vulkan
	{
	public:
		~Vulkan();

		static Vulkan& instance();

		// Vulkan functions
		PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;

		PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;
		PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
		PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
		PFN_vkCreateInstance vkCreateInstance;

		// Helpers
		static VkFormat getFormat(ImageFormat format);
		static ImageFormat getFormat(VkFormat format);
		static VkFormat getFormat(VertexInputFormat format);

		static VkImageAspectFlags getAspect(ImageFormat format);

		static VkImageType getImageType(ImageType type);
		static VkImageCreateFlags getImageFlags(ImageType type);

		static uint32_t getCubemapLayer(CubemapFace cubemapFace);

		static VkImageTiling getTiling(ImageTiling tiling);

		static VkImageUsageFlags getUsages(Flags<ImageUsage> usages, bool isDepth);

		static VkImageLayout getLayout(ImageLayout layout, bool isDepth);
		
		static VkSampleCountFlagBits getSamples(ImageSamples samples);
		static Flags<ImageSamples> getSamples(VkSampleCountFlags samples);
		
		static VkAttachmentLoadOp getAttachmentLoading(AttachmentLoading value);
		
		static VkAttachmentStoreOp getAttachmentStoring(AttachmentStoring value);

		static VkPrimitiveTopology getPrimitiveTopology(PrimitiveTopology value);

		static VkPolygonMode getPolygonMode(PolygonMode value);

		static VkCullModeFlags getCullMode(Flags<CullMode> value);

		static VkFrontFace getFrontFace(FrontFace value);

		static VkBlendOp getBlendOperation(BlendOperation value);

		static VkBlendFactor getBlendFactor(BlendFactor value);

		static VkCompareOp getCompareOperation(CompareOperation value);

		static VkStencilOp getStencilOperation(StencilOperation value);

		static VkDescriptorType getDescriptorType(DescriptorType value);

		static VkShaderStageFlags getShaderStages(Flags<ShaderStage> value);

		static VkPipelineStageFlags getPipelineStages(Flags<PipelineStage> value);

		static VkAccessFlags getMemoryAccesses(Flags<MemoryAccess> value);

		static VkBufferUsageFlags getBufferUsages(Flags<BufferUsage> value);

		static VkMemoryPropertyFlags getMemoryProperties(bool mappable);

		static VkIndexType getIndexType(IndexType value);

		static VkFilter getSamplerFilter(SamplerFilter value);

		static VkSamplerAddressMode getSamplerAddressMode(SamplerAddressMode value);

		static VkSamplerMipmapMode getSamplerMipmapMode(SamplerFilter value);

		static VkBorderColor getSamplerBorderColor(SamplerBorderColor value);

		static SwapChainResult getSwapChainResult(VkResult value);

		static bool isQueueFamilyCompatible(QueueType queueType, VkQueueFlags vkFlags);

	private:
		Vulkan();

		UPtr<Library> m_vulkanLibrary;
	};
}

#endif
