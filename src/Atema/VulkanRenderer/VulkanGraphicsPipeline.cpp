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
#include <Atema/VulkanRenderer/VulkanGraphicsPipeline.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>
#include <Atema/VulkanRenderer/VulkanRenderPass.hpp>
#include <Atema/VulkanRenderer/VulkanShader.hpp>

using namespace at;

VulkanGraphicsPipeline::VulkanGraphicsPipeline(const GraphicsPipeline::Settings& settings) :
	GraphicsPipeline(),
	m_pipelineLayout(VK_NULL_HANDLE),
	m_pipeline(VK_NULL_HANDLE)
{
	auto& renderer = VulkanRenderer::instance();
	m_device = renderer.getLogicalDeviceHandle();

	//-----
	// Shaders
	// Assign shader modules to a specific pipeline stage
	
	VkPipelineShaderStageCreateInfo shaderStages[2];
	{
		// Vertex shader
		auto vertShader = std::static_pointer_cast<VulkanShader>(settings.vertexShader);

		if (!vertShader)
		{
			ATEMA_ERROR("Invalid vertex shader");
		}
		
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShader->getHandle();
		vertShaderStageInfo.pName = "main";
		//vertShaderStageInfo.pSpecializationInfo = nullptr; // Used to specify constants and optimize stuff

		shaderStages[0] = vertShaderStageInfo;

		// Fragment shader
		auto fragShader = std::static_pointer_cast<VulkanShader>(settings.fragmentShader);

		if (!vertShader)
		{
			ATEMA_ERROR("Invalid fragment shader");
		}
		
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShader->getHandle();
		fragShaderStageInfo.pName = "main";
		//fragShaderStageInfo.pSpecializationInfo = nullptr; // Used to specify constants and optimize stuff

		shaderStages[1] = fragShaderStageInfo;
	}

	//-----
	// Vertex input (format of vertex data passed to the shader)
	// Bindings : spacing between data & per-vertex/per-instance
	// Attributes : types, binding to load them from, at which offset
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	{
		auto& inputs = settings.vertexInput.inputs;

		ATEMA_ASSERT(!inputs.empty(), "Invalid vertex input");

		uint32_t offset = 0;
		for (auto& input : inputs)
		{
			VkVertexInputAttributeDescription description;
			description.binding = input.binding;
			description.location = input.location;
			description.format = Vulkan::getFormat(input.format);
			description.offset = offset;

			attributeDescriptions.push_back(description);

			offset += input.getByteSize();
		}
		
		//TODO: Manage instanced rendering
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = offset;
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			bindingDescriptions.push_back(bindingDescription);
		}

		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	}

	//-----
	// Input assembly (what kind of geometry)
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = Vulkan::getPrimitiveTopology(settings.inputAssembly.topology);
	inputAssembly.primitiveRestartEnable = settings.inputAssembly.primitiveRestart ? VK_TRUE : VK_FALSE;

	//-----
	// Viewport (region of the framebuffer to draw to)
	VkViewport viewport{};
	/*
	viewport.x = settings.viewport.position.x;
	viewport.y = settings.viewport.position.y;
	viewport.width = settings.viewport.size.x;
	viewport.height = settings.viewport.size.y;
	viewport.minDepth = settings.viewport.minDepth;
	viewport.maxDepth = settings.viewport.maxDepth;
	//*/

	//-----
	// Scissors (use rasterizer to discard pixels)
	VkRect2D scissor{};
	/*
	scissor.offset.x = settings.scissor.offset.x;
	scissor.offset.y = settings.scissor.offset.y;
	scissor.extent.width = settings.scissor.size.x;
	scissor.extent.height = settings.scissor.size.y;
	//*/

	//-----
	// Possible to use multiple viewports & scissors but requires to enable a GPU feature (device creation)
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	//-----
	// Rasterizer
	// Takes geometry from vertex shader and create fragments using fragment shaders
	// Applies depth testing, face culling, scissors test
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	{
		auto& rasterizerState = settings.rasterization;

		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		// We can clamp fragments outside near/far planes to those planes instead of discarding (requires GPU feature)
		rasterizer.depthClampEnable = rasterizerState.depthClamp ? VK_TRUE : VK_FALSE;
		// We can disable rasterization to prevent the geometry to pass to this stage (no output)
		rasterizer.rasterizerDiscardEnable = rasterizerState.rasterizerDiscard ? VK_TRUE : VK_FALSE;
		// Select the polygon mode : fill, point, line (point & line require GPU feature)
		rasterizer.polygonMode = Vulkan::getPolygonMode(rasterizerState.polygonMode);
		rasterizer.lineWidth = rasterizerState.lineWidth; // Thicker requires GPU feature
		rasterizer.cullMode = Vulkan::getCullMode(rasterizerState.cullMode); // None, front, back, both
		rasterizer.frontFace = Vulkan::getFrontFace(rasterizerState.frontFace); // VK_FRONT_FACE_CLOCKWISE
		rasterizer.depthBiasEnable = VK_FALSE;
		//rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		//rasterizer.depthBiasClamp = 0.0f; // Optional
		//rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
	}

	//-----
	// Multisampling (can be used to perform anti-aliasing but requires GPU feature)
	VkPipelineMultisampleStateCreateInfo multisampling{};
	{
		auto& multisampleSettings = settings.multisample;

		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.rasterizationSamples = Vulkan::getSamples(multisampleSettings.samples);
		if (multisampleSettings.sampleShading)
		{
			multisampling.sampleShadingEnable = VK_TRUE; // Enable sample shading in the pipeline
			multisampling.minSampleShading = multisampleSettings.minSampleShading; // Min fraction for sample shading (closer to 1 is smoother)
		}
		else
		{
			multisampling.sampleShadingEnable = VK_FALSE;
		}
		//multisampling.pSampleMask = nullptr; // Optional
		//multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		//multisampling.alphaToOneEnable = VK_FALSE; // Optional
	}
	
	//-----
	// Depth & stencil

	//-----
	// Color blending (configuration per attached framebuffer)
	uint32_t colorAttachmentCount = 0;

	for (auto& attachment : settings.renderPass->getAttachments())
	{
		if (!Renderer::isDepthImageFormat(attachment.format) && !Renderer::isStencilImageFormat(attachment.format))
			colorAttachmentCount++;
	}
	
	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.reserve(colorAttachmentCount);
	{
		auto& colorBlendSettings = settings.colorBlend;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = colorBlendSettings.enabled ? VK_TRUE : VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = Vulkan::getBlendFactor(colorBlendSettings.colorSrcFactor);
		colorBlendAttachment.dstColorBlendFactor = Vulkan::getBlendFactor(colorBlendSettings.colorDstFactor);
		colorBlendAttachment.colorBlendOp = Vulkan::getBlendOperation(colorBlendSettings.colorOperation);
		colorBlendAttachment.srcAlphaBlendFactor = Vulkan::getBlendFactor(colorBlendSettings.alphaSrcFactor);
		colorBlendAttachment.dstAlphaBlendFactor = Vulkan::getBlendFactor(colorBlendSettings.alphaDstFactor);
		colorBlendAttachment.alphaBlendOp = Vulkan::getBlendOperation(colorBlendSettings.alphaOperation);

		for (size_t i = 0; i < colorAttachmentCount; i++)
			colorBlendAttachments.push_back(colorBlendAttachment);
	}

	// Color blending (global configuration)
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE; // True to use bitwise combination blending, false to use previous method
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
	colorBlending.pAttachments = colorBlendAttachments.data();
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	// Enable depth testing (optionnal if we don't use any depth buffer)
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	{
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = settings.depth.test ? VK_TRUE : VK_FALSE;
		depthStencil.depthWriteEnable = settings.depth.write ? VK_TRUE : VK_FALSE;
		depthStencil.depthCompareOp = Vulkan::getCompareOperation(settings.depth.compareOperation);
		depthStencil.depthBoundsTestEnable = settings.depth.boundsTest ? VK_TRUE : VK_FALSE;
		depthStencil.minDepthBounds = settings.depth.boundsMin; // Optional
		depthStencil.maxDepthBounds = settings.depth.boundsMax; // Optional
		depthStencil.stencilTestEnable = settings.stencil ? VK_TRUE : VK_FALSE;
		depthStencil.front = {}; // Optional
		depthStencil.front.failOp = Vulkan::getStencilOperation(settings.stencilFront.failOperation);
		depthStencil.front.passOp = Vulkan::getStencilOperation(settings.stencilFront.passOperation);
		depthStencil.front.depthFailOp = Vulkan::getStencilOperation(settings.stencilFront.depthFailOperation);
		depthStencil.front.compareOp = Vulkan::getCompareOperation(settings.stencilFront.compareOperation);
		depthStencil.front.compareMask = settings.stencilFront.compareMask;
		depthStencil.front.writeMask = settings.stencilFront.writeMask;
		depthStencil.front.reference = settings.stencilFront.reference;
		depthStencil.back = {}; // Optional
		depthStencil.back.failOp = Vulkan::getStencilOperation(settings.stencilBack.failOperation);
		depthStencil.back.passOp = Vulkan::getStencilOperation(settings.stencilBack.passOperation);
		depthStencil.back.depthFailOp = Vulkan::getStencilOperation(settings.stencilBack.depthFailOperation);
		depthStencil.back.compareOp = Vulkan::getCompareOperation(settings.stencilBack.compareOperation);
		depthStencil.back.compareMask = settings.stencilBack.compareMask;
		depthStencil.back.writeMask = settings.stencilBack.writeMask;
		depthStencil.back.reference = settings.stencilBack.reference;
	}

	//-----
	// Dynamic states (instead of being fixed at pipeline creation)
	std::vector<VkDynamicState> dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	//-----
	// Pipeline layout (to use uniform variables & push constants)
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	{
		for (auto& layout : settings.descriptorSetLayouts)
		{
			auto descriptorSetLayout = std::static_pointer_cast<VulkanDescriptorSetLayout>(layout);
			
			descriptorSetLayouts.push_back(descriptorSetLayout->getHandle());
		}
		
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		//pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		ATEMA_VK_CHECK(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));
	}

	//-----
	// Pipeline creation
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = m_pipelineLayout;
	pipelineInfo.pDepthStencilState = &depthStencil; // Optional
	pipelineInfo.pDynamicState = &dynamicState; // Optional

	auto renderPass = std::static_pointer_cast<VulkanRenderPass>(settings.renderPass);
	
	pipelineInfo.renderPass = renderPass->getHandle(); // Can still be used with other render pass instances (if compatible)
	pipelineInfo.subpass = 0;

	// We can derivate pipelines, which can optimize creation & switch performance
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	//pipelineInfo.basePipelineIndex = -1; // Optional

	ATEMA_VK_CHECK(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline));
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
{
	ATEMA_VK_DESTROY(m_device, vkDestroyPipeline, m_pipeline);
	ATEMA_VK_DESTROY(m_device, vkDestroyPipelineLayout, m_pipelineLayout);
}

VkPipeline VulkanGraphicsPipeline::getHandle() const noexcept
{
	return m_pipeline;
}

VkPipelineLayout VulkanGraphicsPipeline::getLayoutHandle() const noexcept
{
	return m_pipelineLayout;
}
