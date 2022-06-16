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

#include <Atema/VulkanRenderer/VulkanDescriptorSetLayout.hpp>
#include <Atema/VulkanRenderer/VulkanDescriptorPool.hpp>
#include <Atema/VulkanRenderer/VulkanDevice.hpp>

using namespace at;

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const VulkanDevice& device, const DescriptorSetLayout::Settings& settings) :
	DescriptorSetLayout(),
	m_device(device),
	m_descriptorSetLayout(VK_NULL_HANDLE),
	m_bindings(settings.bindings)
{
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

	ATEMA_VK_CHECK(m_device.vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout));

	// Create pool
	m_descriptorPool = std::make_unique<VulkanDescriptorPool>(device, *this, settings.pageSize);
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
{
	m_descriptorPool.reset();

	ATEMA_VK_DESTROY(m_device, vkDestroyDescriptorSetLayout, m_descriptorSetLayout);
}

VkDescriptorSetLayout VulkanDescriptorSetLayout::getHandle() const noexcept
{
	return m_descriptorSetLayout;
}

const std::vector<DescriptorSetBinding>& VulkanDescriptorSetLayout::getBindings() const noexcept
{
	return m_bindings;
}

Ptr<DescriptorSet> VulkanDescriptorSetLayout::createSet()
{
	return m_descriptorPool->createSet();
}