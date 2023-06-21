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

#ifndef ATEMA_GRAPHICS_DEBUGFRAMEGRAPHPASS_HPP
#define ATEMA_GRAPHICS_DEBUGFRAMEGRAPHPASS_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/FrameGraphTexture.hpp>
#include <Atema/Graphics/AbstractRenderPass.hpp>
#include <Atema/Renderer/Color.hpp>
#include <Atema/Renderer/DepthStencil.hpp>

#include <vector>
#include <optional>

namespace at
{
	class VertexBuffer;
	class Sampler;
	class DescriptorSetLayout;
	class Camera;
	class Renderable;
	class FrameGraphBuilder;

	class ATEMA_GRAPHICS_API DebugFrameGraphPass : public AbstractRenderPass
	{
	public:
		struct Settings
		{
			// Output texture
			FrameGraphTextureHandle output = FrameGraph::InvalidTextureHandle;
			// Optional clear values
			// If unspecified or null, the attachment won't be cleared
			std::optional<Color> outputClearValue;

			// Textures to show, separated in an array (size defined by rowCount/columnCount)
			std::vector<FrameGraphTextureHandle> textures;
			// How many columns is the output divided into
			// The rows will be set to the same value
			// 0 means the pass will adapt to the number of textures to display
			size_t columnCount = 0;
		};

		DebugFrameGraphPass();
		DebugFrameGraphPass(const DebugFrameGraphPass& other) = default;
		DebugFrameGraphPass(DebugFrameGraphPass&& other) noexcept = default;
		~DebugFrameGraphPass() = default;

		const char* getName() const noexcept override;

		FrameGraphPass& addToFrameGraph(FrameGraphBuilder& frameGraphBuilder, const Settings& settings);

		void execute(FrameGraphContext& context, const Settings& settings);

		DebugFrameGraphPass& operator=(const DebugFrameGraphPass& other) = default;
		DebugFrameGraphPass& operator=(DebugFrameGraphPass&& other) noexcept = default;

	protected:
		void doBeginFrame() override;
		void doEndFrame() override;

	private:
		Ptr<DescriptorSetLayout> m_setLayout;
		Ptr<GraphicsPipeline> m_pipeline;
		Ptr<Sampler> m_sampler;
		Ptr<VertexBuffer> m_quadMesh;
	};
}

#endif