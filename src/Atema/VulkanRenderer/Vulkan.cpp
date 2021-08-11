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

ImageFormat Vulkan::getFormat(VkFormat format)
{
	switch (format)
	{
		case VK_FORMAT_R8_UNORM: return ImageFormat::R8_UNORM;
		case VK_FORMAT_R8_SNORM: return ImageFormat::R8_SNORM;
		case VK_FORMAT_R8_USCALED: return ImageFormat::R8_USCALED;
		case VK_FORMAT_R8_SSCALED: return ImageFormat::R8_SSCALED;
		case VK_FORMAT_R8_UINT: return ImageFormat::R8_UINT;
		case VK_FORMAT_R8_SINT: return ImageFormat::R8_SINT;
		case VK_FORMAT_R8_SRGB: return ImageFormat::R8_SRGB;
		case VK_FORMAT_R8G8_UNORM: return ImageFormat::RG8_UNORM;
		case VK_FORMAT_R8G8_SNORM: return ImageFormat::RG8_SNORM;
		case VK_FORMAT_R8G8_USCALED: return ImageFormat::RG8_USCALED;
		case VK_FORMAT_R8G8_SSCALED: return ImageFormat::RG8_SSCALED;
		case VK_FORMAT_R8G8_UINT: return ImageFormat::RG8_UINT;
		case VK_FORMAT_R8G8_SINT: return ImageFormat::RG8_SINT;
		case VK_FORMAT_R8G8_SRGB: return ImageFormat::RG8_SRGB;
		case VK_FORMAT_R8G8B8_UNORM: return ImageFormat::RGB8_UNORM;
		case VK_FORMAT_R8G8B8_SNORM: return ImageFormat::RGB8_SNORM;
		case VK_FORMAT_R8G8B8_USCALED: return ImageFormat::RGB8_USCALED;
		case VK_FORMAT_R8G8B8_SSCALED: return ImageFormat::RGB8_SSCALED;
		case VK_FORMAT_R8G8B8_UINT: return ImageFormat::RGB8_UINT;
		case VK_FORMAT_R8G8B8_SINT: return ImageFormat::RGB8_SINT;
		case VK_FORMAT_R8G8B8_SRGB: return ImageFormat::RGB8_SRGB;
		case VK_FORMAT_B8G8R8_UNORM: return ImageFormat::BGR8_UNORM;
		case VK_FORMAT_B8G8R8_SNORM: return ImageFormat::BGR8_SNORM;
		case VK_FORMAT_B8G8R8_USCALED: return ImageFormat::BGR8_USCALED;
		case VK_FORMAT_B8G8R8_SSCALED: return ImageFormat::BGR8_SSCALED;
		case VK_FORMAT_B8G8R8_UINT: return ImageFormat::BGR8_UINT;
		case VK_FORMAT_B8G8R8_SINT: return ImageFormat::BGR8_SINT;
		case VK_FORMAT_B8G8R8_SRGB: return ImageFormat::BGR8_SRGB;
		case VK_FORMAT_R8G8B8A8_UNORM: return ImageFormat::RGBA8_UNORM;
		case VK_FORMAT_R8G8B8A8_SNORM: return ImageFormat::RGBA8_SNORM;
		case VK_FORMAT_R8G8B8A8_USCALED: return ImageFormat::RGBA8_USCALED;
		case VK_FORMAT_R8G8B8A8_SSCALED: return ImageFormat::RGBA8_SSCALED;
		case VK_FORMAT_R8G8B8A8_UINT: return ImageFormat::RGBA8_UINT;
		case VK_FORMAT_R8G8B8A8_SINT: return ImageFormat::RGBA8_SINT;
		case VK_FORMAT_R8G8B8A8_SRGB: return ImageFormat::RGBA8_SRGB;
		case VK_FORMAT_B8G8R8A8_UNORM: return ImageFormat::BGRA8_UNORM;
		case VK_FORMAT_B8G8R8A8_SNORM: return ImageFormat::BGRA8_SNORM;
		case VK_FORMAT_B8G8R8A8_USCALED: return ImageFormat::BGRA8_USCALED;
		case VK_FORMAT_B8G8R8A8_SSCALED: return ImageFormat::BGRA8_SSCALED;
		case VK_FORMAT_B8G8R8A8_UINT: return ImageFormat::BGRA8_UINT;
		case VK_FORMAT_B8G8R8A8_SINT: return ImageFormat::BGRA8_SINT;
		case VK_FORMAT_B8G8R8A8_SRGB: return ImageFormat::BGRA8_SRGB;
		case VK_FORMAT_D32_SFLOAT: return ImageFormat::D32F;
		case VK_FORMAT_D32_SFLOAT_S8_UINT: return ImageFormat::D32F_S8U;
		case VK_FORMAT_D24_UNORM_S8_UINT: return ImageFormat::D24U_S8U;
		default:
		{
			ATEMA_ERROR("Invalid image format");
		}
	}

	return ImageFormat::RGBA8_SRGB;
}

VkFormat Vulkan::getFormat(VertexAttribute::Format format)
{
	switch (format)
	{
		case VertexAttribute::Format::R8_UINT: return VK_FORMAT_R8_UINT;
		case VertexAttribute::Format::R8_SINT: return VK_FORMAT_R8_SINT;
		case VertexAttribute::Format::RG8_UINT: return VK_FORMAT_R8G8_UINT;
		case VertexAttribute::Format::RG8_SINT: return VK_FORMAT_R8G8_SINT;
		case VertexAttribute::Format::RGB8_UINT: return VK_FORMAT_R8G8B8_UINT;
		case VertexAttribute::Format::RGB8_SINT: return VK_FORMAT_R8G8B8_SINT;
		case VertexAttribute::Format::RGBA8_UINT: return VK_FORMAT_R8G8B8A8_UINT;
		case VertexAttribute::Format::RGBA8_SINT: return VK_FORMAT_R8G8B8A8_SINT;
		case VertexAttribute::Format::R16_UINT: return VK_FORMAT_R16_UINT;
		case VertexAttribute::Format::R16_SINT: return VK_FORMAT_R16_SINT;
		case VertexAttribute::Format::R16_SFLOAT: return VK_FORMAT_R16_SFLOAT;
		case VertexAttribute::Format::RG16_UINT: return VK_FORMAT_R16G16_UINT;
		case VertexAttribute::Format::RG16_SINT: return VK_FORMAT_R16G16_SINT;
		case VertexAttribute::Format::RG16_SFLOAT: return VK_FORMAT_R16G16_SFLOAT;
		case VertexAttribute::Format::RGB16_UINT: return VK_FORMAT_R16G16B16_UINT;
		case VertexAttribute::Format::RGB16_SINT: return VK_FORMAT_R16G16B16_SINT;
		case VertexAttribute::Format::RGB16_SFLOAT: return VK_FORMAT_R16G16B16_SFLOAT;
		case VertexAttribute::Format::RGBA16_UINT: return VK_FORMAT_R16G16B16A16_UINT;
		case VertexAttribute::Format::RGBA16_SINT: return VK_FORMAT_R16G16B16A16_SINT;
		case VertexAttribute::Format::RGBA16_SFLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
		case VertexAttribute::Format::R32_UINT: return VK_FORMAT_R32_UINT;
		case VertexAttribute::Format::R32_SINT: return VK_FORMAT_R32_SINT;
		case VertexAttribute::Format::R32_SFLOAT: return VK_FORMAT_R32_SFLOAT;
		case VertexAttribute::Format::RG32_UINT: return VK_FORMAT_R32G32_UINT;
		case VertexAttribute::Format::RG32_SINT: return VK_FORMAT_R32G32_SINT;
		case VertexAttribute::Format::RG32_SFLOAT: return VK_FORMAT_R32G32_SFLOAT;
		case VertexAttribute::Format::RGB32_UINT: return VK_FORMAT_R32G32B32_UINT;
		case VertexAttribute::Format::RGB32_SINT: return VK_FORMAT_R32G32B32_SINT;
		case VertexAttribute::Format::RGB32_SFLOAT: return VK_FORMAT_R32G32B32_SFLOAT;
		case VertexAttribute::Format::RGBA32_UINT: return VK_FORMAT_R32G32B32A32_UINT;
		case VertexAttribute::Format::RGBA32_SINT: return VK_FORMAT_R32G32B32A32_SINT;
		case VertexAttribute::Format::RGBA32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
		case VertexAttribute::Format::R64_UINT: return VK_FORMAT_R64_UINT;
		case VertexAttribute::Format::R64_SINT: return VK_FORMAT_R64_SINT;
		case VertexAttribute::Format::R64_SFLOAT: return VK_FORMAT_R64_SFLOAT;
		case VertexAttribute::Format::RG64_UINT: return VK_FORMAT_R64G64_UINT;
		case VertexAttribute::Format::RG64_SINT: return VK_FORMAT_R64G64_SINT;
		case VertexAttribute::Format::RG64_SFLOAT: return VK_FORMAT_R64G64_SFLOAT;
		case VertexAttribute::Format::RGB64_UINT: return VK_FORMAT_R64G64B64_UINT;
		case VertexAttribute::Format::RGB64_SINT: return VK_FORMAT_R64G64B64_SINT;
		case VertexAttribute::Format::RGB64_SFLOAT: return VK_FORMAT_R64G64B64_SFLOAT;
		case VertexAttribute::Format::RGBA64_UINT: return VK_FORMAT_R64G64B64A64_UINT;
		case VertexAttribute::Format::RGBA64_SINT: return VK_FORMAT_R64G64B64A64_SINT;
		case VertexAttribute::Format::RGBA64_SFLOAT: return VK_FORMAT_R64G64B64A64_SFLOAT;
		default:
		{
			ATEMA_ERROR("Invalid vertex attribute format");
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

VkPrimitiveTopology Vulkan::getPrimitiveTopology(PrimitiveTopology value)
{
	switch (value)
	{
		case PrimitiveTopology::PointList: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case PrimitiveTopology::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case PrimitiveTopology::LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		case PrimitiveTopology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case PrimitiveTopology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		case PrimitiveTopology::TriangleFan: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		case PrimitiveTopology::LineListAdjacency: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
		case PrimitiveTopology::LineStripAdjacency: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
		case PrimitiveTopology::TriangleListAdjacency: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
		case PrimitiveTopology::TriangleStripAdjacency: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
		case PrimitiveTopology::PatchList: return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
		default:
		{
			ATEMA_ERROR("Invalid primitive topology");
		}
	}

	return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
}

VkPolygonMode Vulkan::getPolygonMode(PolygonMode value)
{
	switch (value)
	{
		case PolygonMode::Fill: return VK_POLYGON_MODE_FILL;
		case PolygonMode::Line: return VK_POLYGON_MODE_LINE;
		case PolygonMode::Point: return VK_POLYGON_MODE_POINT;
		default:
		{
			ATEMA_ERROR("Invalid polygon mode");
		}
	}

	return VK_POLYGON_MODE_FILL;
}

VkCullModeFlags Vulkan::getCullMode(Flags<CullMode> value)
{
	VkCullModeFlags flags = 0;

	if (value & CullMode::Front)
		flags |= VK_CULL_MODE_FRONT_BIT;

	if (value & CullMode::Back)
		flags |= VK_CULL_MODE_BACK_BIT;

	return flags;
}

VkFrontFace Vulkan::getFrontFace(FrontFace value)
{
	switch (value)
	{
		case FrontFace::Clockwise: return VK_FRONT_FACE_CLOCKWISE;
		case FrontFace::CounterClockwise: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
		default:
		{
			ATEMA_ERROR("Invalid front face");
		}
	}

	return VK_FRONT_FACE_CLOCKWISE;
}

VkBlendOp Vulkan::getBlendOperation(BlendOperation value)
{
	switch (value)
	{
		case BlendOperation::Add: return VK_BLEND_OP_ADD;
		case BlendOperation::Subtract: return VK_BLEND_OP_SUBTRACT;
		case BlendOperation::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
		case BlendOperation::Min: return VK_BLEND_OP_MIN;
		case BlendOperation::Max: return VK_BLEND_OP_MAX;
		default:
		{
			ATEMA_ERROR("Invalid blend operation");
		}
	}

	return VK_BLEND_OP_ADD;
}

VkBlendFactor Vulkan::getBlendFactor(BlendFactor value)
{
	switch (value)
	{
		case BlendFactor::Zero: return VK_BLEND_FACTOR_ZERO;
		case BlendFactor::One: return VK_BLEND_FACTOR_ONE;
		case BlendFactor::SrcColor: return VK_BLEND_FACTOR_SRC_COLOR;
		case BlendFactor::OneMinusSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case BlendFactor::DstColor: return VK_BLEND_FACTOR_DST_COLOR;
		case BlendFactor::OneMinusDstColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case BlendFactor::SrcAlpha: return VK_BLEND_FACTOR_SRC_ALPHA;
		case BlendFactor::OneMinusSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case BlendFactor::DstAlpha: return VK_BLEND_FACTOR_DST_ALPHA;
		case BlendFactor::OneMinusDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case BlendFactor::ConstantColor: return VK_BLEND_FACTOR_CONSTANT_COLOR;
		case BlendFactor::OneMinusConstantColor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		case BlendFactor::ConstantAlpha: return VK_BLEND_FACTOR_CONSTANT_ALPHA;
		case BlendFactor::OneMinusConstantAlpha: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
		default:
		{
			ATEMA_ERROR("Invalid blend factor");
		}
	}

	return VK_BLEND_FACTOR_ZERO;
}

VkCompareOp Vulkan::getCompareOperation(CompareOperation value)
{
	switch (value)
	{
		case CompareOperation::Never: return VK_COMPARE_OP_NEVER;
		case CompareOperation::Less: return VK_COMPARE_OP_LESS;
		case CompareOperation::Equal: return VK_COMPARE_OP_EQUAL;
		case CompareOperation::LessOrEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
		case CompareOperation::Greater: return VK_COMPARE_OP_GREATER;
		case CompareOperation::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
		case CompareOperation::GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case CompareOperation::Always: return VK_COMPARE_OP_ALWAYS;
		default:
		{
			ATEMA_ERROR("Invalid compare operation");
		}
	}

	return VK_COMPARE_OP_LESS;
}

VkStencilOp Vulkan::getStencilOperation(StencilOperation value)
{
	switch (value)
	{
		case StencilOperation::Keep: return VK_STENCIL_OP_KEEP;
		case StencilOperation::Zero: return VK_STENCIL_OP_ZERO;
		case StencilOperation::Replace: return VK_STENCIL_OP_REPLACE;
		case StencilOperation::IncrementAndClamp: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
		case StencilOperation::DecrementAndClamp: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
		case StencilOperation::Invert: return VK_STENCIL_OP_INVERT;
		case StencilOperation::IncrementAndWrap: return VK_STENCIL_OP_INCREMENT_AND_WRAP;
		case StencilOperation::DecrementAndWrap: return VK_STENCIL_OP_DECREMENT_AND_WRAP;
		default:
		{
			ATEMA_ERROR("Invalid stencil operation");
		}
	}

	return VK_STENCIL_OP_KEEP;
}

VkDescriptorType Vulkan::getDescriptorType(DescriptorType value)
{
	switch (value)
	{
		case DescriptorType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
		case DescriptorType::CombinedImageSampler: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case DescriptorType::SampledImage: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case DescriptorType::StorageImage: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case DescriptorType::UniformTexelBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		case DescriptorType::StorageTexelBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		case DescriptorType::UniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case DescriptorType::StorageBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case DescriptorType::UniformBufferDynamic: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		case DescriptorType::StorageBufferDynamic: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		case DescriptorType::InputAttachment: return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		default:
		{
			ATEMA_ERROR("Invalid descriptor type");
		}
	}

	return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
}

VkShaderStageFlags Vulkan::getShaderStages(Flags<ShaderStage> value)
{
	VkShaderStageFlags flags = 0;

	if (value & ShaderStage::Vertex)
		flags |= VK_SHADER_STAGE_VERTEX_BIT;

	if (value & ShaderStage::TessellationControl)
		flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

	if (value & ShaderStage::TessellationEvaluation)
		flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

	if (value & ShaderStage::Geometry)
		flags |= VK_SHADER_STAGE_GEOMETRY_BIT;

	if (value & ShaderStage::Fragment)
		flags |= VK_SHADER_STAGE_FRAGMENT_BIT;

	if (value & ShaderStage::Compute)
		flags |= VK_SHADER_STAGE_COMPUTE_BIT;

	return flags;
}

VkPipelineStageFlags Vulkan::getPipelineStages(Flags<PipelineStage> value)
{
	VkShaderStageFlags flags = 0;

	if (value & PipelineStage::TopOfPipe)
		flags |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	if (value & PipelineStage::DrawIndirect)
		flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

	if (value & PipelineStage::VertexInput)
		flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

	if (value & PipelineStage::VertexShader)
		flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;

	if (value & PipelineStage::TessellationControl)
		flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;

	if (value & PipelineStage::TessellationEvaluation)
		flags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;

	if (value & PipelineStage::GeometryShader)
		flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;

	if (value & PipelineStage::FragmentShader)
		flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

	if (value & PipelineStage::EarlyFragmentTests)
		flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

	if (value & PipelineStage::LateFragmentTests)
		flags |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

	if (value & PipelineStage::ColorAttachmentOutput)
		flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	if (value & PipelineStage::ComputeShader)
		flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

	if (value & PipelineStage::Transfer)
		flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;

	if (value & PipelineStage::BottomOfPipe)
		flags |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	if (value & PipelineStage::Host)
		flags |= VK_PIPELINE_STAGE_HOST_BIT;

	return flags;
}

