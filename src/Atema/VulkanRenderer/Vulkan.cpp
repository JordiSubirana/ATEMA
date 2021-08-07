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

#include <Atema/VulkanRenderer/Vulkan.hpp>

using namespace at;

#define ATEMA_VULKAN_LOAD(AT_FUNCTION) \
	AT_FUNCTION = reinterpret_cast<PFN_ ## AT_FUNCTION>(vkGetInstanceProcAddr(instance, #AT_FUNCTION))

Vulkan::Vulkan(VkInstance instance)
{
	ATEMA_VULKAN_LOAD(vkCreateDebugUtilsMessengerEXT);
	ATEMA_VULKAN_LOAD(vkDestroyDebugUtilsMessengerEXT);

#ifdef ATEMA_SYSTEM_WINDOWS
	ATEMA_VULKAN_LOAD(vkCreateWin32SurfaceKHR);
#endif
}

Vulkan::~Vulkan()
{
}

VkFormat Vulkan::getFormat(ImageFormat format)
{
	switch (format)
	{
		case ImageFormat::R8_UNORM: return VK_FORMAT_R8_UNORM;
		case ImageFormat::R8_SNORM: return VK_FORMAT_R8_SNORM;
		case ImageFormat::R8_USCALED: return VK_FORMAT_R8_USCALED;
		case ImageFormat::R8_SSCALED: return VK_FORMAT_R8_SSCALED;
		case ImageFormat::R8_UINT: return VK_FORMAT_R8_UINT;
		case ImageFormat::R8_SINT: return VK_FORMAT_R8_SINT;
		case ImageFormat::R8_SRGB: return VK_FORMAT_R8_SRGB;
		case ImageFormat::RG8_UNORM: return VK_FORMAT_R8G8_UNORM;
		case ImageFormat::RG8_SNORM: return VK_FORMAT_R8G8_SNORM;
		case ImageFormat::RG8_USCALED: return VK_FORMAT_R8G8_USCALED;
		case ImageFormat::RG8_SSCALED: return VK_FORMAT_R8G8_SSCALED;
		case ImageFormat::RG8_UINT: return VK_FORMAT_R8G8_UINT;
		case ImageFormat::RG8_SINT: return VK_FORMAT_R8G8_SINT;
		case ImageFormat::RG8_SRGB: return VK_FORMAT_R8G8_SRGB;
		case ImageFormat::RGB8_UNORM: return VK_FORMAT_R8G8B8_UNORM;
		case ImageFormat::RGB8_SNORM: return VK_FORMAT_R8G8B8_SNORM;
		case ImageFormat::RGB8_USCALED: return VK_FORMAT_R8G8B8_USCALED;
		case ImageFormat::RGB8_SSCALED: return VK_FORMAT_R8G8B8_SSCALED;
		case ImageFormat::RGB8_UINT: return VK_FORMAT_R8G8B8_UINT;
		case ImageFormat::RGB8_SINT: return VK_FORMAT_R8G8B8_SINT;
		case ImageFormat::RGB8_SRGB: return VK_FORMAT_R8G8B8_SRGB;
		case ImageFormat::BGR8_UNORM: return VK_FORMAT_B8G8R8_UNORM;
		case ImageFormat::BGR8_SNORM: return VK_FORMAT_B8G8R8_SNORM;
		case ImageFormat::BGR8_USCALED: return VK_FORMAT_B8G8R8_USCALED;
		case ImageFormat::BGR8_SSCALED: return VK_FORMAT_B8G8R8_SSCALED;
		case ImageFormat::BGR8_UINT: return VK_FORMAT_B8G8R8_UINT;
		case ImageFormat::BGR8_SINT: return VK_FORMAT_B8G8R8_SINT;
		case ImageFormat::BGR8_SRGB: return VK_FORMAT_B8G8R8_SRGB;
		case ImageFormat::RGBA8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
		case ImageFormat::RGBA8_SNORM: return VK_FORMAT_R8G8B8A8_SNORM;
		case ImageFormat::RGBA8_USCALED: return VK_FORMAT_R8G8B8A8_USCALED;
		case ImageFormat::RGBA8_SSCALED: return VK_FORMAT_R8G8B8A8_SSCALED;
		case ImageFormat::RGBA8_UINT: return VK_FORMAT_R8G8B8A8_UINT;
		case ImageFormat::RGBA8_SINT: return VK_FORMAT_R8G8B8A8_SINT;
		case ImageFormat::RGBA8_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
		case ImageFormat::BGRA8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
		case ImageFormat::BGRA8_SNORM: return VK_FORMAT_B8G8R8A8_SNORM;
		case ImageFormat::BGRA8_USCALED: return VK_FORMAT_B8G8R8A8_USCALED;
		case ImageFormat::BGRA8_SSCALED: return VK_FORMAT_B8G8R8A8_SSCALED;
		case ImageFormat::BGRA8_UINT: return VK_FORMAT_B8G8R8A8_UINT;
		case ImageFormat::BGRA8_SINT: return VK_FORMAT_B8G8R8A8_SINT;
		case ImageFormat::BGRA8_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
		case ImageFormat::D32F: return VK_FORMAT_D32_SFLOAT;
		case ImageFormat::D32F_S8U: return VK_FORMAT_D32_SFLOAT_S8_UINT;
		case ImageFormat::D24U_S8U: return VK_FORMAT_D24_UNORM_S8_UINT;
		default:
		{
			ATEMA_ERROR("Invalid image format");
		}
	}

	return VK_FORMAT_UNDEFINED;
}

VkImageAspectFlags Vulkan::getAspect(ImageFormat format)
{
	VkImageAspectFlags aspect = 0;

	if (hasDepth(format))
		aspect |= VK_IMAGE_ASPECT_DEPTH_BIT;

	if (hasStencil(format))
		aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;

	if (!aspect)
		aspect = VK_IMAGE_ASPECT_COLOR_BIT;

	return aspect;
}

VkImageTiling Vulkan::getTiling(ImageTiling tiling)
{
	switch (tiling)
	{
		case ImageTiling::Optimal: return VK_IMAGE_TILING_OPTIMAL;
		case ImageTiling::Linear: return VK_IMAGE_TILING_LINEAR;
		default:
		{
			ATEMA_ERROR("Invalid image tiling");
		}
	}

	return VK_IMAGE_TILING_LINEAR;
}

VkImageUsageFlags Vulkan::getUsages(Flags<ImageUsage> usages, bool isDepth)
{
	VkImageUsageFlags flags = 0;

	if (usages & ImageUsage::RenderTarget)
	{
		if (isDepth)
			flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		else
			flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}

	if (usages & ImageUsage::ShaderInput)
	{
		//TODO: See for VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
		flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
	}

	if (usages & ImageUsage::TransferSrc)
	{
		flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	if (usages & ImageUsage::TransferDst)
	{
		flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	return flags;
}

VkImageLayout Vulkan::getLayout(ImageLayout layout, bool isDepth)
{
	switch (layout)
	{
		case ImageLayout::Undefined: return VK_IMAGE_LAYOUT_UNDEFINED;
		case ImageLayout::Attachment:
		{
			if (isDepth)
				return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			
			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		case ImageLayout::ShaderInput: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		case ImageLayout::TransferSrc: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		case ImageLayout::TransferDst: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		case ImageLayout::Present: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		case ImageLayout::All: return VK_IMAGE_LAYOUT_GENERAL;
		default:
		{
			ATEMA_ERROR("Invalid image layout");
		}
	}

	return VK_IMAGE_LAYOUT_GENERAL;
}

VkSampleCountFlagBits Vulkan::getSamples(ImageSamples samples)
{
	switch (samples)
	{
		case ImageSamples::S1: return VK_SAMPLE_COUNT_1_BIT;
		case ImageSamples::S2: return VK_SAMPLE_COUNT_2_BIT;
		case ImageSamples::S4: return VK_SAMPLE_COUNT_4_BIT;
		case ImageSamples::S8: return VK_SAMPLE_COUNT_8_BIT;
		case ImageSamples::S16: return VK_SAMPLE_COUNT_16_BIT;
		case ImageSamples::S32: return VK_SAMPLE_COUNT_32_BIT;
		case ImageSamples::S64: return VK_SAMPLE_COUNT_64_BIT;
		default:
		{
			ATEMA_ERROR("Invalid image samples");
		}
	}

	return VK_SAMPLE_COUNT_1_BIT;
}

VkAttachmentLoadOp Vulkan::getAttachmentLoading(AttachmentLoading value)
{
	switch (value)
	{
		case AttachmentLoading::Undefined: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		case AttachmentLoading::Clear: return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case AttachmentLoading::Load: return VK_ATTACHMENT_LOAD_OP_LOAD;
		default:
		{
			ATEMA_ERROR("Invalid attachment load");
		}
	}

	return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
}

VkAttachmentStoreOp Vulkan::getAttachmentStoring(AttachmentStoring value)
{
	switch (value)
	{
		case AttachmentStoring::Undefined: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		case AttachmentStoring::Store: return VK_ATTACHMENT_STORE_OP_STORE;
		default:
		{
			ATEMA_ERROR("Invalid attachment store");
		}
	}

	return VK_ATTACHMENT_STORE_OP_DONT_CARE;
}

