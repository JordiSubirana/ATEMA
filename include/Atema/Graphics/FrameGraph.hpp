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

#ifndef ATEMA_GRAPHICS_FRAMEGRAPH_HPP
#define ATEMA_GRAPHICS_FRAMEGRAPH_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/FrameGraphTexture.hpp>
#include <Atema/Renderer/Enums.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Graphics/FrameGraphPass.hpp>
#include <Atema/Renderer/CommandBuffer.hpp>

#include <limits>
#include <unordered_map>
#include <vector>

namespace at
{
	class Framebuffer;
	class Image;
	class RenderFrame;
	class RenderPass;

	class ATEMA_GRAPHICS_API FrameGraph
	{
		friend class FrameGraphBuilder;

	public:
		static constexpr FrameGraphTextureHandle InvalidTextureHandle = std::numeric_limits<FrameGraphTextureHandle>::max();

		FrameGraph();
		~FrameGraph();

		void execute(RenderFrame& renderFrame);
		
	private:
		struct TextureBarrier
		{
			// The barrier is valid (used)
			bool valid = false;

			// The barrier can be a renderpass barrier
			bool insideRenderPass = false;

			Flags<PipelineStage> srcPipelineStages;
			Flags<MemoryAccess> srcMemoryAccesses;
			ImageLayout srcLayout = ImageLayout::Undefined;

			Flags<PipelineStage> dstPipelineStages;
			Flags<MemoryAccess> dstMemoryAccesses;
			ImageLayout dstLayout = ImageLayout::Undefined;
		};

		struct Texture
		{
			Ptr<Image> image;

			std::vector<TextureBarrier> barriers;
		};

		struct Pass
		{
			Ptr<RenderPass> renderPass;

			Ptr<Framebuffer> framebuffer;

			std::vector<CommandBuffer::ClearValue> clearValues;

			std::unordered_map<FrameGraphTextureHandle, WPtr<Image>> textures;

			FrameGraphPass::ExecutionCallback executionCallback;

			bool useRenderFrameOutput;

			bool useSecondaryCommandBuffers;
		};

		std::vector<Pass> m_passes;
		std::vector<Texture> m_textures;
	};
}

#endif
