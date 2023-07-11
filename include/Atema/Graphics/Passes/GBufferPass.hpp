/*
	Copyright 2023 Jordi SUBIRANA

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

#ifndef ATEMA_GRAPHICS_GBUFFERPASS_HPP
#define ATEMA_GRAPHICS_GBUFFERPASS_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/FrameGraphTexture.hpp>
#include <Atema/Graphics/AbstractRenderPass.hpp>
#include <Atema/Graphics/Renderable.hpp>
#include <Atema/Renderer/Renderer.hpp>
#include <Atema/Renderer/DepthStencil.hpp>

#include <vector>
#include <optional>

namespace at
{
	class Camera;
	class FrameGraphBuilder;
	
	class ATEMA_GRAPHICS_API GBufferPass : public AbstractRenderPass
	{
	public:
		struct Settings
		{
			// GBuffer texture handles
			std::vector<FrameGraphTextureHandle> gbuffer;
			// Optional clear values
			// If unspecified or null, the attachments won't be cleared
			std::vector<std::optional<Color>> gbufferClearValue;

			// Depth texture handle for depth-stencil read/write operations
			FrameGraphTextureHandle depthStencil = FrameGraph::InvalidTextureHandle;
			// Optional clear values
			// If unspecified or null, the attachment won't be cleared
			std::optional<DepthStencil> depthStencilClearValue;
		};

		GBufferPass() = delete;
		// threadCount : Number of threads this pass is allowed to use
		// 0 means as much as possible
		GBufferPass(size_t threadCount);
		GBufferPass(const GBufferPass& other) = default;
		GBufferPass(GBufferPass&& other) noexcept = default;
		~GBufferPass() = default;

		const char* getName() const noexcept override;

		FrameGraphPass& addToFrameGraph(FrameGraphBuilder& frameGraphBuilder, const Settings& settings);

		void updateResources(RenderFrame& renderFrame, CommandBuffer& commandBuffer) override;

		void execute(FrameGraphContext& context, const Settings& settings);

		GBufferPass& operator=(const GBufferPass& other) = default;
		GBufferPass& operator=(GBufferPass&& other) noexcept = default;

	protected:
		void beginFrame() override;
		void endFrame() override;

	private:
		struct FrameResources
		{
			Ptr<Buffer> buffer;
			Ptr<DescriptorSet> descriptorSet;
		};

		void frustumCull();
		void frustumCullElements(std::vector<RenderElement>& renderElements, size_t index, size_t count) const;
		void sortElements();
		void drawElements(CommandBuffer& commandBuffer, FrameResources& frameResources, size_t index, size_t count);

		size_t m_threadCount;
		
		std::vector<RenderElement> m_renderElements;
	
		std::array<FrameResources, Renderer::FramesInFlight> m_frameResources;
	};
}

#endif