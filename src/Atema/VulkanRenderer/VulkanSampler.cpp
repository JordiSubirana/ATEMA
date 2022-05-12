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

#include <Atema/VulkanRenderer/VulkanSampler.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>

using namespace at;

VulkanSampler::VulkanSampler(const VulkanDevice& device, const Sampler::Settings& settings) :
	Sampler(),
	m_device(device),
	m_sampler(VK_NULL_HANDLE)
{
	auto& physicalDeviceProperties = m_device.getPhysicalDevice().getProperties();

	// Sampler creation
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

	// How to interpolate texels that are magnified/minified
	samplerInfo.magFilter = Vulkan::getSamplerFilter(settings.magFilter);
	samplerInfo.minFilter = Vulkan::getSamplerFilter(settings.minFilter);

	// REPEAT / MIRRORED_REPEAT / CLAMP_TO_EDGE / MIRROR_CLAMP_TO_EDGE / CLAMP_TO_BORDER
	samplerInfo.addressModeU = Vulkan::getSamplerAddressMode(settings.addressModeU);
	samplerInfo.addressModeV = Vulkan::getSamplerAddressMode(settings.addressModeV);
	samplerInfo.addressModeW = Vulkan::getSamplerAddressMode(settings.addressModeW);
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // Used with CLAMP_TO_BORDER

	// Avoid artifacts when sampling high frequency patterns (undersampling : more texels than fragments)
	// REQUIRES PHYSICAL DEVICE FEATURE (see createDevice())
	//TODO: Can be disabled to increase performance
	samplerInfo.anisotropyEnable = settings.anisotropyEnable ? VK_TRUE : VK_FALSE;
	samplerInfo.maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;

	// VK_TRUE : [0, texWidth) & [0, texHeight) / VK_FALSE : [0, 1) & [0, 1)
	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	// If enabled, texels will first be compared to a value, and the result of that comparison is used in filtering operations
	// Can be used for percentage-closer filtering on shadow maps for example
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	// Mipmaps management
	samplerInfo.mipmapMode = Vulkan::getSamplerMipmapMode(settings.mipmapFilter);
	samplerInfo.minLod = settings.minLod; // Optional
	samplerInfo.maxLod = settings.maxLod < 0.0f ? VK_LOD_CLAMP_NONE : settings.maxLod;
	samplerInfo.mipLodBias = settings.loadBias; // Optional

	ATEMA_VK_CHECK(m_device.vkCreateSampler(m_device, &samplerInfo, nullptr, &m_sampler));
}

VulkanSampler::~VulkanSampler()
{
	ATEMA_VK_DESTROY(m_device, vkDestroySampler, m_sampler);
}

VkSampler VulkanSampler::getHandle() const noexcept
{
	return m_sampler;
}
