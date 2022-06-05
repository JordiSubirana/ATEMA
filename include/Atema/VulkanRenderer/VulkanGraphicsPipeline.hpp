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

#ifndef ATEMA_VULKANRENDERER_VULKANGRAPHICSPIPELINE_HPP
#define ATEMA_VULKANRENDERER_VULKANGRAPHICSPIPELINE_HPP

#include <shared_mutex>
#include <Atema/VulkanRenderer/Config.hpp>
#include <Atema/Renderer/GraphicsPipeline.hpp>
#include <Atema/VulkanRenderer/Vulkan.hpp>
#include <Atema/Core/Signal.hpp>

namespace at
{
	class VulkanRenderPass;

	class ATEMA_VULKANRENDERER_API VulkanGraphicsPipeline final : public GraphicsPipeline
	{
	public:
		VulkanGraphicsPipeline() = delete;
		VulkanGraphicsPipeline(const VulkanDevice& device, const GraphicsPipeline::Settings& settings);
		virtual ~VulkanGraphicsPipeline();

		VkPipeline getHandle(VulkanRenderPass& renderPass, uint32_t subpassIndex);

		VkPipelineLayout getLayoutHandle() const noexcept;
		
	private:
		VkPipeline readPipeline(VulkanRenderPass& renderPass, uint32_t subpassIndex);
		VkPipeline createPipeline(VulkanRenderPass& renderPass, uint32_t subpassIndex);

		struct RenderPassData
		{
			ConnectionGuard connectionGuard;
			std::unordered_map<uint32_t, VkPipeline> pipelines;
		};

		const VulkanDevice& m_device;

		std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;

		VkPipelineVertexInputStateCreateInfo m_vertexInputInfo;
		std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;
		std::vector<VkVertexInputBindingDescription> m_bindingDescriptions;

		VkPipelineInputAssemblyStateCreateInfo m_inputAssembly;

		VkPipelineViewportStateCreateInfo m_viewportState;

		VkPipelineRasterizationStateCreateInfo m_rasterizer;

		VkPipelineMultisampleStateCreateInfo m_multisampling;

		VkPipelineColorBlendAttachmentState m_colorBlendAttachment;

		std::vector<VkPipelineColorBlendAttachmentState> m_colorBlendAttachments;

		VkPipelineColorBlendStateCreateInfo m_colorBlending;

		VkPipelineDepthStencilStateCreateInfo m_depthStencil;

		std::vector<VkDynamicState> m_dynamicStates;

		VkPipelineDynamicStateCreateInfo m_dynamicState;

		VkPipelineLayout m_pipelineLayout;

		std::unordered_map<VkRenderPass, Ptr<RenderPassData>> m_renderPassDatas;
		std::shared_mutex m_pipelineMutex;
	};
}

#endif
