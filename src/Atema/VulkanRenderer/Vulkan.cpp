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

#include <Atema/VulkanRenderer/Vulkan.hpp>
#include <Atema/Core/Library.hpp>
#include <Atema/Renderer/Renderer.hpp>

// Dynamic library name
#ifdef ATEMA_SYSTEM_WINDOWS
#define ATEMA_VULKAN_LIBRARY "vulkan-1.dll"
#elif defined ATEMA_SYSTEM_LINUX
#define ATEMA_VULKAN_LIBRARY "libvulkan.so"
#elif defined ATEMA_SYSTEM_MACOS
#define ATEMA_VULKAN_LIBRARY "libMoltenVK.dylib"
#else
#error Vulkan is not implemented on this OS
#endif

using namespace at;

#define ATEMA_VULKAN_LOAD(at_func) at_func = reinterpret_cast<PFN_ ## at_func>(vkGetInstanceProcAddr(nullptr, #at_func)); \
	ATEMA_ASSERT(at_func, "Failed to get function '" #at_func "'");

Vulkan::Vulkan()
{
	m_vulkanLibrary = std::make_unique<Library>(ATEMA_VULKAN_LIBRARY);

	ATEMA_ASSERT(m_vulkanLibrary->isLoaded(), "Failed to load Vulkan dynamic library");

	vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(m_vulkanLibrary->getFunction("vkGetInstanceProcAddr"));

	ATEMA_ASSERT(vkGetInstanceProcAddr, "Failed to get function 'vkGetInstanceProcAddr'");

	ATEMA_VULKAN_LOAD(vkEnumerateInstanceVersion);
	ATEMA_VULKAN_LOAD(vkEnumerateInstanceLayerProperties);
	ATEMA_VULKAN_LOAD(vkEnumerateInstanceExtensionProperties);
	ATEMA_VULKAN_LOAD(vkCreateInstance);
}

Vulkan::~Vulkan()
{
}

Vulkan& Vulkan::instance()
{
	static Vulkan s_instance;

	return s_instance;
}

VkFormat Vulkan::getFormat(ImageFormat format)
{
	switch (format)
	{
		// Color (8 bit components)
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
		// Color (16 bit components)
		case ImageFormat::R16_UNORM: return VK_FORMAT_R16_UNORM;
		case ImageFormat::R16_SNORM: return VK_FORMAT_R16_SNORM;
		case ImageFormat::R16_USCALED: return VK_FORMAT_R16_USCALED;
		case ImageFormat::R16_SSCALED: return VK_FORMAT_R16_SSCALED;
		case ImageFormat::R16_UINT: return VK_FORMAT_R16_UINT;
		case ImageFormat::R16_SINT: return VK_FORMAT_R16_SINT;
		case ImageFormat::R16_SFLOAT: return VK_FORMAT_R16_SFLOAT;
		case ImageFormat::RG16_UNORM: return VK_FORMAT_R16G16_UNORM;
		case ImageFormat::RG16_SNORM: return VK_FORMAT_R16G16_SNORM;
		case ImageFormat::RG16_USCALED: return VK_FORMAT_R16G16_USCALED;
		case ImageFormat::RG16_SSCALED: return VK_FORMAT_R16G16_SSCALED;
		case ImageFormat::RG16_UINT: return VK_FORMAT_R16G16_UINT;
		case ImageFormat::RG16_SINT: return VK_FORMAT_R16G16_SINT;
		case ImageFormat::RG16_SFLOAT: return VK_FORMAT_R16G16_SFLOAT;
		case ImageFormat::RGB16_UNORM: return VK_FORMAT_R16G16B16_UNORM;
		case ImageFormat::RGB16_SNORM: return VK_FORMAT_R16G16B16_SNORM;
		case ImageFormat::RGB16_USCALED: return VK_FORMAT_R16G16B16_USCALED;
		case ImageFormat::RGB16_SSCALED: return VK_FORMAT_R16G16B16_SSCALED;
		case ImageFormat::RGB16_UINT: return VK_FORMAT_R16G16B16_UINT;
		case ImageFormat::RGB16_SINT: return VK_FORMAT_R16G16B16_SINT;
		case ImageFormat::RGB16_SFLOAT: return VK_FORMAT_R16G16B16_SFLOAT;
		case ImageFormat::RGBA16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;
		case ImageFormat::RGBA16_SNORM: return VK_FORMAT_R16G16B16A16_SNORM;
		case ImageFormat::RGBA16_USCALED: return VK_FORMAT_R16G16B16A16_USCALED;
		case ImageFormat::RGBA16_SSCALED: return VK_FORMAT_R16G16B16A16_SSCALED;
		case ImageFormat::RGBA16_UINT: return VK_FORMAT_R16G16B16A16_UINT;
		case ImageFormat::RGBA16_SINT: return VK_FORMAT_R16G16B16A16_SINT;
		case ImageFormat::RGBA16_SFLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
		// Color (32 bit components)
		case ImageFormat::R32_UINT: return VK_FORMAT_R32_UINT;
		case ImageFormat::R32_SINT: return VK_FORMAT_R32_SINT;
		case ImageFormat::R32_SFLOAT: return VK_FORMAT_R32_SFLOAT;
		case ImageFormat::RG32_UINT: return VK_FORMAT_R32G32_UINT;
		case ImageFormat::RG32_SINT: return VK_FORMAT_R32G32_SINT;
		case ImageFormat::RG32_SFLOAT: return VK_FORMAT_R32G32_SFLOAT;
		case ImageFormat::RGB32_UINT: return VK_FORMAT_R32G32B32_UINT;
		case ImageFormat::RGB32_SINT: return VK_FORMAT_R32G32B32_SINT;
		case ImageFormat::RGB32_SFLOAT: return VK_FORMAT_R32G32B32_SFLOAT;
		case ImageFormat::RGBA32_UINT: return VK_FORMAT_R32G32B32A32_UINT;
		case ImageFormat::RGBA32_SINT: return VK_FORMAT_R32G32B32A32_SINT;
		case ImageFormat::RGBA32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
		// Color (64 bit components)
		case ImageFormat::R64_UINT: return VK_FORMAT_R64_UINT;
		case ImageFormat::R64_SINT: return VK_FORMAT_R64_SINT;
		case ImageFormat::R64_SFLOAT: return VK_FORMAT_R64_SFLOAT;
		case ImageFormat::RG64_UINT: return VK_FORMAT_R64G64_UINT;
		case ImageFormat::RG64_SINT: return VK_FORMAT_R64G64_SINT;
		case ImageFormat::RG64_SFLOAT: return VK_FORMAT_R64G64_SFLOAT;
		case ImageFormat::RGB64_UINT: return VK_FORMAT_R64G64B64_UINT;
		case ImageFormat::RGB64_SINT: return VK_FORMAT_R64G64B64_SINT;
		case ImageFormat::RGB64_SFLOAT: return VK_FORMAT_R64G64B64_SFLOAT;
		case ImageFormat::RGBA64_UINT: return VK_FORMAT_R64G64B64A64_UINT;
		case ImageFormat::RGBA64_SINT: return VK_FORMAT_R64G64B64A64_SINT;
		case ImageFormat::RGBA64_SFLOAT: return VK_FORMAT_R64G64B64A64_SFLOAT;
		// DepthStencil
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
		// Color (8 bit components)
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
		// Color (16 bit components)
		case VK_FORMAT_R16_UNORM: return ImageFormat::R16_UNORM;
		case VK_FORMAT_R16_SNORM: return ImageFormat::R16_SNORM;
		case VK_FORMAT_R16_USCALED: return ImageFormat::R16_USCALED;
		case VK_FORMAT_R16_SSCALED: return ImageFormat::R16_SSCALED;
		case VK_FORMAT_R16_UINT: return ImageFormat::R16_UINT;
		case VK_FORMAT_R16_SINT: return ImageFormat::R16_SINT;
		case VK_FORMAT_R16_SFLOAT: return ImageFormat::R16_SFLOAT;
		case VK_FORMAT_R16G16_UNORM: return ImageFormat::RG16_UNORM;
		case VK_FORMAT_R16G16_SNORM: return ImageFormat::RG16_SNORM;
		case VK_FORMAT_R16G16_USCALED: return ImageFormat::RG16_USCALED;
		case VK_FORMAT_R16G16_SSCALED: return ImageFormat::RG16_SSCALED;
		case VK_FORMAT_R16G16_UINT: return ImageFormat::RG16_UINT;
		case VK_FORMAT_R16G16_SINT: return ImageFormat::RG16_SINT;
		case VK_FORMAT_R16G16_SFLOAT: return ImageFormat::RG16_SFLOAT;
		case VK_FORMAT_R16G16B16_UNORM: return ImageFormat::RGB16_UNORM;
		case VK_FORMAT_R16G16B16_SNORM: return ImageFormat::RGB16_SNORM;
		case VK_FORMAT_R16G16B16_USCALED: return ImageFormat::RGB16_USCALED;
		case VK_FORMAT_R16G16B16_SSCALED: return ImageFormat::RGB16_SSCALED;
		case VK_FORMAT_R16G16B16_UINT: return ImageFormat::RGB16_UINT;
		case VK_FORMAT_R16G16B16_SINT: return ImageFormat::RGB16_SINT;
		case VK_FORMAT_R16G16B16_SFLOAT: return ImageFormat::RGB16_SFLOAT;
		case VK_FORMAT_R16G16B16A16_UNORM: return ImageFormat::RGBA16_UNORM;
		case VK_FORMAT_R16G16B16A16_SNORM: return ImageFormat::RGBA16_SNORM;
		case VK_FORMAT_R16G16B16A16_USCALED: return ImageFormat::RGBA16_USCALED;
		case VK_FORMAT_R16G16B16A16_SSCALED: return ImageFormat::RGBA16_SSCALED;
		case VK_FORMAT_R16G16B16A16_UINT: return ImageFormat::RGBA16_UINT;
		case VK_FORMAT_R16G16B16A16_SINT: return ImageFormat::RGBA16_SINT;
		case VK_FORMAT_R16G16B16A16_SFLOAT: return ImageFormat::RGBA16_SFLOAT;
		// Color (32 bit components)
		case VK_FORMAT_R32_UINT: return ImageFormat::R32_UINT;
		case VK_FORMAT_R32_SINT: return ImageFormat::R32_SINT;
		case VK_FORMAT_R32_SFLOAT: return ImageFormat::R32_SFLOAT;
		case VK_FORMAT_R32G32_UINT: return ImageFormat::RG32_UINT;
		case VK_FORMAT_R32G32_SINT: return ImageFormat::RG32_SINT;
		case VK_FORMAT_R32G32_SFLOAT: return ImageFormat::RG32_SFLOAT;
		case VK_FORMAT_R32G32B32_UINT: return ImageFormat::RGB32_UINT;
		case VK_FORMAT_R32G32B32_SINT: return ImageFormat::RGB32_SINT;
		case VK_FORMAT_R32G32B32_SFLOAT: return ImageFormat::RGB32_SFLOAT;
		case VK_FORMAT_R32G32B32A32_UINT: return ImageFormat::RGBA32_UINT;
		case VK_FORMAT_R32G32B32A32_SINT: return ImageFormat::RGBA32_SINT;
		case VK_FORMAT_R32G32B32A32_SFLOAT: return ImageFormat::RGBA32_SFLOAT;
		// Color (64 bit components)
		case VK_FORMAT_R64_UINT: return ImageFormat::R64_UINT;
		case VK_FORMAT_R64_SINT: return ImageFormat::R64_SINT;
		case VK_FORMAT_R64_SFLOAT: return ImageFormat::R64_SFLOAT;
		case VK_FORMAT_R64G64_UINT: return ImageFormat::RG64_UINT;
		case VK_FORMAT_R64G64_SINT: return ImageFormat::RG64_SINT;
		case VK_FORMAT_R64G64_SFLOAT: return ImageFormat::RG64_SFLOAT;
		case VK_FORMAT_R64G64B64_UINT: return ImageFormat::RGB64_UINT;
		case VK_FORMAT_R64G64B64_SINT: return ImageFormat::RGB64_SINT;
		case VK_FORMAT_R64G64B64_SFLOAT: return ImageFormat::RGB64_SFLOAT;
		case VK_FORMAT_R64G64B64A64_UINT: return ImageFormat::RGBA64_UINT;
		case VK_FORMAT_R64G64B64A64_SINT: return ImageFormat::RGBA64_SINT;
		case VK_FORMAT_R64G64B64A64_SFLOAT: return ImageFormat::RGBA64_SFLOAT;
		// DepthStencil
		case VK_FORMAT_D32_SFLOAT: return ImageFormat::D32F;
		case VK_FORMAT_D32_SFLOAT_S8_UINT: return ImageFormat::D32F_S8U;
		case VK_FORMAT_D24_UNORM_S8_UINT: return ImageFormat::D24U_S8U;
		default:
		{
			ATEMA_ERROR("Invalid image format");
		}
	}

	return ImageFormat::RGBA8_UNORM;
}

VkFormat Vulkan::getFormat(VertexInputFormat format)
{
	switch (format)
	{
		case VertexInputFormat::R8_UINT: return VK_FORMAT_R8_UINT;
		case VertexInputFormat::R8_SINT: return VK_FORMAT_R8_SINT;
		case VertexInputFormat::RG8_UINT: return VK_FORMAT_R8G8_UINT;
		case VertexInputFormat::RG8_SINT: return VK_FORMAT_R8G8_SINT;
		case VertexInputFormat::RGB8_UINT: return VK_FORMAT_R8G8B8_UINT;
		case VertexInputFormat::RGB8_SINT: return VK_FORMAT_R8G8B8_SINT;
		case VertexInputFormat::RGBA8_UINT: return VK_FORMAT_R8G8B8A8_UINT;
		case VertexInputFormat::RGBA8_SINT: return VK_FORMAT_R8G8B8A8_SINT;
		case VertexInputFormat::R16_UINT: return VK_FORMAT_R16_UINT;
		case VertexInputFormat::R16_SINT: return VK_FORMAT_R16_SINT;
		case VertexInputFormat::R16_SFLOAT: return VK_FORMAT_R16_SFLOAT;
		case VertexInputFormat::RG16_UINT: return VK_FORMAT_R16G16_UINT;
		case VertexInputFormat::RG16_SINT: return VK_FORMAT_R16G16_SINT;
		case VertexInputFormat::RG16_SFLOAT: return VK_FORMAT_R16G16_SFLOAT;
		case VertexInputFormat::RGB16_UINT: return VK_FORMAT_R16G16B16_UINT;
		case VertexInputFormat::RGB16_SINT: return VK_FORMAT_R16G16B16_SINT;
		case VertexInputFormat::RGB16_SFLOAT: return VK_FORMAT_R16G16B16_SFLOAT;
		case VertexInputFormat::RGBA16_UINT: return VK_FORMAT_R16G16B16A16_UINT;
		case VertexInputFormat::RGBA16_SINT: return VK_FORMAT_R16G16B16A16_SINT;
		case VertexInputFormat::RGBA16_SFLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
		case VertexInputFormat::R32_UINT: return VK_FORMAT_R32_UINT;
		case VertexInputFormat::R32_SINT: return VK_FORMAT_R32_SINT;
		case VertexInputFormat::R32_SFLOAT: return VK_FORMAT_R32_SFLOAT;
		case VertexInputFormat::RG32_UINT: return VK_FORMAT_R32G32_UINT;
		case VertexInputFormat::RG32_SINT: return VK_FORMAT_R32G32_SINT;
		case VertexInputFormat::RG32_SFLOAT: return VK_FORMAT_R32G32_SFLOAT;
		case VertexInputFormat::RGB32_UINT: return VK_FORMAT_R32G32B32_UINT;
		case VertexInputFormat::RGB32_SINT: return VK_FORMAT_R32G32B32_SINT;
		case VertexInputFormat::RGB32_SFLOAT: return VK_FORMAT_R32G32B32_SFLOAT;
		case VertexInputFormat::RGBA32_UINT: return VK_FORMAT_R32G32B32A32_UINT;
		case VertexInputFormat::RGBA32_SINT: return VK_FORMAT_R32G32B32A32_SINT;
		case VertexInputFormat::RGBA32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
		case VertexInputFormat::R64_UINT: return VK_FORMAT_R64_UINT;
		case VertexInputFormat::R64_SINT: return VK_FORMAT_R64_SINT;
		case VertexInputFormat::R64_SFLOAT: return VK_FORMAT_R64_SFLOAT;
		case VertexInputFormat::RG64_UINT: return VK_FORMAT_R64G64_UINT;
		case VertexInputFormat::RG64_SINT: return VK_FORMAT_R64G64_SINT;
		case VertexInputFormat::RG64_SFLOAT: return VK_FORMAT_R64G64_SFLOAT;
		case VertexInputFormat::RGB64_UINT: return VK_FORMAT_R64G64B64_UINT;
		case VertexInputFormat::RGB64_SINT: return VK_FORMAT_R64G64B64_SINT;
		case VertexInputFormat::RGB64_SFLOAT: return VK_FORMAT_R64G64B64_SFLOAT;
		case VertexInputFormat::RGBA64_UINT: return VK_FORMAT_R64G64B64A64_UINT;
		case VertexInputFormat::RGBA64_SINT: return VK_FORMAT_R64G64B64A64_SINT;
		case VertexInputFormat::RGBA64_SFLOAT: return VK_FORMAT_R64G64B64A64_SFLOAT;
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

	if (Renderer::isDepthImageFormat(format))
		aspect |= VK_IMAGE_ASPECT_DEPTH_BIT;

	if (Renderer::isStencilImageFormat(format))
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

	if (usages & ImageUsage::ShaderSampling)
		flags |= VK_IMAGE_USAGE_SAMPLED_BIT;

	if (usages & ImageUsage::ShaderInput)
		flags |= VK_IMAGE_USAGE_SAMPLED_BIT;

	if (usages & ImageUsage::TransferSrc)
		flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	if (usages & ImageUsage::TransferDst)
		flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

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
		case ImageLayout::ShaderRead: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
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

Flags<ImageSamples> Vulkan::getSamples(VkSampleCountFlags samples)
{
	Flags<ImageSamples> flags = 0;

	if (samples & VK_SAMPLE_COUNT_1_BIT)
		flags |= ImageSamples::S1;

	if (samples & VK_SAMPLE_COUNT_2_BIT)
		flags |= ImageSamples::S2;

	if (samples & VK_SAMPLE_COUNT_4_BIT)
		flags |= ImageSamples::S4;

	if (samples & VK_SAMPLE_COUNT_8_BIT)
		flags |= ImageSamples::S8;

	if (samples & VK_SAMPLE_COUNT_16_BIT)
		flags |= ImageSamples::S16;

	if (samples & VK_SAMPLE_COUNT_32_BIT)
		flags |= ImageSamples::S32;

	if (samples & VK_SAMPLE_COUNT_64_BIT)
		flags |= ImageSamples::S64;
	
	return flags;
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

	if (value & CullMode::None)
		flags |= VK_CULL_MODE_NONE;

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

VkAccessFlags Vulkan::getMemoryAccesses(Flags<MemoryAccess> value)
{
	VkAccessFlags flags = 0;

	if (value & MemoryAccess::IndirectCommandRead)
		flags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;

	if (value & MemoryAccess::IndexBufferRead)
		flags |= VK_ACCESS_INDEX_READ_BIT;

	if (value & MemoryAccess::VertexBufferRead)
		flags |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

	if (value & MemoryAccess::UniformBufferRead)
		flags |= VK_ACCESS_UNIFORM_READ_BIT;

	if (value & MemoryAccess::InputAttachmentRead)
		flags |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;

	if (value & MemoryAccess::ShaderRead)
		flags |= VK_ACCESS_SHADER_READ_BIT;

	if (value & MemoryAccess::ShaderWrite)
		flags |= VK_ACCESS_SHADER_WRITE_BIT;

	if (value & MemoryAccess::ColorAttachmentRead)
		flags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

	if (value & MemoryAccess::ColorAttachmentWrite)
		flags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	if (value & MemoryAccess::DepthStencilAttachmentRead)
		flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

	if (value & MemoryAccess::DepthStencilAttachmentWrite)
		flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	if (value & MemoryAccess::TransferRead)
		flags |= VK_ACCESS_TRANSFER_READ_BIT;

	if (value & MemoryAccess::TransferWrite)
		flags |= VK_ACCESS_TRANSFER_WRITE_BIT;

	if (value & MemoryAccess::HostRead)
		flags |= VK_ACCESS_HOST_READ_BIT;

	if (value & MemoryAccess::HostWrite)
		flags |= VK_ACCESS_HOST_WRITE_BIT;

	if (value & MemoryAccess::MemoryRead)
		flags |= VK_ACCESS_MEMORY_READ_BIT;

	if (value & MemoryAccess::MemoryWrite)
		flags |= VK_ACCESS_MEMORY_WRITE_BIT;

	return flags;
}

VkBufferUsageFlags Vulkan::getBufferUsages(BufferUsage value)
{
	switch (value)
	{
		case BufferUsage::Vertex:
			return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		case BufferUsage::Index:
			return VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		case BufferUsage::Uniform:
			return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		case BufferUsage::Transfer:
			return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		default:
		{
			ATEMA_ERROR("Invalid buffer usage");
		}
	}

	return 0;
}

VkMemoryPropertyFlags Vulkan::getMemoryProperties(bool mappable)
{
	// Try to go for device memory unless we need to be mappable
	if (mappable)
		return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
}

VkIndexType Vulkan::getIndexType(IndexType value)
{
	switch (value)
	{
		case IndexType::U16: return VK_INDEX_TYPE_UINT16;
		case IndexType::U32: return VK_INDEX_TYPE_UINT32;
		default:
		{
			ATEMA_ERROR("Invalid index type");
		}
	}

	return VK_INDEX_TYPE_UINT32;
}

VkFilter Vulkan::getSamplerFilter(SamplerFilter value)
{
	switch (value)
	{
		case SamplerFilter::Nearest: return VK_FILTER_NEAREST;
		case SamplerFilter::Linear: return VK_FILTER_LINEAR;
		default:
		{
			ATEMA_ERROR("Invalid sampler filter");
		}
	}

	return VK_FILTER_NEAREST;
}

VkSamplerAddressMode Vulkan::getSamplerAddressMode(SamplerAddressMode value)
{
	switch (value)
	{
		case SamplerAddressMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case SamplerAddressMode::MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case SamplerAddressMode::ClampToEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case SamplerAddressMode::ClampToBorder: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		default:
		{
			ATEMA_ERROR("Invalid sampler address mode");
		}
	}

	return VK_SAMPLER_ADDRESS_MODE_REPEAT;
}

VkSamplerMipmapMode Vulkan::getSamplerMipmapMode(SamplerFilter value)
{
	switch (value)
	{
		case SamplerFilter::Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		case SamplerFilter::Linear: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		default:
		{
			ATEMA_ERROR("Invalid sampler filter");
		}
	}

	return VK_SAMPLER_MIPMAP_MODE_NEAREST;
}

VkBorderColor Vulkan::getSamplerBorderColor(SamplerBorderColor value)
{
	switch (value)
	{
		case SamplerBorderColor::TransparentInt: return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
		case SamplerBorderColor::TransparentFloat: return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		case SamplerBorderColor::BlackInt: return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		case SamplerBorderColor::BlackFloat: return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		case SamplerBorderColor::WhiteInt: return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
		case SamplerBorderColor::WhiteFloat: return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		default:
		{
			ATEMA_ERROR("Invalid sampler border color");
		}
	}

	return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
}

SwapChainResult Vulkan::getSwapChainResult(VkResult value)
{
	switch (value)
	{
		case VK_SUCCESS:
			return SwapChainResult::Success;
		case VK_NOT_READY:
			return SwapChainResult::NotReady;
		case VK_SUBOPTIMAL_KHR:
			return SwapChainResult::Suboptimal;
		case VK_ERROR_OUT_OF_DATE_KHR:
		case VK_ERROR_SURFACE_LOST_KHR:
			return SwapChainResult::OutOfDate;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		case VK_ERROR_DEVICE_LOST:
		case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
		case VK_TIMEOUT:
		default:
		{
			break;
		}
	}

	return SwapChainResult::Error;
}

bool Vulkan::isQueueFamilyCompatible(QueueType queueType, VkQueueFlags vkFlags)
{
	switch (queueType)
	{
		// We want graphics AND compute bits for this queue type
		case QueueType::Graphics:
			return (vkFlags & VK_QUEUE_GRAPHICS_BIT) && (vkFlags & VK_QUEUE_COMPUTE_BIT);
		// Check for compute bit
		case QueueType::Compute:
			return vkFlags & VK_QUEUE_COMPUTE_BIT;
		// Check transfer capability (graphics & compute queues support transfers)
		case QueueType::Transfer:
			return vkFlags & (VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
		default:
			break;
	}

	return false;
}

