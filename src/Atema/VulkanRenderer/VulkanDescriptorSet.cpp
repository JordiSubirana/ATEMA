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

void VulkanDescriptorSet::update(
	const std::vector<uint32_t>& bufferBindings,
	const std::vector<uint32_t>& bufferIndices,
	const std::vector<std::vector<Ptr<Buffer>>>& buffers,
	const std::vector<std::vector<size_t>>& bufferRanges,
	const std::vector<uint32_t>& imageSamplerBindings,
	const std::vector<uint32_t>& imageSamplerIndices,
	const std::vector<std::vector<Ptr<ImageView>>>& imageViews,
	const std::vector<std::vector<Ptr<Sampler>>>& samplers)
{
	ATEMA_ASSERT(bufferBindings.size() == bufferIndices.size(), "Inconsistent buffer sizes");
	ATEMA_ASSERT(bufferBindings.size() == buffers.size(), "Inconsistent buffer sizes");
	ATEMA_ASSERT(bufferBindings.size() == bufferRanges.size(), "Inconsistent buffer sizes");
	ATEMA_ASSERT(imageSamplerBindings.size() == imageSamplerIndices.size(), "Inconsistent image sampler sizes");
	ATEMA_ASSERT(imageSamplerBindings.size() == imageViews.size(), "Inconsistent image sampler sizes");
	ATEMA_ASSERT(imageSamplerBindings.size() == samplers.size(), "Inconsistent image sampler sizes");

	std::vector<std::vector<VkDescriptorBufferInfo>> descriptorBuffers;
	std::vector<std::vector<VkDescriptorImageInfo>> descriptorImages;
	std::vector<VkWriteDescriptorSet> descriptorWrites;
	
	for (size_t i = 0; i < bufferBindings.size(); i++)
	{
		descriptorBuffers.resize(descriptorBuffers.size() + 1);

		auto& descriptors = descriptorBuffers.back();
		descriptors.reserve(buffers[i].size());

		const auto& ranges = bufferRanges[i];
		
		if (ranges.empty())
		{
			for (auto& buffer : buffers[i])
			{
				VkDescriptorBufferInfo descriptor{};
				descriptor.buffer = std::static_pointer_cast<VulkanBuffer>(buffer)->getHandle();
				descriptor.offset = 0;
				descriptor.range = VK_WHOLE_SIZE;

				descriptors.push_back(descriptor);
			}
		}
		else
		{
			ATEMA_ASSERT(buffers[i].size() == ranges.size(), "Each buffer must have a range");
			
			for (size_t j = 0; j < buffers[i].size(); j++)
			{
				auto& buffer = buffers[i][j];
				const auto range = ranges[i];

				VkDescriptorBufferInfo descriptor{};
				descriptor.buffer = std::static_pointer_cast<VulkanBuffer>(buffer)->getHandle();
				descriptor.offset = 0;
				descriptor.range = range ? static_cast<VkDeviceSize>(range) : VK_WHOLE_SIZE;

				descriptors.push_back(descriptor);
			}
		}

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_descriptorSet;
		descriptorWrite.dstBinding = bufferBindings[i]; // Binding in the shader
		descriptorWrite.dstArrayElement = 0; // First index we want to update (descriptors can be arrays)
		descriptorWrite.descriptorType = m_bindingTypes[bufferBindings[i]]; // Type of descriptor
		descriptorWrite.descriptorCount = static_cast<uint32_t>(descriptors.size()); // How many elements we want to update
		descriptorWrite.pBufferInfo = descriptors.data(); // Used for descriptors that refer to buffer data
		descriptorWrite.pImageInfo = nullptr; // Used for descriptors that refer to image data
		descriptorWrite.pTexelBufferView = nullptr; // Used for descriptors that refer to buffer views

		descriptorWrites.push_back(descriptorWrite);
	}

	for (size_t i = 0; i < imageSamplerBindings.size(); i++)
	{
		ATEMA_ASSERT(imageViews[i].size() == samplers[i].size(), "Each image must be associated with a sampler");

		descriptorImages.resize(descriptorImages.size() + 1);

		auto& descriptors = descriptorImages.back();
		descriptors.resize(imageViews[i].size());

		for (size_t j = 0; j < descriptors.size(); j++)
		{
			const auto imageView = std::static_pointer_cast<VulkanImageView>(imageViews[i][j]);
			const auto sampler = std::static_pointer_cast<VulkanSampler>(samplers[i][j]);

			descriptors[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			descriptors[j].imageView = imageView->getHandle();
			descriptors[j].sampler = sampler->getHandle();
		}

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_descriptorSet;
		descriptorWrite.dstBinding = imageSamplerBindings[i]; // Binding in the shader
		descriptorWrite.dstArrayElement = 0; // First index we want to update (descriptors can be arrays)
		descriptorWrite.descriptorType = m_bindingTypes[imageSamplerBindings[i]]; // Type of descriptor
		descriptorWrite.descriptorCount = static_cast<uint32_t>(descriptors.size()); // How many elements we want to update
		descriptorWrite.pBufferInfo = nullptr; // Used for descriptors that refer to buffer data
		descriptorWrite.pImageInfo = descriptors.data(); // Used for descriptors that refer to image data
		descriptorWrite.pTexelBufferView = nullptr; // Used for descriptors that refer to buffer views

		descriptorWrites.push_back(descriptorWrite);
	}

	m_device.vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}
