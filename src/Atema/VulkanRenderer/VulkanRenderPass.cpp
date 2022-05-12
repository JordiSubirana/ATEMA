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

// Special thanks to Granite articles & code, that made me understand a lot of things about subpasses behaviour and why we should use them
// https://github.com/Themaister/Granite

#include <Atema/VulkanRenderer/VulkanRenderPass.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>

using namespace at;

namespace
{
	struct AttachmentData
	{
		AttachmentData() : description({}) {}
		
		AttachmentData(const AttachmentDescription& settings) : description({})
		{
			hasDepth = Renderer::isDepthImageFormat(settings.format);

			// Attachment Description
			description.format = Vulkan::getFormat(settings.format);
			description.samples = Vulkan::getSamples(settings.samples);
			description.loadOp = Vulkan::getAttachmentLoading(settings.loading);
			description.storeOp = Vulkan::getAttachmentStoring(settings.storing);
			if (Renderer::isStencilImageFormat(settings.format))
			{
				description.stencilLoadOp = description.loadOp;
				description.stencilStoreOp = description.storeOp;
			}
			else
			{
				description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			}
			description.initialLayout = Vulkan::getLayout(settings.initialLayout, hasDepth);
			description.finalLayout = Vulkan::getLayout(settings.finalLayout, hasDepth);
		}
		
		VkAttachmentDescription description;
		bool hasDepth = false;
	};

	enum class AttachmentSubpassRole
	{
		Input = 0x0001,
		Color = 0x0002,
		Resolve = 0x0004,
		Depth = 0x0008
	};

	ATEMA_DECLARE_FLAGS(AttachmentSubpassRole);

	VkImageLayout checkAttachmentLayout(Flags<AttachmentSubpassRole> roles, bool isDepth)
	{
		if (roles & AttachmentSubpassRole::Color)
		{
			// If the image is also used as an input, we'll use a general layout
			if (roles & AttachmentSubpassRole::Input)
			{
				return VK_IMAGE_LAYOUT_GENERAL;
			}

			if (roles & AttachmentSubpassRole::Depth)
			{
				ATEMA_ERROR("Invalid subpass definition : an attachment can't be used as an output and a depth/stencil");

				return VK_IMAGE_LAYOUT_UNDEFINED;
			}

			if (roles & AttachmentSubpassRole::Resolve)
			{
				ATEMA_ERROR("Invalid subpass definition : an attachment can't be used as an output and a resolve attachment");

				return VK_IMAGE_LAYOUT_UNDEFINED;
			}

			if (isDepth)
			{
				ATEMA_ERROR("Invalid subpass definition : an output attachment must not have a depth/stencil format");

				return VK_IMAGE_LAYOUT_UNDEFINED;
			}

			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		else if (roles & AttachmentSubpassRole::Depth)
		{
			// If the image is also used as an input, we'll use a general layout
			if (roles & AttachmentSubpassRole::Input)
			{
				return VK_IMAGE_LAYOUT_GENERAL;
			}

			if (roles & AttachmentSubpassRole::Resolve)
			{
				ATEMA_ERROR("Invalid subpass definition : an attachment can't be used as a depth attachment and a resolve attachment");

				return VK_IMAGE_LAYOUT_UNDEFINED;
			}

			if (!isDepth)
			{
				ATEMA_ERROR("Invalid subpass definition : a depth attachment must not have a color format");

				return VK_IMAGE_LAYOUT_UNDEFINED;
			}

			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else if (roles & AttachmentSubpassRole::Resolve)
		{
			// If the image is also used as an input, we'll use a general layout
			if (roles & AttachmentSubpassRole::Input)
			{
				return VK_IMAGE_LAYOUT_GENERAL;
			}

			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		else if (roles & AttachmentSubpassRole::Input)
		{
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		return VK_IMAGE_LAYOUT_GENERAL;
	}

	bool isLayoutCompatible(VkImageLayout layout, Flags<AttachmentSubpassRole> roles)
	{
		if (roles & AttachmentSubpassRole::Color)
		{
			if (roles & AttachmentSubpassRole::Input)
				return layout == VK_IMAGE_LAYOUT_GENERAL;

			// No layout can match this combo of roles
			if (roles & AttachmentSubpassRole::Depth || roles & AttachmentSubpassRole::Resolve)
				return false;

			return layout == VK_IMAGE_LAYOUT_GENERAL || layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		else if (roles & AttachmentSubpassRole::Depth)
		{
			if (roles & AttachmentSubpassRole::Input)
				return layout == VK_IMAGE_LAYOUT_GENERAL;

			// No layout can match this combo of roles
			if (roles & AttachmentSubpassRole::Resolve)
				return false;

			return layout == VK_IMAGE_LAYOUT_GENERAL || layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else if (roles & AttachmentSubpassRole::Resolve)
		{
			if (roles & AttachmentSubpassRole::Input)
				return layout == VK_IMAGE_LAYOUT_GENERAL;

			return layout == VK_IMAGE_LAYOUT_GENERAL || layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		else if (roles & AttachmentSubpassRole::Input)
		{
			return layout == VK_IMAGE_LAYOUT_GENERAL || layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		return false;
	}

	enum class SubpassExternalDependency
	{
		Color = 0x0001,
		Input = 0x0002,
		Depth = 0x0004
	};

	ATEMA_DECLARE_FLAGS(SubpassExternalDependency);

	enum class SubpassFeedbackDependency
	{
		Color = 0x0001,
		Depth = 0x0002
	};

	ATEMA_DECLARE_FLAGS(SubpassFeedbackDependency);

	struct SubpassData
	{
		std::vector<Flags<AttachmentSubpassRole>> roles;
		std::vector<VkImageLayout> layouts;

		Flags<SubpassExternalDependency> externalDependencies;
		Flags<SubpassFeedbackDependency> feedbackDependencies;

		Flags<AttachmentSubpassRole> subpassRoles;
		
		std::vector<VkAttachmentReference> color;
		std::vector<VkAttachmentReference> input;
		std::vector<VkAttachmentReference> resolve;
		std::vector<uint32_t> preserve;
		VkAttachmentReference depthStencil;
	};
}

VulkanRenderPass::VulkanRenderPass(const VulkanDevice& device, const RenderPass::Settings& settings) :
	RenderPass(),
	m_device(device),
	m_renderPass(VK_NULL_HANDLE),
	m_attachments(settings.attachments)
{
	ATEMA_ASSERT(!settings.attachments.empty(), "Invalid attachments");
	ATEMA_ASSERT(!settings.subpasses.empty(), "RenderPass need at least one subpass");

	//-----
	// Create attachments
	std::vector<AttachmentData> attachments;
	attachments.reserve(settings.attachments.size());
	
	for (auto& attachment : settings.attachments)
	{
		attachments.push_back(AttachmentData(attachment));
	}

	//-----
	// Create subpasses
	std::vector<SubpassData> subpasses;
	subpasses.reserve(settings.subpasses.size());

	for (auto& subpassSettings : settings.subpasses)
	{
		SubpassData subpass;

		// Initialize attachment references
		{
			subpass.roles.resize(settings.attachments.size());
			subpass.layouts.resize(settings.attachments.size(), VK_IMAGE_LAYOUT_UNDEFINED);

			// Input
			for (auto& attachmentIndex : subpassSettings.input)
			{
				if (attachmentIndex == RenderPass::UnusedAttachment)
				{
					subpass.input.push_back({ VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED });
				}
				else
				{
					// The layout will be defined later
					subpass.input.push_back({ attachmentIndex, VK_IMAGE_LAYOUT_UNDEFINED });

					subpass.roles[attachmentIndex] |= AttachmentSubpassRole::Input;
					subpass.subpassRoles |= AttachmentSubpassRole::Input;
				}
			}

			// Color
			for (auto& attachmentIndex : subpassSettings.color)
			{
				if (attachmentIndex == RenderPass::UnusedAttachment)
				{
					subpass.color.push_back({ VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED });
				}
				else
				{
					// The layout will be defined later
					subpass.color.push_back({ attachmentIndex, VK_IMAGE_LAYOUT_UNDEFINED });

					subpass.roles[attachmentIndex] |= AttachmentSubpassRole::Color;
					subpass.subpassRoles |= AttachmentSubpassRole::Color;
				}
			}

			// Resolve
			for (auto& attachmentIndex : subpassSettings.resolve)
			{
				if (attachmentIndex == RenderPass::UnusedAttachment)
				{
					subpass.resolve.push_back({ VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED });
				}
				else
				{
					// The layout will be defined later
					subpass.resolve.push_back({ attachmentIndex, VK_IMAGE_LAYOUT_UNDEFINED });

					subpass.roles[attachmentIndex] |= AttachmentSubpassRole::Resolve;
					subpass.subpassRoles |= AttachmentSubpassRole::Resolve;
				}
			}

			// Depth
			if (subpassSettings.depthStencil == RenderPass::UnusedAttachment)
			{
				subpass.depthStencil = { VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED };
			}
			else
			{
				subpass.depthStencil = { subpassSettings.depthStencil, VK_IMAGE_LAYOUT_UNDEFINED };

				subpass.roles[subpassSettings.depthStencil] |= AttachmentSubpassRole::Depth;
				subpass.subpassRoles |= AttachmentSubpassRole::Depth;
			}
		}
		
		// Initialize layouts and check user settings are valid
		{
			for (size_t i = 0; i < settings.attachments.size(); i++)
			{
				const auto& attachment = attachments[i];
				const auto& roles = subpass.roles[i];
				auto& layout = subpass.layouts[i];

				layout = checkAttachmentLayout(roles, attachment.hasDepth);
			}
		}

		// Update reference layouts
		{
			for (auto& reference : subpass.input)
			{
				if (reference.attachment != VK_ATTACHMENT_UNUSED)
					reference.layout = subpass.layouts[reference.attachment];
			}

			for (auto& reference : subpass.color)
			{
				if (reference.attachment != VK_ATTACHMENT_UNUSED)
					reference.layout = subpass.layouts[reference.attachment];
			}

			for (auto& reference : subpass.resolve)
			{
				if (reference.attachment != VK_ATTACHMENT_UNUSED)
					reference.layout = subpass.layouts[reference.attachment];
			}

			if (subpass.depthStencil.attachment != VK_ATTACHMENT_UNUSED)
				subpass.depthStencil.layout = subpass.layouts[subpass.depthStencil.attachment];
		}

		subpasses.push_back(subpass);
	}

	//-----
	// Override subpass layouts to avoid unnecessary transitions
	// Also check if we need external/feedback dependencies
	for (size_t attachmentIndex = 0; attachmentIndex < attachments.size(); attachmentIndex++)
	{
		auto& attachment = attachments[attachmentIndex];

		bool isUsed = false;
		
		auto currentLayout = attachment.description.initialLayout;

		for (size_t subpassIndex = 0; subpassIndex < subpasses.size(); subpassIndex++)
		{
			auto& subpass = subpasses[subpassIndex];

			const auto& roles = subpass.roles[attachmentIndex];

			// If the attachment was used before but not during this subpass, preserve its content for potential later use
			if (isUsed && !roles)
			{
				subpass.preserve.push_back(static_cast<uint32_t>(attachmentIndex));

				continue;
			}

			// First subpass using this attachment
			if (!isUsed && roles)
			{
				isUsed = true;

				// Ensure the initial layout is compatible to avoid useless transitions
				if (!isLayoutCompatible(currentLayout, roles))
				{
					//TODO: Implement this 
					// Override the initial layout to match the one we need during the first subpass
					// Only usable for certain image types and depending on loadOp
					//attachment.description.initialLayout = subpass.layouts[attachmentIndex];
					//currentLayout = attachment.description.initialLayout;

					// As we change the layout, we need an external dependency
					if (roles & AttachmentSubpassRole::Input)
						subpass.externalDependencies |= SubpassExternalDependency::Input;

					if (roles & AttachmentSubpassRole::Color)
						subpass.externalDependencies |= SubpassExternalDependency::Color;

					if (roles & AttachmentSubpassRole::Depth)
						subpass.externalDependencies |= SubpassExternalDependency::Depth;
				}
			}

			// Feedback dependencies if an attachment is used as an Input and as a Color/Depth
			{
				if (roles & AttachmentSubpassRole::Input && roles & AttachmentSubpassRole::Color)
					subpass.feedbackDependencies |= SubpassFeedbackDependency::Color;

				if (roles & AttachmentSubpassRole::Input && roles & AttachmentSubpassRole::Depth)
					subpass.feedbackDependencies |= SubpassFeedbackDependency::Depth;
			}

			// Check if the current attachment layout match its role
			// If it is, just use this one until we need to change it
			if (isLayoutCompatible(currentLayout, roles))
			{
				if (roles & AttachmentSubpassRole::Input)
				{
					for (auto& reference : subpass.input)
					{
						if (reference.attachment == attachmentIndex)
							reference.layout = currentLayout;
					}
				}

				if (roles & AttachmentSubpassRole::Color)
				{
					for (auto& reference : subpass.color)
					{
						if (reference.attachment == attachmentIndex)
							reference.layout = currentLayout;
					}
				}

				if (roles & AttachmentSubpassRole::Resolve)
				{
					for (auto& reference : subpass.resolve)
					{
						if (reference.attachment == attachmentIndex)
							reference.layout = currentLayout;
					}
				}

				if (roles & AttachmentSubpassRole::Depth && subpass.depthStencil.attachment == attachmentIndex)
				{
					subpass.depthStencil.layout = currentLayout;
				}

				// Override the layout defined earlier
				subpass.layouts[attachmentIndex] = currentLayout;
			}
			// If the layout is not compatible, use the one defined earlier
			else
			{
				currentLayout = subpass.layouts[attachmentIndex];
			}
		}
		
		// If no final layout was specified, keep the last one used
		if (attachment.description.finalLayout == VK_IMAGE_LAYOUT_UNDEFINED)
			attachment.description.finalLayout = currentLayout;
	}

	//-----
	// Create vulkan structures

	// Attachment descriptions
	std::vector<VkAttachmentDescription> attachmentDescriptions;
	attachmentDescriptions.reserve(attachments.size());

	for (auto& attachment : attachments)
	{
		attachmentDescriptions.push_back(attachment.description);
	}

	// Subpasses
	std::vector<VkSubpassDescription> subpassDescriptions;
	subpassDescriptions.reserve(subpasses.size());

	for (auto& subpass : subpasses)
	{
		VkSubpassDescription subpassDescription{};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		
		subpassDescription.inputAttachmentCount = static_cast<uint32_t>(subpass.input.size());
		subpassDescription.pInputAttachments = subpass.input.empty() ? nullptr : subpass.input.data();

		subpassDescription.colorAttachmentCount = static_cast<uint32_t>(subpass.color.size());
		subpassDescription.pColorAttachments = subpass.color.empty() ? nullptr : subpass.color.data();

		subpassDescription.pResolveAttachments = subpass.resolve.empty() ? nullptr : subpass.resolve.data();

		subpassDescription.preserveAttachmentCount = static_cast<uint32_t>(subpass.preserve.size());
		subpassDescription.pPreserveAttachments = subpass.preserve.empty() ? nullptr : subpass.preserve.data();

		subpassDescription.pDepthStencilAttachment = &subpass.depthStencil;

		subpassDescriptions.push_back(subpassDescription);
	}

	// Subpass dependencies
	std::vector<VkSubpassDependency> subpassDependencies;

	for (size_t subpassIndex = 0; subpassIndex < subpasses.size(); subpassIndex++)
	{
		const auto& subpass = subpasses[subpassIndex];

		// External dependencies
		if (subpass.externalDependencies)
		{
			auto& subpassDependency = subpassDependencies.emplace_back();

			subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			subpassDependency.dstSubpass = static_cast<uint32_t>(subpassIndex);

			if (subpass.externalDependencies & SubpassExternalDependency::Input)
			{
				subpassDependency.srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				subpassDependency.srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				
				subpassDependency.dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				subpassDependency.dstAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			}

			if (subpass.externalDependencies & SubpassExternalDependency::Color)
			{
				subpassDependency.srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				subpassDependency.srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				
				subpassDependency.dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				subpassDependency.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			}

			if (subpass.externalDependencies & SubpassExternalDependency::Depth)
			{
				subpassDependency.srcStageMask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				subpassDependency.srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				
				subpassDependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				subpassDependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			}
		}

		// Feedback dependencies
		if (subpass.feedbackDependencies)
		{
			auto& subpassDependency = subpassDependencies.emplace_back();

			subpassDependency.srcSubpass = static_cast<uint32_t>(subpassIndex);
			subpassDependency.dstSubpass = static_cast<uint32_t>(subpassIndex);
			
			subpassDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			if (subpass.feedbackDependencies & SubpassFeedbackDependency::Color)
			{
				subpassDependency.srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				subpassDependency.srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			}

			if (subpass.feedbackDependencies & SubpassFeedbackDependency::Depth)
			{
				subpassDependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				subpassDependency.srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			}

			subpassDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			subpassDependency.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		}

		// Dependencies between consecutive subpasses
		if (subpassIndex > 0)
		{
			const auto previousSubpassIndex = subpassIndex - 1;
			const auto& previousSubpass = subpasses[previousSubpassIndex];

			auto& subpassDependency = subpassDependencies.emplace_back();

			subpassDependency.srcSubpass = static_cast<uint32_t>(previousSubpassIndex);
			subpassDependency.dstSubpass = static_cast<uint32_t>(subpassIndex);

			subpassDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			// Previous (source)
			if (previousSubpass.subpassRoles & AttachmentSubpassRole::Color)
			{
				subpassDependency.srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				subpassDependency.srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			}

			if (previousSubpass.subpassRoles & AttachmentSubpassRole::Depth)
			{
				subpassDependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				subpassDependency.srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			}

			// Current (destination)
			if (subpass.subpassRoles & AttachmentSubpassRole::Input)
			{
				subpassDependency.dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				subpassDependency.dstAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			}

			if (subpass.subpassRoles & AttachmentSubpassRole::Color)
			{
				subpassDependency.dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				subpassDependency.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			}

			if (subpass.subpassRoles & AttachmentSubpassRole::Depth)
			{
				subpassDependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				subpassDependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			}
		}
	}
	
	//-----
	// Create render pass
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
	renderPassInfo.pAttachments = attachmentDescriptions.empty() ? nullptr : attachmentDescriptions.data();
	
	renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
	renderPassInfo.pSubpasses = subpassDescriptions.empty() ? nullptr : subpassDescriptions.data();
	
	renderPassInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
	renderPassInfo.pDependencies = subpassDependencies.empty() ? nullptr : subpassDependencies.data();

	ATEMA_VK_CHECK(m_device.vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass));
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
