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

#ifndef ATEMA_VULKANRENDERER_VULKANDESCRIPTORSET_HPP
#define ATEMA_VULKANRENDERER_VULKANDESCRIPTORSET_HPP

#include <Atema/VulkanRenderer/Config.hpp>
#include <Atema/VulkanRenderer/Vulkan.hpp>
#include <Atema/Renderer/DescriptorSet.hpp>
#include <Atema/Core/SparseSet.hpp>
#include <Atema/Core/Signal.hpp>

#include <functional>

namespace at
{
	class ATEMA_VULKANRENDERER_API VulkanDescriptorSet final : public DescriptorSet
	{
	public:
		VulkanDescriptorSet() = delete;
		VulkanDescriptorSet(const VulkanDevice& device, VkDescriptorSet descriptorSet, const SparseSet<VkDescriptorType>& bindingTypes);
		
		virtual ~VulkanDescriptorSet();

		VkDescriptorSet getHandle() const noexcept;

		void update(uint32_t binding, const Buffer& buffer, size_t offset, size_t size) override;
		void update(uint32_t binding, uint32_t index, const std::vector<const Buffer*>& buffers) override;
		void update(uint32_t binding, uint32_t index, const std::vector<const Buffer*>& buffers, const std::vector<size_t>& bufferOffsets, const std::vector<size_t>& bufferSizes) override;
		void update(uint32_t binding, const ImageView& imageView, const Sampler& sampler) override;
		void update(uint32_t binding, uint32_t index, const std::vector<const ImageView*>& imageViews, const std::vector<const Sampler*>& samplers) override;

		Signal<> onDestroy;

	private:
		void update(uint32_t binding, uint32_t index, uint32_t descriptorCount, const VkDescriptorImageInfo* imageInfo, const VkDescriptorBufferInfo* bufferInfo, const VkBufferView* texelBufferView);

		const VulkanDevice& m_device;
		VkDescriptorSet m_descriptorSet;
		const SparseSet<VkDescriptorType>& m_bindingTypes;
	};
}

#endif
