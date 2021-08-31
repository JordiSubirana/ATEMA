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

#include <Atema/VulkanRenderer/VulkanRenderPass.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>

using namespace at;

VulkanRenderPass::VulkanRenderPass(const RenderPass::Settings& settings) :
	RenderPass(),
	m_renderPass(VK_NULL_HANDLE),
	m_attachments(settings.attachments)
{
	auto& renderer = VulkanRenderer::instance();
	m_device = renderer.getLogicalDeviceHandle();

	//TODO: Manage resolve attachments
	std::vector<VkAttachmentDescription> attachmentDescriptions;
	std::vector<VkAttachmentReference> colorReferences;
	std::vector<VkAttachmentReference> depthReferences;

	int index = 0;
	for (auto& attachment : settings.attachments)
	{
		bool depth = hasDepth(attachment.format);

		// Attachment Description
		VkAttachmentDescription attachmentDescription{};
		attachmentDescription.format = Vulkan::getFormat(attachment.format);
		attachmentDescription.samples = Vulkan::getSamples(attachment.samples);
		attachmentDescription.loadOp = Vulkan::getAttachmentLoading(attachment.loading);
		attachmentDescription.storeOp = Vulkan::getAttachmentStoring(attachment.storing);
		if (hasStencil(attachment.format))
		{
			attachmentDescription.stencilLoadOp = attachmentDescription.loadOp;
			attachmentDescription.stencilStoreOp = attachmentDescription.storeOp;
		}
		else
		{
			attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}
		attachmentDescription.initialLayout = Vulkan::getLayout(attachment.initialLayout, depth);
		attachmentDescription.finalLayout = Vulkan::getLayout(attachment.finalLayout, depth);

		attachmentDescriptions.push_back(attachmentDescription);

		// Attachment Reference
		VkAttachmentReference attachmentReference;
		attachmentReference.attachment = index++;
		if (depth)
		{
			attachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthReferences.push_back(attachmentReference);
		}
		else
		{
			attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorReferences.push_back(attachmentReference);
		}
	}

	// Create subpasses
	//TODO: Manage multiple subpasses
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
	subpass.pColorAttachments = colorReferences.data(); // Index 0 = layout (location = 0) out vec4 in fragment shader
	subpass.pDepthStencilAttachment = depthReferences.data();
	subpass.pResolveAttachments = nullptr; // For multisampling

	// Subpass dependencies
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // Refers to implicit subpass before the render pass (or after if defined in dstSubpass)
	dependency.dstSubpass = 0; // Refers to the only subpass we have right now
	// Specify the operations to wait on and the stages in which these operations occur
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	// The operations that should wait on this are in the color attachment stage and involve the writing of the color attachment
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	// Create render pass
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
	renderPassInfo.pAttachments = attachmentDescriptions.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	ATEMA_VK_CHECK(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass));
}

VulkanRenderPass::~VulkanRenderPass()
{
	ATEMA_VK_DESTROY(m_device, vkDestroyRenderPass, m_renderPass);
}

VkRenderPass VulkanRenderPass::getHandle() const noexcept
{
	return m_renderPass;
}

const std::vector<AttachmentDescription>& VulkanRenderPass::getAttachments() const noexcept
{
	return m_attachments;
}
