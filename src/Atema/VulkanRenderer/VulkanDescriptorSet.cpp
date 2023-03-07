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

#include <Atema/VulkanRenderer/VulkanDescriptorSet.hpp>
#include <Atema/VulkanRenderer/VulkanBuffer.hpp>
#include <Atema/VulkanRenderer/VulkanImageView.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>
#include <Atema/VulkanRenderer/VulkanSampler.hpp>

using namespace at;

VulkanDescriptorSet::VulkanDescriptorSet(const VulkanDevice& device, VkDescriptorSet descriptorSet, const SparseSet<VkDescriptorType>& bindingTypes, std::function<void()> destroyCallback) :
	DescriptorSet(),
	m_device(device),
	m_descriptorSet(descriptorSet),
	m_destroyCallback(destroyCallback),
	m_bindingTypes(bindingTypes)
{
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

void VulkanDescriptorSet::update(uint32_t binding, const Buffer& buffer, size_t offset, size_t size)
{
	VkDescriptorBufferInfo descriptor{};
	descriptor.buffer = static_cast<const VulkanBuffer&>(buffer).getHandle();
	descriptor.offset = static_cast<VkDeviceSize>(offset);
	descriptor.range = size == 0 ? VK_WHOLE_SIZE : static_cast<VkDeviceSize>(size);

	update(binding, 0, 1, nullptr, &descriptor, nullptr);
}

void VulkanDescriptorSet::update(uint32_t binding, uint32_t index, const std::vector<const Buffer*>& buffers)
{
	std::vector<VkDescriptorBufferInfo> descriptors;
	descriptors.reserve(buffers.size());

	for (const auto& buffer : buffers)
	{
		ATEMA_ASSERT(buffer, "Invalid buffer");
		
		auto& descriptor = descriptors.emplace_back();

		descriptor.buffer = static_cast<const VulkanBuffer&>(*buffer).getHandle();
		descriptor.offset = 0;
		descriptor.range = VK_WHOLE_SIZE;
	}

	update(binding, index, descriptors.size(), nullptr, descriptors.data(), nullptr);
}

void VulkanDescriptorSet::update(uint32_t binding, uint32_t index, const std::vector<const Buffer*>& buffers, const std::vector<size_t>& bufferOffsets, const std::vector<size_t>& bufferSizes)
{
	ATEMA_ASSERT(buffers.size() == bufferOffsets.size(), "buffers & bufferOffsets sizes must be equal");
	ATEMA_ASSERT(buffers.size() == bufferSizes.size(), "buffers & bufferSizes sizes must be equal");
	
	std::vector<VkDescriptorBufferInfo> descriptors;
	descriptors.reserve(buffers.size());

	for (size_t i = 0; i < buffers.size(); i++)
	{
		const auto& buffer = buffers[i];
		const auto& offset = bufferOffsets[i];
		const auto& size = bufferSizes[i];
		
		ATEMA_ASSERT(buffer, "Invalid buffer");

		auto& descriptor = descriptors.emplace_back();

		descriptor.buffer = static_cast<const VulkanBuffer&>(*buffer).getHandle();
		descriptor.offset = offset;
		descriptor.range = size == 0 ? VK_WHOLE_SIZE : static_cast<VkDeviceSize>(size);
	}

	update(binding, index, descriptors.size(), nullptr, descriptors.data(), nullptr);
}

void VulkanDescriptorSet::update(uint32_t binding, const ImageView& imageView, const Sampler& sampler)
{
	VkDescriptorImageInfo descriptor{};
	descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	descriptor.imageView = static_cast<const VulkanImageView&>(imageView).getHandle();
	descriptor.sampler = static_cast<const VulkanSampler&>(sampler).getHandle();

	update(binding, 0, 1, &descriptor, nullptr, nullptr);
}

void VulkanDescriptorSet::update(uint32_t binding, uint32_t index, const std::vector<const ImageView*>& imageViews, const std::vector<const Sampler*>& samplers)
{
	ATEMA_ASSERT(imageViews.size() == samplers.size(), "imageViews & samplers sizes must be equal");
	
	std::vector<VkDescriptorImageInfo> descriptors;
	descriptors.reserve(imageViews.size());

	for (size_t i = 0; i < imageViews.size(); i++)
	{
		const auto& imageView = imageViews[i];
		const auto& sampler = samplers[i];

		ATEMA_ASSERT(imageView, "Invalid image view");
		ATEMA_ASSERT(sampler, "Invalid sampler");

		auto& descriptor = descriptors.emplace_back();

		descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		descriptor.imageView = static_cast<const VulkanImageView&>(*imageView).getHandle();
		descriptor.sampler = static_cast<const VulkanSampler&>(*sampler).getHandle();
	}

	update(binding, index, descriptors.size(), descriptors.data(), nullptr, nullptr);
}

void VulkanDescriptorSet::update(uint32_t binding, uint32_t index, uint32_t descriptorCount, const VkDescriptorImageInfo* imageInfo, const VkDescriptorBufferInfo* bufferInfo, const VkBufferView* texelBufferView)
{
	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = m_descriptorSet;
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstArrayElement = index;
	descriptorWrite.descriptorType = m_bindingTypes[binding];
	descriptorWrite.descriptorCount = descriptorCount;
	descriptorWrite.pImageInfo = imageInfo;
	descriptorWrite.pBufferInfo = bufferInfo;
	descriptorWrite.pTexelBufferView = texelBufferView;

	m_device.vkUpdateDescriptorSets(m_device, 1, &descriptorWrite, 0, nullptr);
}
