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

VulkanCommandBuffer::VulkanCommandBuffer(VkCommandPool commandPool, QueueType queueType, uint32_t queueFamilyIndex, const CommandBuffer::Settings& settings) :
	CommandBuffer(queueType),
	m_device(VulkanRenderer::instance().getDevice()),
	m_commandBuffer(VK_NULL_HANDLE),
	m_commandPool(commandPool),
	m_queueFamilyIndex(queueFamilyIndex),
	m_singleUse(settings.singleUse),
	m_isSecondary(settings.secondary),
	m_secondaryBegan(false),
	m_currentPipelineLayout(VK_NULL_HANDLE),
	m_currentRenderPass(nullptr),
	m_currentSubpassIndex(0)
{
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

uint32_t VulkanCommandBuffer::getQueueFamilyIndex() const noexcept
{
	return m_queueFamilyIndex;
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

void VulkanCommandBuffer::beginSecondary(const RenderPass& renderPass, const Framebuffer& framebuffer)
{
	const auto& vkRenderPass = static_cast<const VulkanRenderPass&>(renderPass);
	const auto& vkFramebuffer = static_cast<const VulkanFramebuffer&>(framebuffer);

	m_currentRenderPass = &vkRenderPass;
	m_currentSubpassIndex = 0;

	auto framebufferSize = vkFramebuffer.getSize();

	// Inheritance info for the secondary command buffers
	VkCommandBufferInheritanceInfo inheritanceInfo{};
	inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritanceInfo.renderPass = vkRenderPass.getHandle();
	inheritanceInfo.subpass = 0;
	// Secondary command buffer also use the currently active framebuffer
	inheritanceInfo.framebuffer = vkFramebuffer.getHandle();
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

void VulkanCommandBuffer::beginRenderPass(const RenderPass& renderPass, const Framebuffer& framebuffer, const std::vector<ClearValue>& clearValues, bool useSecondaryCommands)
{
	const auto& vkRenderPass = static_cast<const VulkanRenderPass&>(renderPass);
	const auto& vkFramebuffer = static_cast<const VulkanFramebuffer&>(framebuffer);

	m_currentRenderPass = &vkRenderPass;
	m_currentSubpassIndex = 0;
	
	auto framebufferSize = vkFramebuffer.getSize();
	auto& attachments = vkRenderPass.getAttachments();
	
	// Start render pass
	std::vector<VkClearValue> vkClearValues(clearValues.size());

	for (size_t i = 0; i < vkClearValues.size(); i++)
	{
		auto& value = clearValues[i];
		auto& attachment = attachments[i];

		if (Renderer::isDepthImageFormat(attachment.format) || Renderer::isStencilImageFormat(attachment.format))
		{
			const auto& depthStencil = value.get<DepthStencil>();

			vkClearValues[i].depthStencil.depth = depthStencil.depth;
			vkClearValues[i].depthStencil.stencil = depthStencil.stencil;
		}
		else
		{
			const auto& color = value.get<Color>();

			vkClearValues[i].color.float32[0] = color.r;
			vkClearValues[i].color.float32[1] = color.g;
			vkClearValues[i].color.float32[2] = color.b;
			vkClearValues[i].color.float32[3] = color.a;
		}
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = vkRenderPass.getHandle();
	renderPassInfo.framebuffer = vkFramebuffer.getHandle();
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

void VulkanCommandBuffer::bindPipeline(const GraphicsPipeline& pipeline)
{
	if (!m_currentRenderPass)
	{
		ATEMA_ERROR("No RenderPass is active");
	}

	const auto& vkPipeline = static_cast<const VulkanGraphicsPipeline&>(pipeline);
	auto pipelineHandle = vkPipeline.getHandle(*m_currentRenderPass, m_currentSubpassIndex);

	m_device.vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineHandle);

	m_currentPipelineLayout = vkPipeline.getLayoutHandle();
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

void VulkanCommandBuffer::nextSubpass(bool useSecondaryCommands)
{
	m_device.vkCmdNextSubpass(m_commandBuffer, useSecondaryCommands ? VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : VK_SUBPASS_CONTENTS_INLINE);

	m_currentSubpassIndex++;
}

void VulkanCommandBuffer::endRenderPass()
{
	m_device.vkCmdEndRenderPass(m_commandBuffer);

	m_currentRenderPass = nullptr;
	m_currentSubpassIndex = 0;
}

void VulkanCommandBuffer::copyBuffer(const Buffer& srcBuffer, Buffer& dstBuffer, size_t size, size_t srcOffset, size_t dstOffset)
{
	const auto& vkSrcBuffer = static_cast<const VulkanBuffer&>(srcBuffer);
	const auto& vkDstBuffer = static_cast<const VulkanBuffer&>(dstBuffer);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = static_cast<VkDeviceSize>(srcOffset);
	copyRegion.dstOffset = static_cast<VkDeviceSize>(dstOffset);
	copyRegion.size = static_cast<VkDeviceSize>(size);
	
	m_device.vkCmdCopyBuffer(m_commandBuffer, vkSrcBuffer.getHandle(), vkDstBuffer.getHandle(), 1, &copyRegion);
}

void VulkanCommandBuffer::copyBuffer(const Buffer& srcBuffer, Image& dstImage, ImageLayout dstLayout)
{
	const auto& vkBuffer = static_cast<const VulkanBuffer&>(srcBuffer);
	const auto& vkImage = static_cast<const VulkanImage&>(dstImage);
	const auto format = vkImage.getFormat();
	const auto layout = Vulkan::getLayout(dstLayout, Renderer::isDepthImageFormat(format));
	
	ATEMA_ASSERT(layout == VK_IMAGE_LAYOUT_GENERAL || layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, "Invalid image layout");

	const auto size = vkImage.getSize();
	
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = Vulkan::getAspect(vkImage.getFormat());
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { size.x, size.y, 1 };

	m_device.vkCmdCopyBufferToImage(
		m_commandBuffer,
		vkBuffer.getHandle(),
		vkImage.getHandle(),
		layout,
		1,
		&region
	);
}

void VulkanCommandBuffer::copyImage(
	const Image& srcImage, ImageLayout srcLayout, uint32_t srcLayer, uint32_t srcMipLevel,
	Image& dstImage, ImageLayout dstLayout, uint32_t dstLayer, uint32_t dstMipLevel,
	uint32_t layerCount)
{
	//TODO: Ensure src & dst format are compatible

	const auto srcFormat = srcImage.getFormat();
	const auto dstFormat = dstImage.getFormat();

	const auto vkSrcLayout = Vulkan::getLayout(srcLayout, Renderer::isDepthImageFormat(srcFormat));
	const auto vkDstLayout = Vulkan::getLayout(dstLayout, Renderer::isDepthImageFormat(dstFormat));

	ATEMA_ASSERT(vkSrcLayout == VK_IMAGE_LAYOUT_GENERAL || vkSrcLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, "Invalid source image layout");
	ATEMA_ASSERT(vkDstLayout == VK_IMAGE_LAYOUT_GENERAL || vkDstLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, "Invalid destination image layout");

	const auto& vkSrcImage = static_cast<const VulkanImage&>(srcImage);
	const auto& vkDstImage = static_cast<const VulkanImage&>(dstImage);

	const auto srcSize = srcImage.getSize();
	const auto dstSize = dstImage.getSize();

	// If layerCount is 0, we'll keep the minimum remaining layer count between src & dst
	if (layerCount == 0)
		layerCount = std::min(srcImage.getLayers() - srcLayer, dstImage.getLayers() - dstLayer);

	VkImageCopy region;
	region.extent.width = std::min(srcSize.x, dstSize.x);
	region.extent.height = std::min(srcSize.y, dstSize.y);
	region.extent.depth = 1;
	region.srcOffset = { 0, 0, 0 };
	region.srcSubresource.mipLevel = srcMipLevel;
	region.srcSubresource.baseArrayLayer = srcLayer;
	region.srcSubresource.layerCount = layerCount;
	region.srcSubresource.aspectMask = Vulkan::getAspect(srcFormat);
	region.dstOffset = { 0, 0, 0 };
	region.dstSubresource.mipLevel = dstMipLevel;
	region.dstSubresource.baseArrayLayer = dstLayer;
	region.dstSubresource.layerCount = layerCount;
	region.dstSubresource.aspectMask = Vulkan::getAspect(dstFormat);

	m_device.vkCmdCopyImage(m_commandBuffer, vkSrcImage.getHandle(), vkSrcLayout, vkDstImage.getHandle(), vkDstLayout, 1, &region);
}

void VulkanCommandBuffer::blitImage(
	const Image& srcImage, ImageLayout srcLayout, uint32_t srcLayer, uint32_t srcMipLevel,
	Image& dstImage, ImageLayout dstLayout, uint32_t dstLayer, uint32_t dstMipLevel,
	SamplerFilter filter, uint32_t layerCount)
{
	//TODO: Ensure src & dst format are compatible

	const auto srcFormat = srcImage.getFormat();
	const auto dstFormat = dstImage.getFormat();

	const auto vkSrcLayout = Vulkan::getLayout(srcLayout, Renderer::isDepthImageFormat(srcFormat));
	const auto vkDstLayout = Vulkan::getLayout(dstLayout, Renderer::isDepthImageFormat(dstFormat));

	ATEMA_ASSERT(vkSrcLayout == VK_IMAGE_LAYOUT_GENERAL || vkSrcLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, "Invalid source image layout");
	ATEMA_ASSERT(vkDstLayout == VK_IMAGE_LAYOUT_GENERAL || vkDstLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, "Invalid destination image layout");

	const auto& vkSrcImage = static_cast<const VulkanImage&>(srcImage);
	const auto& vkDstImage = static_cast<const VulkanImage&>(dstImage);

	const auto srcSize = srcImage.getSize();
	const auto dstSize = dstImage.getSize();

	// If layerCount is 0, we'll keep the minimum remaining layer count between src & dst
	if (layerCount == 0)
		layerCount = std::min(srcImage.getLayers() - srcLayer, dstImage.getLayers() - dstLayer);

	VkImageBlit region;
	region.srcSubresource.mipLevel = srcMipLevel;
	region.srcSubresource.baseArrayLayer = srcLayer;
	region.srcSubresource.layerCount = layerCount;
	region.srcSubresource.aspectMask = Vulkan::getAspect(srcFormat);
	region.dstSubresource.mipLevel = dstMipLevel;
	region.dstSubresource.baseArrayLayer = dstLayer;
	region.dstSubresource.layerCount = layerCount;
	region.dstSubresource.aspectMask = Vulkan::getAspect(dstFormat);

	m_device.vkCmdBlitImage(m_commandBuffer, vkSrcImage.getHandle(), vkSrcLayout, vkDstImage.getHandle(), vkDstLayout, 1, &region, Vulkan::getSamplerFilter(filter));
}

void VulkanCommandBuffer::bindVertexBuffer(const Buffer& buffer, uint32_t binding)
{
	const auto vkBuffer = static_cast<const VulkanBuffer&>(buffer).getHandle();
	
	const VkDeviceSize offset = 0;
	
	m_device.vkCmdBindVertexBuffers(m_commandBuffer, binding, 1, &vkBuffer, &offset);
}

void VulkanCommandBuffer::bindIndexBuffer(const Buffer& buffer, IndexType indexType)
{
	const auto vkBuffer = static_cast<const VulkanBuffer&>(buffer).getHandle();

	m_device.vkCmdBindIndexBuffer(m_commandBuffer, vkBuffer, 0, Vulkan::getIndexType(indexType));
}

void VulkanCommandBuffer::bindDescriptorSet(uint32_t index, const DescriptorSet& descriptorSet, const uint32_t* dynamicBufferOffsets, size_t dynamicBufferOffsetsCount)
{
	const auto vkDescriptorSet = static_cast<const VulkanDescriptorSet&>(descriptorSet).getHandle();

	m_device.vkCmdBindDescriptorSets(
		m_commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_currentPipelineLayout,
		index,
		1,
		&vkDescriptorSet,
		static_cast<uint32_t>(dynamicBufferOffsetsCount),
		dynamicBufferOffsets);
}

void VulkanCommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	m_device.vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanCommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	m_device.vkCmdDrawIndexed(m_commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanCommandBuffer::memoryBarrier(Flags<PipelineStage> srcPipelineStages, Flags<MemoryAccess> srcMemoryAccesses, Flags<PipelineStage> dstPipelineStages, Flags<MemoryAccess> dstMemoryAccesses)
{
	// Pipeline stages
	const auto srcStages = Vulkan::getPipelineStages(srcPipelineStages);
	const auto dstStages = Vulkan::getPipelineStages(dstPipelineStages);

	VkMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;

	// Access mask
	barrier.srcAccessMask = Vulkan::getMemoryAccesses(srcMemoryAccesses);
	barrier.dstAccessMask = Vulkan::getMemoryAccesses(dstMemoryAccesses);

	// Pipeline barrier
	m_device.vkCmdPipelineBarrier(
		m_commandBuffer,

		// Depends on what happens before and after the barrier
		srcStages, // In which pipeline stage the operations occur (before the barrier)
		dstStages, // The pipeline stage in which operations will wait on the barrier

		0,

		// All types of pipeline barriers can be used
		1, &barrier,
		0, nullptr,
		0, nullptr
	);
}

void VulkanCommandBuffer::bufferBarrier(const Buffer& buffer,
	Flags<PipelineStage> srcPipelineStages, Flags<PipelineStage> dstPipelineStages,
	Flags<MemoryAccess> srcMemoryAccesses, Flags<MemoryAccess> dstMemoryAccesses,
	size_t offset, size_t size)
{
	bufferBarrier(buffer,
		srcPipelineStages, dstPipelineStages,
		srcMemoryAccesses, dstMemoryAccesses,
		VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
		offset, size);
}

void VulkanCommandBuffer::imageBarrier(const Image& image,
	Flags<PipelineStage> srcPipelineStages, Flags<PipelineStage> dstPipelineStages,
	Flags<MemoryAccess> srcMemoryAccesses, Flags<MemoryAccess> dstMemoryAccesses,
	ImageLayout srcLayout, ImageLayout dstLayout,
	uint32_t baseLayer, uint32_t layerCount, uint32_t baseMipLevel, uint32_t mipLevelCount)
{
	imageBarrier(image,
		srcPipelineStages, dstPipelineStages,
		srcMemoryAccesses, dstMemoryAccesses,
		VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
		srcLayout, dstLayout,
		baseLayer, layerCount, baseMipLevel, mipLevelCount);
}

void VulkanCommandBuffer::releaseOwnership(const Buffer& buffer, QueueType dstQueueType, Flags<PipelineStage> srcPipelineStages, Flags<PipelineStage> dstPipelineStages, Flags<MemoryAccess> srcMemoryAccesses, size_t offset, size_t size)
{
	bufferBarrier(buffer,
		srcPipelineStages, dstPipelineStages,
		srcMemoryAccesses, 0,
		m_queueFamilyIndex, m_device.getQueueFamilyIndex(dstQueueType),
		offset, size);
}

void VulkanCommandBuffer::releaseOwnership(const Image& image, QueueType dstQueueType, Flags<PipelineStage> srcPipelineStages, Flags<PipelineStage> dstPipelineStages, Flags<MemoryAccess> srcMemoryAccesses, ImageLayout srcLayout, ImageLayout dstLayout, uint32_t baseLayer, uint32_t layerCount, uint32_t baseMipLevel, uint32_t mipLevelCount)
{
	imageBarrier(image,
		srcPipelineStages, dstPipelineStages,
		srcMemoryAccesses, 0,
		m_queueFamilyIndex, m_device.getQueueFamilyIndex(dstQueueType),
		srcLayout, dstLayout,
		baseLayer, layerCount, baseMipLevel, mipLevelCount);
}

void VulkanCommandBuffer::acquireOwnership(const Buffer& buffer, QueueType srcQueueType, Flags<PipelineStage> srcPipelineStages, Flags<PipelineStage> dstPipelineStages, Flags<MemoryAccess> dstMemoryAccesses, size_t offset, size_t size)
{
	bufferBarrier(buffer,
		srcPipelineStages, dstPipelineStages,
		0, dstMemoryAccesses,
		m_device.getQueueFamilyIndex(srcQueueType), m_queueFamilyIndex,
		offset, size);
}

void VulkanCommandBuffer::acquireOwnership(const Image& image, QueueType srcQueueType, Flags<PipelineStage> srcPipelineStages, Flags<PipelineStage> dstPipelineStages, Flags<MemoryAccess> dstMemoryAccesses, ImageLayout srcLayout, ImageLayout dstLayout, uint32_t baseLayer, uint32_t layerCount, uint32_t baseMipLevel, uint32_t mipLevelCount)
{
	imageBarrier(image,
		srcPipelineStages, dstPipelineStages,
		0, dstMemoryAccesses,
		m_device.getQueueFamilyIndex(srcQueueType), m_queueFamilyIndex,
		srcLayout, dstLayout,
		baseLayer, layerCount, baseMipLevel, mipLevelCount);
}

void VulkanCommandBuffer::createMipmaps(Image& image, Flags<PipelineStage> dstPipelineStages, Flags<MemoryAccess> dstMemoryAccesses, ImageLayout dstLayout)
{
	//TODO: Use mipmaps in different files, to avoid generating it a runtime

	const auto& vkImage = static_cast<const VulkanImage&>(image);
	const auto mipLevels = vkImage.getMipLevels();
	const auto format = Vulkan::getFormat(vkImage.getFormat());
	const auto size = vkImage.getSize();
	const auto imageHandle = vkImage.getHandle();

	const auto dstStages = Vulkan::getPipelineStages(dstPipelineStages);
	const auto dstAccessMask = Vulkan::getMemoryAccesses(dstMemoryAccesses);
	const auto newLayout = Vulkan::getLayout(dstLayout, Renderer::isDepthImageFormat(vkImage.getFormat()));

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

	// Generate mip levels
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = imageHandle;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = Vulkan::getAspect(vkImage.getFormat());
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

		// Transition level i-1 to the desired layout
		// Will wait on the blit command to finish
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = newLayout;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = dstAccessMask;

		m_device.vkCmdPipelineBarrier(
			m_commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, dstStages, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipWidth > 1)
			mipWidth /= 2;

		if (mipHeight > 1)
			mipHeight /= 2;
	}

	// Transition the last level to the desired layout
	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = newLayout;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = dstAccessMask;

	m_device.vkCmdPipelineBarrier(
		m_commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, dstStages, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);
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

void VulkanCommandBuffer::bufferBarrier(const Buffer& buffer,
	Flags<PipelineStage> srcPipelineStages, Flags<PipelineStage> dstPipelineStages,
	Flags<MemoryAccess> srcMemoryAccesses, Flags<MemoryAccess> dstMemoryAccesses,
	uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex,
	size_t offset, size_t size)
{
	VkBufferMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;

	// Access mask
	barrier.srcAccessMask = Vulkan::getMemoryAccesses(srcMemoryAccesses);
	barrier.dstAccessMask = Vulkan::getMemoryAccesses(dstMemoryAccesses);

	// Change queue ownership (indices of the queue families if we want to use it)
	barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
	barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;

	// Buffer resource & range
	barrier.buffer = static_cast<const VulkanBuffer&>(buffer).getHandle();
	barrier.offset = static_cast<VkDeviceSize>(offset);
	barrier.size = size == 0 ? VK_WHOLE_SIZE : static_cast<VkDeviceSize>(size);

	// Pipeline barrier
	m_device.vkCmdPipelineBarrier(
		m_commandBuffer,

		// Depends on what happens before and after the barrier
		Vulkan::getPipelineStages(srcPipelineStages), // In which pipeline stage the operations occur (before the barrier)
		Vulkan::getPipelineStages(dstPipelineStages), // The pipeline stage in which operations will wait on the barrier

		0,

		// All types of pipeline barriers can be used
		0, nullptr,
		1, &barrier,
		0, nullptr
	);
}

void VulkanCommandBuffer::imageBarrier(const Image& image,
	Flags<PipelineStage> srcPipelineStages, Flags<PipelineStage> dstPipelineStages,
	Flags<MemoryAccess> srcMemoryAccesses, Flags<MemoryAccess> dstMemoryAccesses,
	uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex,
	ImageLayout srcLayout, ImageLayout dstLayout,
	uint32_t baseLayer, uint32_t layerCount, uint32_t baseMipLevel, uint32_t mipLevelCount)
{
	const auto& vkImage = static_cast<const VulkanImage&>(image);
	const auto format = vkImage.getFormat();

	// Pipeline barriers are used to synchronize resources (ensure we are not writing & reading at the same time)
	// We can also change layout or change queue family ownership for a resource (when VK_SHARING_MODE_EXCLUSIVE is used)
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

	// Access mask
	barrier.srcAccessMask = Vulkan::getMemoryAccesses(srcMemoryAccesses);
	barrier.dstAccessMask = Vulkan::getMemoryAccesses(dstMemoryAccesses);

	// Change layout
	barrier.oldLayout = Vulkan::getLayout(srcLayout, Renderer::isDepthImageFormat(format)); // or VK_IMAGE_LAYOUT_UNDEFINED if we don't care about previous content
	barrier.newLayout = Vulkan::getLayout(dstLayout, Renderer::isDepthImageFormat(format));

	// Change queue ownership (indices of the queue families if we want to use it)
	barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
	barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;

	// Specify image & subResourceRange (used for mipmaps or arrays)
	barrier.image = vkImage.getHandle();
	barrier.subresourceRange.baseMipLevel = baseMipLevel;
	barrier.subresourceRange.levelCount = mipLevelCount == 0 ? VK_REMAINING_MIP_LEVELS : mipLevelCount;
	barrier.subresourceRange.baseArrayLayer = baseLayer;
	barrier.subresourceRange.layerCount = layerCount == 0 ? VK_REMAINING_ARRAY_LAYERS : layerCount;
	barrier.subresourceRange.aspectMask = Vulkan::getAspect(format);

	// Pipeline barrier
	m_device.vkCmdPipelineBarrier(
		m_commandBuffer,

		// Depends on what happens before and after the barrier
		Vulkan::getPipelineStages(srcPipelineStages), // In which pipeline stage the operations occur (before the barrier)
		Vulkan::getPipelineStages(dstPipelineStages), // The pipeline stage in which operations will wait on the barrier

		0,

		// All types of pipeline barriers can be used
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
}
