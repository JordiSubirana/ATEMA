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

#include <Atema/Renderer/Viewport.hpp>
#include <Atema/VulkanRenderer/VulkanBuffer.hpp>
#include <Atema/VulkanRenderer/VulkanCommandBuffer.hpp>
#include <Atema/VulkanRenderer/VulkanCommandPool.hpp>
#include <Atema/VulkanRenderer/VulkanDescriptorSet.hpp>
#include <Atema/VulkanRenderer/VulkanFramebuffer.hpp>
#include <Atema/VulkanRenderer/VulkanGraphicsPipeline.hpp>
#include <Atema/VulkanRenderer/VulkanImage.hpp>
#include <Atema/VulkanRenderer/VulkanPhysicalDevice.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>
#include <Atema/VulkanRenderer/VulkanRenderPass.hpp>

using namespace at;

VulkanCommandBuffer::VulkanCommandBuffer(VkCommandPool commandPool, const CommandBuffer::Settings& settings) :
	CommandBuffer(),
	m_device(VulkanRenderer::instance().getDevice()),
	m_commandBuffer(VK_NULL_HANDLE),
	m_singleUse(settings.singleUse),
	m_isSecondary(settings.secondary),
	m_secondaryBegan(false),
	m_currentPipelineLayout(VK_NULL_HANDLE)
{
	m_commandPool = commandPool;
	
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_commandPool;
	allocInfo.level = settings.secondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	ATEMA_VK_CHECK(m_device.vkAllocateCommandBuffers(m_device, &allocInfo, &m_commandBuffer));
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
	m_device.vkFreeCommandBuffers(m_device, m_commandPool, 1, &m_commandBuffer);
	
	m_commandBuffer = VK_NULL_HANDLE;
}

VkCommandBuffer VulkanCommandBuffer::getHandle() const noexcept
{
	return m_commandBuffer;
}

void VulkanCommandBuffer::begin()
{
	if (!m_isSecondary)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		// VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : Executed once then rerecorded
		// VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : Secondary command buffer that will be entirely within a single render pass
		// VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : Can be resubmitted while it is also already pending execution
		beginInfo.flags = m_singleUse ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0; // Optional
		//beginInfo.pInheritanceInfo = nullptr; // Optional (for secondary command buffers)

		// Start recording (and reset command buffer if it was already recorded)
		ATEMA_VK_CHECK(m_device.vkBeginCommandBuffer(m_commandBuffer, &beginInfo));
	}
}

void VulkanCommandBuffer::beginSecondary(const Ptr<RenderPass>& renderPass, const Ptr<Framebuffer>& framebuffer)
{
	auto vkRenderPass = std::static_pointer_cast<VulkanRenderPass>(renderPass);
	auto vkFramebuffer = std::static_pointer_cast<VulkanFramebuffer>(framebuffer);

	ATEMA_ASSERT(vkRenderPass, "Invalid RenderPass");
	ATEMA_ASSERT(vkFramebuffer, "Invalid Framebuffer");

	auto framebufferSize = vkFramebuffer->getSize();

	// Inheritance info for the secondary command buffers
	VkCommandBufferInheritanceInfo inheritanceInfo{};
	inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritanceInfo.renderPass = vkRenderPass->getHandle();
	inheritanceInfo.subpass = 0;
	// Secondary command buffer also use the currently active framebuffer
	inheritanceInfo.framebuffer = vkFramebuffer->getHandle();
	// Misc
	inheritanceInfo.occlusionQueryEnable = VK_FALSE;

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	// VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : Executed once then rerecorded
	// VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : Secondary command buffer that will be entirely within a single render pass
	// VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : Can be resubmitted while it is also already pending execution
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | (m_singleUse ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0); // Optional
	beginInfo.pInheritanceInfo = &inheritanceInfo; // Optional (for secondary command buffers)

	// Start recording (and reset command buffer if it was already recorded)
	ATEMA_VK_CHECK(m_device.vkBeginCommandBuffer(m_commandBuffer, &beginInfo));
}

void VulkanCommandBuffer::beginRenderPass(const Ptr<RenderPass>& renderPass, const Ptr<Framebuffer>& framebuffer, const std::vector<ClearValue>& clearValues, bool useSecondaryCommands)
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

		if (Renderer::isDepthImageFormat(attachment.format) || Renderer::isStencilImageFormat(attachment.format))
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
	m_device.vkCmdBeginRenderPass(
		m_commandBuffer,
		&renderPassInfo,
		useSecondaryCommands ? VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanCommandBuffer::bindPipeline(const Ptr<GraphicsPipeline>& pipeline)
{
	auto vkPipeline = std::static_pointer_cast<VulkanGraphicsPipeline>(pipeline);

	m_device.vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline->getHandle());

	m_currentPipelineLayout = vkPipeline->getLayoutHandle();
}

void VulkanCommandBuffer::setViewport(const Viewport& viewport)
{
	VkViewport vkViewport;
	vkViewport.x = viewport.position.x;
	vkViewport.y = viewport.position.y;
	vkViewport.width = viewport.size.x;
	vkViewport.height = viewport.size.y;
	vkViewport.minDepth = viewport.minDepth;
	vkViewport.maxDepth = viewport.maxDepth;

	m_device.vkCmdSetViewport(m_commandBuffer, 0, 1, &vkViewport);
}

void VulkanCommandBuffer::setScissor(const Vector2i& position, const Vector2u& size)
{
	VkRect2D vkScissor;
	vkScissor.offset.x = static_cast<int32_t>(position.x);
	vkScissor.offset.y = static_cast<int32_t>(position.y);
	vkScissor.extent.width = static_cast<uint32_t>(size.x);
	vkScissor.extent.height = static_cast<uint32_t>(size.y);
	
	m_device.vkCmdSetScissor(m_commandBuffer, 0, 1, &vkScissor);
}

void VulkanCommandBuffer::endRenderPass()
{
	m_device.vkCmdEndRenderPass(m_commandBuffer);
}

void VulkanCommandBuffer::copyBuffer(const Ptr<Buffer>& srcBuffer, const Ptr<Buffer>& dstBuffer, size_t size, size_t srcOffset, size_t dstOffset)
{
	ATEMA_ASSERT(srcBuffer, "Invalid source buffer");
	ATEMA_ASSERT(dstBuffer, "Invalid destination buffer");

	const auto vkSrcBuffer = std::static_pointer_cast<VulkanBuffer>(srcBuffer);
	const auto vkDstBuffer = std::static_pointer_cast<VulkanBuffer>(dstBuffer);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = static_cast<VkDeviceSize>(srcOffset);
	copyRegion.dstOffset = static_cast<VkDeviceSize>(dstOffset);
	copyRegion.size = static_cast<VkDeviceSize>(size);
	
	m_device.vkCmdCopyBuffer(m_commandBuffer, vkSrcBuffer->getHandle(), vkDstBuffer->getHandle(), 1, &copyRegion);
}

void VulkanCommandBuffer::copyBuffer(const Ptr<Buffer>& srcBuffer, const Ptr<Image>& dstImage)
{
	ATEMA_ASSERT(srcBuffer, "Invalid source buffer");
	ATEMA_ASSERT(dstImage, "Invalid destination image");

	const auto vkBuffer = std::static_pointer_cast<VulkanBuffer>(srcBuffer);
	const auto vkImage = std::static_pointer_cast<VulkanImage>(dstImage);
	const auto layout = vkImage->getLayouts()[0];
	
	ATEMA_ASSERT(layout == VK_IMAGE_LAYOUT_GENERAL || layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, "Invalid image layout");

	const auto size = vkImage->getSize();
	
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = Vulkan::getAspect(vkImage->getFormat());
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { size.x, size.y, 1 };

	m_device.vkCmdCopyBufferToImage(
		m_commandBuffer,
		vkBuffer->getHandle(),
		vkImage->getImageHandle(),
		layout,
		1,
		&region
	);
}

void VulkanCommandBuffer::bindVertexBuffer(const Ptr<Buffer>& buffer, uint32_t binding)
{
	ATEMA_ASSERT(buffer, "Invalid buffer");

	const auto vkBuffer = std::static_pointer_cast<VulkanBuffer>(buffer)->getHandle();
	
	const VkDeviceSize offset = 0;
	
	m_device.vkCmdBindVertexBuffers(m_commandBuffer, binding, 1, &vkBuffer, &offset);
}

void VulkanCommandBuffer::bindIndexBuffer(const Ptr<Buffer>& buffer, IndexType indexType)
{
	ATEMA_ASSERT(buffer, "Invalid buffer");

	const auto vkBuffer = std::static_pointer_cast<VulkanBuffer>(buffer)->getHandle();

	m_device.vkCmdBindIndexBuffer(m_commandBuffer, vkBuffer, 0, Vulkan::getIndexType(indexType));
}

void VulkanCommandBuffer::bindDescriptorSet(uint32_t index, const Ptr<DescriptorSet>& descriptorSet)
{
	bindDescriptorSet(index, descriptorSet, {});
}

void VulkanCommandBuffer::bindDescriptorSet(uint32_t index, const Ptr<DescriptorSet>& descriptorSet, const std::vector<uint32_t>& dynamicBufferOffsets)
{
	ATEMA_ASSERT(descriptorSet, "Invalid descriptor set");

	const auto vkDescriptorSet = std::static_pointer_cast<VulkanDescriptorSet>(descriptorSet)->getHandle();

	m_device.vkCmdBindDescriptorSets(
		m_commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_currentPipelineLayout,
		index,
		1,
		&vkDescriptorSet,
		static_cast<uint32_t>(dynamicBufferOffsets.size()),
		dynamicBufferOffsets.empty() ? nullptr : dynamicBufferOffsets.data());
}

void VulkanCommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	m_device.vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanCommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	m_device.vkCmdDrawIndexed(m_commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanCommandBuffer::setImageLayout(const Ptr<Image>& image, ImageLayout layout, uint32_t firstMipLevel, uint32_t levelCount)
{
	ATEMA_ASSERT(image, "Invalid image");

	const auto vkImage = std::static_pointer_cast<VulkanImage>(image);
	const auto format = vkImage->getFormat();
	auto& layouts = vkImage->getLayouts();

	if (levelCount == 0)
		levelCount = static_cast<uint32_t>(layouts.size()) - firstMipLevel;

	const auto oldLayout = layouts[firstMipLevel];
	const auto newLayout = Vulkan::getLayout(layout, Renderer::isDepthImageFormat(format));
	
	if (oldLayout == newLayout)
		return;

	// Pipeline barriers are used to synchronize resources (ensure we are not writing & reading at the same time)
	// We can also change layout or change queue family ownership for a resource (when VK_SHARING_MODE_EXCLUSIVE is used)
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

	// Change layout
	barrier.oldLayout = oldLayout; // or VK_IMAGE_LAYOUT_UNDEFINED if we don't care about previous content
	barrier.newLayout = newLayout;

	// Change queue ownership (indices of the queue families if we want to use it)
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	// Specify image & subResourceRange (used for mipmaps or arrays)
	barrier.image = vkImage->getImageHandle();
	barrier.subresourceRange.baseMipLevel = firstMipLevel;
	barrier.subresourceRange.levelCount = levelCount;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (Renderer::isStencilImageFormat(format))
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	// Depends on what happens before and after the barrier
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	// Undefined to transfer destination : we don't need to wait on anything
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		// We don't need to wait : empty access mask
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		// We don't need to wait : earliest possible pipeline stage
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		// Not a real stage for graphics & compute pipelines
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	// Transfer destination to shader reading : shader reads should wait on transfer writes (here fragment shader)
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		// Earliest stage when we'll need to use the depth buffer (here to read)
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		ATEMA_ERROR("Unsupported layout transition");
	}

	m_device.vkCmdPipelineBarrier(
		m_commandBuffer,
		// Depends on what happens before and after the barrier
		sourceStage, // In which pipeline stage the operations occur (before the barrier)
		destinationStage, // The pipeline stage in which operations will wait on the barrier

		0,

		// All types of pipeline barriers can be used
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	for (uint32_t i = 0; i < levelCount; i++)
		layouts[i] = newLayout;
}

void VulkanCommandBuffer::createMipmaps(const Ptr<Image>& image)
{
	//TODO: Use mipmaps in different files, to avoid generating it a runtime

	ATEMA_ASSERT(image, "Invalid image");

	const auto vkImage = std::static_pointer_cast<VulkanImage>(image);
	const auto mipLevels = vkImage->getMipLevels();
	const auto format = Vulkan::getFormat(vkImage->getFormat());
	const auto size = vkImage->getSize();
	const auto imageHandle = vkImage->getImageHandle();
	auto& layouts = vkImage->getLayouts();
	auto oldLayouts = layouts;

	// Check if image format supports linear blitting (needed for vkCmdBlitImage)
	auto& renderer = VulkanRenderer::instance();
	const auto& physicalDevice = renderer.getPhysicalDevice().getHandle();

	VkFormatProperties formatProperties;
	renderer.getInstance().vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);

	// Here we check in optimal tiling features because our image is in optimal tiling
	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		//TODO: Instead of throwing an error, make a custom mipmap generation without using vkCmdBlitImage
		ATEMA_ERROR("Texture image format does not support linear blitting");
	}

	// Set image layout to TransferDst, then we'll transition each mip level to src when needed
	setImageLayout(image, ImageLayout::TransferDst);

	// Generate mip levels
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = vkImage->getImageHandle();
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = Vulkan::getAspect(vkImage->getFormat());
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = static_cast<int32_t>(size.x);
	int32_t mipHeight = static_cast<int32_t>(size.y);

	for (uint32_t i = 1; i < mipLevels; i++)
	{
		// Transition level i-1 to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
		// Will wait for this level to be filled by previous blit or vkCmdCopyBufferToImage
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		m_device.vkCmdPipelineBarrier(
			m_commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		layouts[i - 1] = barrier.newLayout;

		// Which regions will be used for the blit
		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;

		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		// Blit (must be submitted to a queue with graphics capability)
		m_device.vkCmdBlitImage(
			m_commandBuffer,
			imageHandle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		// Transition level i-1 to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		// Will wait on the blit command to finish
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		m_device.vkCmdPipelineBarrier(
			m_commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		layouts[i - 1] = barrier.newLayout;

		if (mipWidth > 1)
			mipWidth /= 2;

		if (mipHeight > 1)
			mipHeight /= 2;
	}

	// Transition the last level to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	m_device.vkCmdPipelineBarrier(
		m_commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	layouts[mipLevels - 1] = barrier.newLayout;
}

void VulkanCommandBuffer::executeSecondaryCommands(const std::vector<Ptr<CommandBuffer>>& commandBuffers)
{
	std::vector<VkCommandBuffer> vkCommandBuffers;
	vkCommandBuffers.reserve(commandBuffers.size());

	for (auto& commandBuffer : commandBuffers)
	{
		auto vkCommandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(commandBuffer);

		vkCommandBuffers.push_back(vkCommandBuffer->getHandle());
	}

	m_device.vkCmdExecuteCommands(m_commandBuffer, static_cast<uint32_t>(vkCommandBuffers.size()), vkCommandBuffers.data());
}

void VulkanCommandBuffer::end()
{
	if (m_isSecondary)
		m_secondaryBegan = false;

	ATEMA_VK_CHECK(m_device.vkEndCommandBuffer(m_commandBuffer));
}
