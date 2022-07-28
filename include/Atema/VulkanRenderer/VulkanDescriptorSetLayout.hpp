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

#ifndef ATEMA_VULKANRENDERER_VULKANDESCRIPTORSETLAYOUT_HPP
#define ATEMA_VULKANRENDERER_VULKANDESCRIPTORSETLAYOUT_HPP

#include <Atema/VulkanRenderer/Config.hpp>
#include <Atema/VulkanRenderer/Vulkan.hpp>
#include <Atema/Renderer/DescriptorSetLayout.hpp>

namespace at
{
	class VulkanDevice;
	class VulkanDescriptorPool;

	class ATEMA_VULKANRENDERER_API VulkanDescriptorSetLayout final : public DescriptorSetLayout
	{
	public:
		VulkanDescriptorSetLayout() = delete;
		VulkanDescriptorSetLayout(const VulkanDevice& device, const DescriptorSetLayout::Settings& settings);
		virtual ~VulkanDescriptorSetLayout();

		VkDescriptorSetLayout getHandle() const noexcept;

		const std::vector<DescriptorSetBinding>& getBindings() const noexcept override;

		Ptr<DescriptorSet> createSet() override;

		const VulkanDescriptorPool& getDescriptorPool() const;

	private:
		const VulkanDevice& m_device;
		VkDescriptorSetLayout m_descriptorSetLayout;

		std::vector<DescriptorSetBinding> m_bindings;

		UPtr<VulkanDescriptorPool> m_descriptorPool;
	};
}

#endif
