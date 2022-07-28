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

#ifndef ATEMA_VULKANRENDERER_VULKANDESCRIPTORPOOL_HPP
#define ATEMA_VULKANRENDERER_VULKANDESCRIPTORPOOL_HPP

#include <Atema/VulkanRenderer/Config.hpp>
#include <Atema/VulkanRenderer/Vulkan.hpp>
#include <Atema/Core/SparseSet.hpp>

#include <queue>

namespace at
{
	class DescriptorSet;
	class VulkanDescriptorSetLayout;

	class ATEMA_VULKANRENDERER_API VulkanDescriptorPool final
	{
	public:
		VulkanDescriptorPool() = delete;
		VulkanDescriptorPool(const VulkanDevice& device, const VulkanDescriptorSetLayout& descriptorSetLayout, uint32_t pageSize);
		virtual ~VulkanDescriptorPool();

		Ptr<DescriptorSet> createSet();

		VkDescriptorPool getHandle() const;

	private:
		class Pool
		{
		public:
			Pool() = delete;
			Pool(const VulkanDevice& device, VkDescriptorSetLayout layout, const SparseSet<VkDescriptorType>& bindingTypes, const VkDescriptorPoolCreateInfo& settings);
			~Pool();

			bool isFull() const noexcept;

			Ptr<DescriptorSet> createSet();

			VkDescriptorPool getHandle() const;

		private:
			const VulkanDevice& m_device;
			VkDescriptorPool m_pool;
			VkDescriptorSetLayout m_layout;
			const SparseSet<VkDescriptorType>& m_bindingTypes;
			std::queue<VkDescriptorSet> m_unusedSets;
			uint32_t m_size;
			uint32_t m_maxSize;
		};
		
		void addPool();
		
		const VulkanDevice& m_device;
		std::vector<VkDescriptorPoolSize> m_poolSizes;
		VkDescriptorPoolCreateInfo m_poolSettings;
		std::vector<Ptr<Pool>> m_pools;
		VkDescriptorSetLayout m_layout;
		SparseSet<VkDescriptorType> m_bindingTypes;
	};
}

#endif
