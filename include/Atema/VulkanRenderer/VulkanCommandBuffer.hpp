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

#ifndef ATEMA_VULKANRENDERER_VULKANCOMMANDBUFFER_HPP
#define ATEMA_VULKANRENDERER_VULKANCOMMANDBUFFER_HPP

#include <Atema/VulkanRenderer/Config.hpp>
#include <Atema/Renderer/CommandBuffer.hpp>
#include <Atema/VulkanRenderer/Vulkan.hpp>

namespace at
{
	class VulkanRenderPass;

	class ATEMA_VULKANRENDERER_API VulkanCommandBuffer final : public CommandBuffer
	{
	public:
		VulkanCommandBuffer() = delete;
		VulkanCommandBuffer(VkCommandPool commandPool, QueueType queueType, uint32_t queueFamilyIndex, const CommandBuffer::Settings& settings);
		virtual ~VulkanCommandBuffer();

		VkCommandBuffer getHandle() const noexcept;

		uint32_t getQueueFamilyIndex() const noexcept;

		void begin() override;

		void beginSecondary(const RenderPass& renderPass, const Framebuffer& framebuffer) override;
		
		void beginRenderPass(const RenderPass& renderPass, const Framebuffer& framebuffer, const std::vector<ClearValue>& clearValues, bool useSecondaryCommands) override;

		void bindPipeline(const GraphicsPipeline& pipeline) override;

		void setViewport(const Viewport& viewport) override;

		void setScissor(const Vector2i& position, const Vector2u& size) override;

		void nextSubpass(bool useSecondaryCommands) override;

		void endRenderPass() override;

		void copyBuffer(const Buffer& srcBuffer, Buffer& dstBuffer, size_t size, size_t srcOffset, size_t dstOffset) override;

		void copyBuffer(const Buffer& srcBuffer, Image& dstImage, ImageLayout dstLayout) override;

		void copyImage(const Image& srcImage, ImageLayout srcLayout, uint32_t srcLayer, uint32_t srcMipLevel, Image& dstImage, ImageLayout dstLayout, uint32_t dstLayer, uint32_t dstMipLevel, uint32_t layerCount) override;

		void blitImage(const Image& srcImage, ImageLayout srcLayout, uint32_t srcLayer, uint32_t srcMipLevel, Image& dstImage, ImageLayout dstLayout, uint32_t dstLayer, uint32_t dstMipLevel, SamplerFilter filter, uint32_t layerCount) override;

		void bindVertexBuffer(const Buffer& buffer, uint32_t binding) override;

		void bindIndexBuffer(const Buffer& buffer, IndexType indexType) override;

		void bindDescriptorSet(uint32_t index, const DescriptorSet& descriptorSet, const uint32_t* dynamicBufferOffsets, size_t dynamicBufferOffsetsCount) override;
		
		void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;

		void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) override;

		void memoryBarrier(Flags<PipelineStage> srcPipelineStages, Flags<MemoryAccess> srcMemoryAccesses, Flags<PipelineStage> dstPipelineStages, Flags<MemoryAccess> dstMemoryAccesses) override;

		void bufferBarrier(const Buffer& buffer, Flags<PipelineStage> srcPipelineStages, Flags<PipelineStage> dstPipelineStages, Flags<MemoryAccess> srcMemoryAccesses, Flags<MemoryAccess> dstMemoryAccesses, size_t offset = 0, size_t size = 0) override;

		void imageBarrier(const Image& image, Flags<PipelineStage> srcPipelineStages, Flags<PipelineStage> dstPipelineStages, Flags<MemoryAccess> srcMemoryAccesses, Flags<MemoryAccess> dstMemoryAccesses, ImageLayout srcLayout, ImageLayout dstLayout, uint32_t baseLayer = 0, uint32_t layerCount = 0, uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 0) override;

		void releaseOwnership(const Buffer& buffer, QueueType dstQueueType, Flags<PipelineStage> srcPipelineStages, Flags<PipelineStage> dstPipelineStages, Flags<MemoryAccess> srcMemoryAccesses, size_t offset = 0, size_t size = 0) override;
		void releaseOwnership(const Image& image, QueueType dstQueueType, Flags<PipelineStage> srcPipelineStages, Flags<PipelineStage> dstPipelineStages, Flags<MemoryAccess> srcMemoryAccesses, ImageLayout srcLayout, ImageLayout dstLayout, uint32_t baseLayer = 0, uint32_t layerCount = 0, uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 0) override;

		void acquireOwnership(const Buffer& buffer, QueueType srcQueueType, Flags<PipelineStage> srcPipelineStages, Flags<PipelineStage> dstPipelineStages, Flags<MemoryAccess> dstMemoryAccesses, size_t offset = 0, size_t size = 0) override;
		void acquireOwnership(const Image& image, QueueType srcQueueType, Flags<PipelineStage> srcPipelineStages, Flags<PipelineStage> dstPipelineStages, Flags<MemoryAccess> dstMemoryAccesses, ImageLayout srcLayout, ImageLayout dstLayout, uint32_t baseLayer = 0, uint32_t layerCount = 0, uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 0) override;

		void createMipmaps(Image& image, Flags<PipelineStage> dstPipelineStages, Flags<MemoryAccess> dstMemoryAccesses, ImageLayout dstLayout) override;

		void executeSecondaryCommands(const std::vector<Ptr<CommandBuffer>>& commandBuffers) override;
		
		void end() override;
		
	private:
		void bufferBarrier(const Buffer& buffer, Flags<PipelineStage> srcPipelineStages, Flags<PipelineStage> dstPipelineStages, Flags<MemoryAccess> srcMemoryAccesses, Flags<MemoryAccess> dstMemoryAccesses, uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex, size_t offset, size_t size);
		void imageBarrier(const Image& image, Flags<PipelineStage> srcPipelineStages, Flags<PipelineStage> dstPipelineStages, Flags<MemoryAccess> srcMemoryAccesses, Flags<MemoryAccess> dstMemoryAccesses, uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex, ImageLayout srcLayout, ImageLayout dstLayout, uint32_t baseLayer, uint32_t layerCount, uint32_t baseMipLevel, uint32_t mipLevelCount);

		const VulkanDevice& m_device;
		VkCommandBuffer m_commandBuffer;
		VkCommandPool m_commandPool;
		uint32_t m_queueFamilyIndex;
		bool m_singleUse;
		bool m_isSecondary;
		bool m_secondaryBegan;
		VkPipelineLayout m_currentPipelineLayout;
		const VulkanRenderPass* m_currentRenderPass;
		uint32_t m_currentSubpassIndex;
	};
}

#endif
