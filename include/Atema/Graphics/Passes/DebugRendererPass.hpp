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

#ifndef ATEMA_GRAPHICS_DEBUGRENDERERPASS_HPP
#define ATEMA_GRAPHICS_DEBUGRENDERERPASS_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/FrameGraphTexture.hpp>
#include <Atema/Graphics/AbstractRenderPass.hpp>
#include <Atema/Renderer/Color.hpp>
#include <Atema/Renderer/DepthStencil.hpp>

#include <vector>
#include <optional>

namespace at
{
	class DebugRenderer;
	class Camera;
	class Renderable;
	class FrameGraphBuilder;

	class ATEMA_GRAPHICS_API DebugRendererPass : public AbstractRenderPass
	{
	public:
		struct Settings
		{
			// Output texture
			FrameGraphTextureHandle output = FrameGraph::InvalidTextureHandle;
			// Optional clear values
			// If unspecified or null, the attachment won't be cleared
			std::optional<Color> outputClearValue;

			// Depth texture handle for depth read/write operations
			FrameGraphTextureHandle depthStencil = FrameGraph::InvalidTextureHandle;
			// Optional clear values
			// If unspecified or null, the attachment won't be cleared
			std::optional<DepthStencil> depthStencilClearValue;
		};

		DebugRendererPass();
		DebugRendererPass(const DebugRendererPass& other) = default;
		DebugRendererPass(DebugRendererPass&& other) noexcept = default;
		~DebugRendererPass() = default;

		const char* getName() const noexcept override;

		FrameGraphPass& addToFrameGraph(FrameGraphBuilder& frameGraphBuilder, const Settings& settings);

		void execute(FrameGraphContext& context, const Settings& settings);

		DebugRendererPass& operator=(const DebugRendererPass& other) = default;
		DebugRendererPass& operator=(DebugRendererPass&& other) noexcept = default;

	protected:
		void beginFrame() override;

	private:
		Ptr<DebugRenderer> m_debugRenderer;
	};
}

#endif