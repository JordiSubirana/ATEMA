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

#ifndef ATEMA_RENDERER_COMMANDBUFFER_HPP
#define ATEMA_RENDERER_COMMANDBUFFER_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Renderer/Enums.hpp>
#include <Atema/Math/Vector.hpp>
#include <Atema/Renderer/Color.hpp>
#include <Atema/Core/Variant.hpp>
#include <Atema/Renderer/DepthStencil.hpp>

#include <vector>

namespace at
{
	class Viewport;
	class Buffer;
	class CommandPool;
	class DescriptorSet;
	class Framebuffer;
	class GraphicsPipeline;
	class Image;
	class RenderPass;

	class ATEMA_RENDERER_API CommandBuffer : public NonCopyable
	{
	public:
		struct Settings
		{
			bool singleUse = false;
			bool secondary = false;
		};

		using ClearValue = Variant<Color, DepthStencil>;

		CommandBuffer() = delete;
		virtual ~CommandBuffer();

		QueueType getQueueType() const noexcept;

		virtual void begin() = 0;

		virtual void beginSecondary(const Ptr<RenderPass>& renderPass, const Ptr<Framebuffer>& framebuffer) = 0;

		virtual void beginRenderPass(const Ptr<RenderPass>& renderPass, const Ptr<Framebuffer>& framebuffer, const std::vector<ClearValue>& clearValues, bool useSecondaryCommands = false) = 0;

		virtual void bindPipeline(const Ptr<GraphicsPipeline>& pipeline) = 0;

		virtual void setViewport(const Viewport& viewport) = 0;
		
		virtual void setScissor(const Vector2i& position, const Vector2u& size) = 0;

		virtual void nextSubpass(bool useSecondaryCommands = false) = 0;

		virtual void endRenderPass() = 0;

		virtual void copyBuffer(const Ptr<Buffer>& srcBuffer, const Ptr<Buffer>& dstBuffer, size_t size, size_t srcOffset = 0, size_t dstOffset = 0) = 0;

		// Copy buffer data to an image
		// dstLayout must either be ImageLayout::TransferDst or ImageLayout::General
		virtual void copyBuffer(const Ptr<Buffer>& srcBuffer, const Ptr<Image>& dstImage, ImageLayout dstLayout) = 0;

		// Copy some image layers to another image layers
		// srcLayout must either be ImageLayout::TransferSrc or ImageLayout::General
		// dstLayout must either be ImageLayout::TransferDst or ImageLayout::General
		virtual void copyImage(const Ptr<Image>& srcImage, ImageLayout srcLayout, uint32_t srcLayer, uint32_t srcMipLevel, const Ptr<Image>& dstImage, ImageLayout dstLayout, uint32_t dstLayer, uint32_t dstMipLevel, uint32_t layerCount = 1) = 0;

		// Blit some image layers to another image layers
		// srcLayout must either be ImageLayout::TransferSrc or ImageLayout::General
		// dstLayout must either be ImageLayout::TransferDst or ImageLayout::General
		virtual void blitImage(const Ptr<Image>& srcImage, ImageLayout srcLayout, uint32_t srcLayer, uint32_t srcMipLevel, const Ptr<Image>& dstImage, ImageLayout dstLayout, uint32_t dstLayer, uint32_t dstMipLevel, SamplerFilter filter, uint32_t layerCount = 1) = 0;

		virtual void bindVertexBuffer(const Ptr<Buffer>& buffer, uint32_t binding) = 0;

		virtual void bindIndexBuffer(const Ptr<Buffer>& buffer, IndexType indexType) = 0;

		virtual void bindDescriptorSet(uint32_t index, const Ptr<DescriptorSet>& descriptorSet) = 0;
		virtual void bindDescriptorSet(uint32_t index, const Ptr<DescriptorSet>& descriptorSet, const std::vector<uint32_t>& dynamicBufferOffsets) = 0;

		virtual void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) = 0;

		virtual void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) = 0;

		void imageBarrier(const Ptr<Image>& image, ImageBarrier barrier);
		virtual void imageBarrier(const Ptr<Image>& image, Flags<PipelineStage> srcPipelineStages, Flags<MemoryAccess> srcMemoryAccesses, ImageLayout srcLayout, Flags<PipelineStage> dstPipelineStages, Flags<MemoryAccess> dstMemoryAccesses, ImageLayout dstLayout, uint32_t baseLayer = 0, uint32_t layerCount = 0, uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 0) = 0;

		// ImageLayout::TransferDst needed
		virtual void createMipmaps(const Ptr<Image>& image, Flags<PipelineStage> dstPipelineStages, Flags<MemoryAccess> dstMemoryAccesses, ImageLayout dstLayout) = 0;

		virtual void executeSecondaryCommands(const std::vector<Ptr<CommandBuffer>>& commandBuffers) = 0;
		
		virtual void end() = 0;
		
	protected:
		CommandBuffer(QueueType queueType);

	private:
		QueueType m_queueType;
	};
}

#endif
