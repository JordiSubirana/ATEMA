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
#include <Atema/VulkanRenderer/VulkanDescriptorSetLayout.hpp>
#include <Atema/VulkanRenderer/VulkanGraphicsPipeline.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>
#include <Atema/VulkanRenderer/VulkanRenderPass.hpp>
#include <Atema/VulkanRenderer/VulkanShader.hpp>

using namespace at;

VulkanGraphicsPipeline::VulkanGraphicsPipeline(const VulkanDevice& device, const GraphicsPipeline::Settings& settings) :
	GraphicsPipeline(),
	m_device(device),
	m_vertexShader(settings.vertexShader),
	m_fragmentShader(settings.fragmentShader),
	m_pipelineLayout(VK_NULL_HANDLE)
{
	const auto& state = settings.state;

	//-----
	// Shaders
	// Assign shader modules to a specific pipeline stage
	m_shaderStages.resize(2);
	{
		// Vertex shader
		auto vertShader = std::static_pointer_cast<VulkanShader>(m_vertexShader);

		if (!vertShader)
		{
			ATEMA_ERROR("Invalid vertex shader");
		}
		
		auto& vertShaderStageInfo = m_shaderStages[0];
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.flags = 0;
		vertShaderStageInfo.module = vertShader->getHandle();
		vertShaderStageInfo.pName = "main";
		vertShaderStageInfo.pSpecializationInfo = nullptr; // Used to specify constants and optimize stuff
		vertShaderStageInfo.pNext = nullptr;

		// Fragment shader
		auto fragShader = std::static_pointer_cast<VulkanShader>(m_fragmentShader);

		if (!fragShader)
		{
			ATEMA_ERROR("Invalid fragment shader");
		}
		
		auto& fragShaderStageInfo = m_shaderStages[1];
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.flags = 0;
		fragShaderStageInfo.module = fragShader->getHandle();
		fragShaderStageInfo.pName = "main";
		fragShaderStageInfo.pSpecializationInfo = nullptr; // Used to specify constants and optimize stuff
		fragShaderStageInfo.pNext = nullptr;
	}

	//-----
	// Vertex input (format of vertex data passed to the shader)
	// Bindings : spacing between data & per-vertex/per-instance
	// Attributes : types, binding to load them from, at which offset
	{
		auto& inputs = state.vertexInput.inputs;

		ATEMA_ASSERT(!inputs.empty(), "Invalid vertex input");

		uint32_t offset = 0;
		for (auto& input : inputs)
		{
			VkVertexInputAttributeDescription description;
			description.binding = input.binding;
			description.location = input.location;
			description.format = Vulkan::getFormat(input.format);
			description.offset = offset;

			m_attributeDescriptions.push_back(description);

			offset += static_cast<uint32_t>(input.getByteSize());
		}
		
		//TODO: Manage instanced rendering
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = offset;
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			m_bindingDescriptions.push_back(bindingDescription);
		}

		m_vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		m_vertexInputInfo.vertexBindingDescriptionCount = 1;
		m_vertexInputInfo.pVertexBindingDescriptions = m_bindingDescriptions.data();
		m_vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_attributeDescriptions.size());
		m_vertexInputInfo.pVertexAttributeDescriptions = m_attributeDescriptions.data();
		m_vertexInputInfo.flags = 0;
		m_vertexInputInfo.pNext = nullptr;
	}

	//-----
	// Input assembly (what kind of geometry)
	m_inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	m_inputAssembly.topology = Vulkan::getPrimitiveTopology(state.inputAssembly.topology);
	m_inputAssembly.primitiveRestartEnable = state.inputAssembly.primitiveRestart ? VK_TRUE : VK_FALSE;
	m_inputAssembly.flags = 0;
	m_inputAssembly.pNext = nullptr;

	//-----
	// Possible to use multiple viewports & scissors but requires to enable a GPU feature (device creation)
	m_viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	m_viewportState.viewportCount = 1;
	m_viewportState.pViewports = nullptr;
	m_viewportState.scissorCount = 1;
	m_viewportState.pScissors = nullptr;
	m_viewportState.flags = 0;
	m_viewportState.pNext = nullptr;

	//-----
	// Rasterizer
	// Takes geometry from vertex shader and create fragments using fragment shaders
	// Applies depth testing, face culling, scissors test
	{
		auto& rasterizerState = state.rasterization;

		m_rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		// We can clamp fragments outside near/far planes to those planes instead of discarding (requires GPU feature)
		m_rasterizer.depthClampEnable = rasterizerState.depthClamp ? VK_TRUE : VK_FALSE;
		// We can disable rasterization to prevent the geometry to pass to this stage (no output)
		m_rasterizer.rasterizerDiscardEnable = rasterizerState.rasterizerDiscard ? VK_TRUE : VK_FALSE;
		// Select the polygon mode : fill, point, line (point & line require GPU feature)
		m_rasterizer.polygonMode = Vulkan::getPolygonMode(rasterizerState.polygonMode);
		m_rasterizer.lineWidth = rasterizerState.lineWidth; // Thicker requires GPU feature
		m_rasterizer.cullMode = Vulkan::getCullMode(rasterizerState.cullMode); // None, front, back, both
		m_rasterizer.frontFace = Vulkan::getFrontFace(rasterizerState.frontFace); // VK_FRONT_FACE_CLOCKWISE
		m_rasterizer.depthBiasEnable = VK_FALSE;
		m_rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		m_rasterizer.depthBiasClamp = 0.0f; // Optional
		m_rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
		m_rasterizer.flags = 0;
		m_rasterizer.pNext = nullptr;
	}

	//-----
	// Multisampling (can be used to perform anti-aliasing but requires GPU feature)
	{
		auto& multisampleSettings = state.multisample;

		m_multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		m_multisampling.rasterizationSamples = Vulkan::getSamples(multisampleSettings.samples);
		if (multisampleSettings.sampleShading)
		{
			m_multisampling.sampleShadingEnable = VK_TRUE; // Enable sample shading in the pipeline
			m_multisampling.minSampleShading = multisampleSettings.minSampleShading; // Min fraction for sample shading (closer to 1 is smoother)
		}
		else
		{
			m_multisampling.sampleShadingEnable = VK_FALSE;
		}
		m_multisampling.pSampleMask = nullptr; // Optional
		m_multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		m_multisampling.alphaToOneEnable = VK_FALSE; // Optional
		m_multisampling.flags = 0;
		m_multisampling.pNext = nullptr;
	}
	
	//-----
	// Depth & stencil

	//-----
	// Color blending (configuration per attached framebuffer)
	m_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	m_colorBlendAttachment.blendEnable = state.colorBlend.enabled ? VK_TRUE : VK_FALSE;
	m_colorBlendAttachment.srcColorBlendFactor = Vulkan::getBlendFactor(state.colorBlend.colorSrcFactor);
	m_colorBlendAttachment.dstColorBlendFactor = Vulkan::getBlendFactor(state.colorBlend.colorDstFactor);
	m_colorBlendAttachment.colorBlendOp = Vulkan::getBlendOperation(state.colorBlend.colorOperation);
	m_colorBlendAttachment.srcAlphaBlendFactor = Vulkan::getBlendFactor(state.colorBlend.alphaSrcFactor);
	m_colorBlendAttachment.dstAlphaBlendFactor = Vulkan::getBlendFactor(state.colorBlend.alphaDstFactor);
	m_colorBlendAttachment.alphaBlendOp = Vulkan::getBlendOperation(state.colorBlend.alphaOperation);
	

	// Color blending (global configuration)
	m_colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	m_colorBlending.logicOpEnable = VK_FALSE; // True to use bitwise combination blending, false to use previous method
	m_colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	m_colorBlending.blendConstants[0] = 0.0f; // Optional
	m_colorBlending.blendConstants[1] = 0.0f; // Optional
	m_colorBlending.blendConstants[2] = 0.0f; // Optional
	m_colorBlending.blendConstants[3] = 0.0f; // Optional
	// Fill attachments during pipeline creation (depends on render pass)
	m_colorBlending.attachmentCount = 0;
	m_colorBlending.pAttachments = nullptr;
	m_colorBlending.flags = 0;
	m_colorBlending.pNext = nullptr;

	// Enable depth testing (optionnal if we don't use any depth buffer)
	{
		m_depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		m_depthStencil.depthTestEnable = state.depth.test ? VK_TRUE : VK_FALSE;
		m_depthStencil.depthWriteEnable = state.depth.write ? VK_TRUE : VK_FALSE;
		m_depthStencil.depthCompareOp = Vulkan::getCompareOperation(state.depth.compareOperation);
		m_depthStencil.depthBoundsTestEnable = state.depth.boundsTest ? VK_TRUE : VK_FALSE;
		m_depthStencil.minDepthBounds = state.depth.boundsMin; // Optional
		m_depthStencil.maxDepthBounds = state.depth.boundsMax; // Optional
		m_depthStencil.stencilTestEnable = state.stencil ? VK_TRUE : VK_FALSE;
		m_depthStencil.front = {}; // Optional
		m_depthStencil.front.failOp = Vulkan::getStencilOperation(state.stencilFront.failOperation);
		m_depthStencil.front.passOp = Vulkan::getStencilOperation(state.stencilFront.passOperation);
		m_depthStencil.front.depthFailOp = Vulkan::getStencilOperation(state.stencilFront.depthFailOperation);
		m_depthStencil.front.compareOp = Vulkan::getCompareOperation(state.stencilFront.compareOperation);
		m_depthStencil.front.compareMask = state.stencilFront.compareMask;
		m_depthStencil.front.writeMask = state.stencilFront.writeMask;
		m_depthStencil.front.reference = state.stencilFront.reference;
		m_depthStencil.back = {}; // Optional
		m_depthStencil.back.failOp = Vulkan::getStencilOperation(state.stencilBack.failOperation);
		m_depthStencil.back.passOp = Vulkan::getStencilOperation(state.stencilBack.passOperation);
		m_depthStencil.back.depthFailOp = Vulkan::getStencilOperation(state.stencilBack.depthFailOperation);
		m_depthStencil.back.compareOp = Vulkan::getCompareOperation(state.stencilBack.compareOperation);
		m_depthStencil.back.compareMask = state.stencilBack.compareMask;
		m_depthStencil.back.writeMask = state.stencilBack.writeMask;
		m_depthStencil.back.reference = state.stencilBack.reference;
		m_depthStencil.flags = 0;
		m_depthStencil.pNext = nullptr;
	}

	//-----
	// Dynamic states (instead of being fixed at pipeline creation)
	m_dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	m_dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	m_dynamicState.dynamicStateCount = static_cast<uint32_t>(m_dynamicStates.size());
	m_dynamicState.pDynamicStates = m_dynamicStates.data();
	m_dynamicState.flags = 0;
	m_dynamicState.pNext = nullptr;

	//-----
	// Pipeline layout (to use uniform variables & push constants)
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	{
		for (auto& layout : settings.descriptorSetLayouts)
		{
			VkDescriptorSetLayout descriptorSetLayoutHandle = VK_NULL_HANDLE;

			auto descriptorSetLayout = std::static_pointer_cast<VulkanDescriptorSetLayout>(layout);
			if (descriptorSetLayout)
				descriptorSetLayoutHandle = descriptorSetLayout->getHandle();
			
			descriptorSetLayouts.emplace_back(descriptorSetLayoutHandle);
		}
		
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		//pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
		pipelineLayoutInfo.flags = 0;
		pipelineLayoutInfo.pNext = nullptr;

		ATEMA_VK_CHECK(m_device.vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));
	}
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
{
	for (auto& renderPassData : m_renderPassDatas)
	{
		for (auto& subpassPipelines : renderPassData.second->pipelines)
			ATEMA_VK_DESTROY(m_device, vkDestroyPipeline, subpassPipelines.second);
	}

	m_renderPassDatas.clear();

	ATEMA_VK_DESTROY(m_device, vkDestroyPipelineLayout, m_pipelineLayout);
}

VkPipeline VulkanGraphicsPipeline::getHandle(const VulkanRenderPass& renderPass, uint32_t subpassIndex) const
{
	const auto pipeline = readPipeline(renderPass, subpassIndex);

	if (pipeline != VK_NULL_HANDLE)
		return pipeline;

	return createPipeline(renderPass, subpassIndex);
}

VkPipelineLayout VulkanGraphicsPipeline::getLayoutHandle() const noexcept
{
	return m_pipelineLayout;
}

VkPipeline VulkanGraphicsPipeline::readPipeline(const VulkanRenderPass& renderPass, uint32_t subpassIndex) const
{
	std::shared_lock readLock(m_pipelineMutex);

	// Return the pipeline if it exists
	const auto renderPassDataIt = m_renderPassDatas.find(renderPass.getHandle());
	if (renderPassDataIt != m_renderPassDatas.end())
	{
		const auto& pipelines = renderPassDataIt->second->pipelines;
		const auto subpassPipelineIt = pipelines.find(subpassIndex);

		if (subpassPipelineIt != pipelines.end())
			return subpassPipelineIt->second;
	}

	// Needs to be created!
	return VK_NULL_HANDLE;
}

VkPipeline VulkanGraphicsPipeline::createPipeline(const VulkanRenderPass& renderPass, uint32_t subpassIndex) const
{
	std::unique_lock writeLock(m_pipelineMutex);

	const auto renderPassHandle = renderPass.getHandle();

	// Return the pipeline if it exists
	const auto renderPassDataIt = m_renderPassDatas.find(renderPass.getHandle());
	if (renderPassDataIt != m_renderPassDatas.end())
	{
		const auto& pipelines = renderPassDataIt->second->pipelines;
		const auto subpassPipelineIt = pipelines.find(subpassIndex);

		if (subpassPipelineIt != pipelines.end())
			return subpassPipelineIt->second;
	}
	// We will create an entry for this RenderPass for the first time
	// On RenderPass's destruction, we'll want to free all created pipelines
	else
	{
		auto data = std::make_shared<RenderPassData>();
		data->connectionGuard.connect(const_cast<Signal<>&>(renderPass.onDestroy), [this, renderPassHandle]()
			{
				std::unique_lock writeLock(m_pipelineMutex);

				auto& data = m_renderPassDatas[renderPassHandle];

				if (data)
				{
					for (auto& subpassPipelines : data->pipelines)
						ATEMA_VK_DESTROY(m_device, vkDestroyPipeline, subpassPipelines.second);
				}

				m_renderPassDatas.erase(renderPassHandle);
			});

		m_renderPassDatas.emplace(renderPassHandle, std::move(data));
	}

	// Create the pipeline if it does not exist
	auto& pipeline = m_renderPassDatas[renderPassHandle]->pipelines[subpassIndex];
	{
		// Color blending (configuration per attached framebuffer)
		const auto colorAttachmentCount = renderPass.getColorAttachmentCount(subpassIndex);

		if (m_colorBlendAttachments.size() < colorAttachmentCount)
		{
			const auto previousSize = m_colorBlendAttachments.size();

			m_colorBlendAttachments.resize(colorAttachmentCount);

			for (size_t i = previousSize; i < m_colorBlendAttachments.size(); i++)
				m_colorBlendAttachments[i] = m_colorBlendAttachment;
		}

		m_colorBlending.attachmentCount = static_cast<uint32_t>(m_colorBlendAttachments.size());
		m_colorBlending.pAttachments = m_colorBlendAttachments.data();

		// Pipeline creation
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = m_shaderStages.data();

		pipelineInfo.pVertexInputState = &m_vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &m_inputAssembly;
		pipelineInfo.pViewportState = &m_viewportState;
		pipelineInfo.pRasterizationState = &m_rasterizer;
		pipelineInfo.pMultisampleState = &m_multisampling;
		pipelineInfo.pColorBlendState = &m_colorBlending;
		pipelineInfo.layout = m_pipelineLayout;
		pipelineInfo.pDepthStencilState = &m_depthStencil; // Optional
		pipelineInfo.pDynamicState = &m_dynamicState; // Optional

		pipelineInfo.renderPass = renderPassHandle; // Can still be used with other render pass instances (if compatible)
		pipelineInfo.subpass = subpassIndex;

		// We can derivate pipelines, which can optimize creation & switch performance
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		//pipelineInfo.basePipelineIndex = -1; // Optional

		ATEMA_VK_CHECK(m_device.vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));
	}

	return pipeline;
}
