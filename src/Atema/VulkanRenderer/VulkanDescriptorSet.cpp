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

#include <Atema/VulkanRenderer/VulkanBuffer.hpp>
#include <Atema/VulkanRenderer/VulkanDescriptorSet.hpp>
#include <Atema/VulkanRenderer/VulkanImage.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>
#include <Atema/VulkanRenderer/VulkanSampler.hpp>

using namespace at;

// DescriptorSetLayout
VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const DescriptorSetLayout::Settings& settings) :
	DescriptorSetLayout(),
	m_descriptorSetLayout(VK_NULL_HANDLE),
	m_bindings(settings.bindings)
{
	auto& renderer = VulkanRenderer::instance();
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
	auto& renderer = VulkanRenderer::instance();
	auto device = renderer.getLogicalDeviceHandle();

	ATEMA_VK_DESTROY(device, vkDestroyDescriptorSetLayout, m_descriptorSetLayout);
}

VkDescriptorSetLayout VulkanDescriptorSetLayout::getHandle() const noexcept
{
	return m_descriptorSetLayout;
}

const std::vector<DescriptorSetBinding>& VulkanDescriptorSetLayout::getBindings() const noexcept
{
	return m_bindings;
}

// DescriptorSet
VulkanDescriptorSet::VulkanDescriptorSet(VkDescriptorSet descriptorSet, std::function<void()> destroyCallback) :
	DescriptorSet(),
	m_device(VK_NULL_HANDLE),
	m_descriptorSet(descriptorSet),
	m_destroyCallback(destroyCallback)
{
	auto& renderer = VulkanRenderer::instance();
	m_device = renderer.getLogicalDeviceHandle();
}

VulkanDescriptorSet::~VulkanDescriptorSet()
{
	// Destruction handled by the pool
	m_destroyCallback();
}

VkDescriptorSet VulkanDescriptorSet::getHandle() const noexcept
{
	return m_descriptorSet;
}

void VulkanDescriptorSet::update(
	const std::vector<uint32_t>& bufferBindings,
	const std::vector<uint32_t>& bufferIndices,
	const std::vector<std::vector<Ptr<Buffer>>>& buffers,
	const std::vector<uint32_t>& imageSamplerBindings,
	const std::vector<uint32_t>& imageSamplerIndices,
	const std::vector<std::vector<Ptr<Image>>>& images,
	const std::vector<std::vector<Ptr<Sampler>>>& samplers)
{
	ATEMA_ASSERT(bufferBindings.size() == bufferIndices.size(), "Inconsistent buffer sizes");
	ATEMA_ASSERT(bufferBindings.size() == buffers.size(), "Inconsistent buffer sizes");
	ATEMA_ASSERT(imageSamplerBindings.size() == imageSamplerIndices.size(), "Inconsistent image sampler sizes");
	ATEMA_ASSERT(imageSamplerBindings.size() == images.size(), "Inconsistent image sampler sizes");
	ATEMA_ASSERT(imageSamplerBindings.size() == samplers.size(), "Inconsistent image sampler sizes");

	std::vector<std::vector<VkDescriptorBufferInfo>> descriptorBuffers;
	std::vector<std::vector<VkDescriptorImageInfo>> descriptorImages;
	std::vector<VkWriteDescriptorSet> descriptorWrites;
	
	for (size_t i = 0; i < bufferBindings.size(); i++)
	{
		descriptorBuffers.resize(descriptorBuffers.size() + 1);

		auto& descriptors = descriptorBuffers.back();
		descriptors.reserve(buffers[i].size());
		
		for (auto& buffer : buffers[i])
		{
			VkDescriptorBufferInfo descriptor{};
			descriptor.buffer = std::static_pointer_cast<VulkanBuffer>(buffer)->getHandle();
			descriptor.offset = 0;
			descriptor.range = VK_WHOLE_SIZE;

			descriptors.push_back(descriptor);
		}

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_descriptorSet;
		descriptorWrite.dstBinding = bufferBindings[i]; // Binding in the shader
		descriptorWrite.dstArrayElement = 0; // First index we want to update (descriptors can be arrays)
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // Type of descriptor
		descriptorWrite.descriptorCount = static_cast<uint32_t>(descriptors.size()); // How many elements we want to update
		descriptorWrite.pBufferInfo = descriptors.data(); // Used for descriptors that refer to buffer data
		descriptorWrite.pImageInfo = nullptr; // Used for descriptors that refer to image data
		descriptorWrite.pTexelBufferView = nullptr; // Used for descriptors that refer to buffer views

		descriptorWrites.push_back(descriptorWrite);
	}

	for (size_t i = 0; i < imageSamplerBindings.size(); i++)
	{
		descriptorImages.resize(descriptorImages.size() + 1);

		auto& descriptors = descriptorImages.back();
		descriptors.resize(images[i].size());

		for (size_t j = 0; j < descriptors.size(); j++)
		{
			auto image = std::static_pointer_cast<VulkanImage>(images[i][j]);
			auto sampler = std::static_pointer_cast<VulkanSampler>(samplers[i][j]);

			descriptors[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			descriptors[j].imageView = image->getViewHandle();
			descriptors[j].sampler = sampler->getHandle();
		}

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_descriptorSet;
		descriptorWrite.dstBinding = imageSamplerBindings[i]; // Binding in the shader
		descriptorWrite.dstArrayElement = 0; // First index we want to update (descriptors can be arrays)
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // Type of descriptor
		descriptorWrite.descriptorCount = static_cast<uint32_t>(descriptors.size()); // How many elements we want to update
		descriptorWrite.pBufferInfo = nullptr; // Used for descriptors that refer to buffer data
		descriptorWrite.pImageInfo = descriptors.data(); // Used for descriptors that refer to image data
		descriptorWrite.pTexelBufferView = nullptr; // Used for descriptors that refer to buffer views

		descriptorWrites.push_back(descriptorWrite);
	}

	vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}
