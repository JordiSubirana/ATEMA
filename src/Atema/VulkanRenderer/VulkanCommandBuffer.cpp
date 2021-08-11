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

#include <Atema/VulkanRenderer/VulkanCommandBuffer.hpp>
#include <Atema/VulkanRenderer/VulkanCommandPool.hpp>
#include <Atema/VulkanRenderer/VulkanFramebuffer.hpp>
#include <Atema/VulkanRenderer/VulkanGraphicsPipeline.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>
#include <Atema/VulkanRenderer/VulkanRenderPass.hpp>

using namespace at;

VulkanCommandBuffer::VulkanCommandBuffer(const CommandBuffer::Settings& settings) :
	CommandBuffer(),
	m_commandBuffer(VK_NULL_HANDLE)
{
	auto& renderer = VulkanRenderer::getInstance();
	auto device = renderer.getLogicalDeviceHandle();

	auto commandPool = std::static_pointer_cast<VulkanCommandPool>(settings.commandPool);

	if (!commandPool)
	{
		ATEMA_ERROR("Invalid CommandPool");
	}

	m_commandPool = commandPool->getHandle();
	
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Primary, secondary
	allocInfo.commandBufferCount = 1;

	ATEMA_VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, &m_commandBuffer));
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
	auto& renderer = VulkanRenderer::getInstance();
	auto device = renderer.getLogicalDeviceHandle();

	vkFreeCommandBuffers(device, m_commandPool, 1, &m_commandBuffer);
	
	m_commandBuffer = VK_NULL_HANDLE;
}

VkCommandBuffer VulkanCommandBuffer::getHandle() const noexcept
{
	return m_commandBuffer;
}

void VulkanCommandBuffer::begin()
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	// VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : Executed once then rerecorded
	// VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : Secondary command buffer that will be entirely within a single render pass
	// VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : Can be resubmitted while it is also already pending execution
	//beginInfo.flags = 0; // Optional
	//beginInfo.pInheritanceInfo = nullptr; // Optional (for secondary command buffers)

	// Start recording (and reset command buffer if it was already recorded)
	ATEMA_VK_CHECK(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));
}

void VulkanCommandBuffer::beginRenderPass(const Ptr<RenderPass>& renderPass, const Ptr<Framebuffer>& framebuffer, const std::vector<ClearValue>& clearValues)
{
	auto vkRenderPass = std::static_pointer_cast<VulkanRenderPass>(renderPass);
	auto vkFramebuffer = std::static_pointer_cast<VulkanFramebuffer>(framebuffer);

	ATEMA_ASSERT(vkRenderPass, "Invalid RenderPass");
	ATEMA_ASSERT(vkFramebuffer, "Invalid Framebuffer");
	
	auto framebufferSize = vkFramebuffer->getSize();
	auto& attachments = vkRenderPass->getAttachments();
	
	// Start render pass
	std::vector<VkClearValue> vkClearValues(clearValues.size());

	for (size_t i = 0; i < vkClearValues.size(); i++)
	{
		auto& value = clearValues[i];
		auto& attachment = attachments[i];

		if (hasDepth(attachment.format) || hasStencil(attachment.format))
		{
			vkClearValues[i].depthStencil.depth = value.depthStencil.depth;
			vkClearValues[i].depthStencil.stencil = value.depthStencil.stencil;
		}
		else
		{
			vkClearValues[i].color.float32[0] = value.color[0];
			vkClearValues[i].color.float32[1] = value.color[1];
			vkClearValues[i].color.float32[2] = value.color[2];
			vkClearValues[i].color.float32[3] = value.color[3];
		}
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = vkRenderPass->getHandle();
	renderPassInfo.framebuffer = vkFramebuffer->getHandle();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = { framebufferSize.x, framebufferSize.y };
	renderPassInfo.clearValueCount = static_cast<uint32_t>(vkClearValues.size());
	renderPassInfo.pClearValues = vkClearValues.data();

	// VK_SUBPASS_CONTENTS_INLINE : render pass in primary command buffer, no secondary buffer will be used
	// VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : render pass commands executed in secondary command buffer
	vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanCommandBuffer::bindPipeline(const Ptr<GraphicsPipeline>& pipeline)
{
	auto vkPipeline = std::static_pointer_cast<VulkanGraphicsPipeline>(pipeline);

	vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline->getHandle());
}

void VulkanCommandBuffer::endRenderPass()
{
	vkCmdEndRenderPass(m_commandBuffer);
}

void VulkanCommandBuffer::end()
{
	ATEMA_VK_CHECK(vkEndCommandBuffer(m_commandBuffer));
}
