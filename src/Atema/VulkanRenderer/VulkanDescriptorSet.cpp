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

#include <Atema/VulkanRenderer/VulkanDescriptorSet.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>

using namespace at;

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const DescriptorSetLayout::Settings& settings) :
	DescriptorSetLayout(),
	m_descriptorSetLayout(VK_NULL_HANDLE)
{
	auto& renderer = VulkanRenderer::getInstance();
	auto device = renderer.getLogicalDeviceHandle();

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	for (auto& binding : settings.bindings)
	{
		VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{};
		descriptorSetLayoutBinding.binding = binding.binding;
		descriptorSetLayoutBinding.descriptorType = Vulkan::getDescriptorType(binding.type);
		descriptorSetLayoutBinding.descriptorCount = binding.count;
		descriptorSetLayoutBinding.stageFlags = Vulkan::getShaderStages(binding.shaderStages);
		descriptorSetLayoutBinding.pImmutableSamplers = nullptr; // Optional

		bindings.push_back(descriptorSetLayoutBinding);
	}
	
	// Create layout
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	ATEMA_VK_CHECK(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_descriptorSetLayout));
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
{
	auto& renderer = VulkanRenderer::getInstance();
	auto device = renderer.getLogicalDeviceHandle();

	ATEMA_VK_DESTROY(device, vkDestroyDescriptorSetLayout, m_descriptorSetLayout);
}

VkDescriptorSetLayout VulkanDescriptorSetLayout::getHandle() const noexcept
{
	return m_descriptorSetLayout;
}
