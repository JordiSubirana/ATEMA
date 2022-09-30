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

		void beginSecondary(const Ptr<RenderPass>& renderPass, const Ptr<Framebuffer>& framebuffer) override;
		
		void beginRenderPass(const Ptr<RenderPass>& renderPass, const Ptr<Framebuffer>& framebuffer, const std::vector<ClearValue>& clearValues, bool useSecondaryCommands) override;

		void bindPipeline(const Ptr<GraphicsPipeline>& pipeline) override;

		void setViewport(const Viewport& viewport) override;

		void setScissor(const Vector2i& position, const Vector2u& size) override;

		void nextSubpass(bool useSecondaryCommands) override;

		void endRenderPass() override;

		void copyBuffer(const Ptr<Buffer>& srcBuffer, const Ptr<Buffer>& dstBuffer, size_t size, size_t srcOffset, size_t dstOffset) override;

		void copyBuffer(const Ptr<Buffer>& srcBuffer, const Ptr<Image>& dstImage, ImageLayout dstLayout) override;

		void copyImage(const Ptr<Image>& srcImage, ImageLayout srcLayout, uint32_t srcLayer, uint32_t srcMipLevel, const Ptr<Image>& dstImage, ImageLayout dstLayout, uint32_t dstLayer, uint32_t dstMipLevel, uint32_t layerCount) override;

		void blitImage(const Ptr<Image>& srcImage, ImageLayout srcLayout, uint32_t srcLayer, uint32_t srcMipLevel, const Ptr<Image>& dstImage, ImageLayout dstLayout, uint32_t dstLayer, uint32_t dstMipLevel, SamplerFilter filter, uint32_t layerCount) override;

		void bindVertexBuffer(const Ptr<Buffer>& buffer, uint32_t binding) override;

		void bindIndexBuffer(const Ptr<Buffer>& buffer, IndexType indexType) override;

		void bindDescriptorSet(uint32_t index, const Ptr<DescriptorSet>& descriptorSet) override;
		void bindDescriptorSet(uint32_t index, const Ptr<DescriptorSet>& descriptorSet, const std::vector<uint32_t>& dynamicBufferOffsets) override;
		
		void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;

		void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) override;

		void imageBarrier(const Ptr<Image>& image, Flags<PipelineStage> srcPipelineStages, Flags<MemoryAccess> srcMemoryAccesses, ImageLayout srcLayout, Flags<PipelineStage> dstPipelineStages, Flags<MemoryAccess> dstMemoryAccesses, ImageLayout dstLayout, uint32_t baseLayer = 0, uint32_t layerCount = 0, uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 0) override;

		void createMipmaps(const Ptr<Image>& image, Flags<PipelineStage> dstPipelineStages, Flags<MemoryAccess> dstMemoryAccesses, ImageLayout dstLayout) override;

		void executeSecondaryCommands(const std::vector<Ptr<CommandBuffer>>& commandBuffers) override;
		
		void end() override;
		
	private:
		const VulkanDevice& m_device;
		VkCommandBuffer m_commandBuffer;
		VkCommandPool m_commandPool;
		uint32_t m_queueFamilyIndex;
		bool m_singleUse;
		bool m_isSecondary;
		bool m_secondaryBegan;
		VkPipelineLayout m_currentPipelineLayout;
		VulkanRenderPass* m_currentRenderPass;
		uint32_t m_currentSubpassIndex;
	};
}

#endif
