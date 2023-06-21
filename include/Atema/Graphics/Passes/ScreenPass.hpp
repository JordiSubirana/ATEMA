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

#ifndef ATEMA_GRAPHICS_SCREENPASS_HPP
#define ATEMA_GRAPHICS_SCREENPASS_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/FrameGraphTexture.hpp>
#include <Atema/Graphics/AbstractRenderPass.hpp>

namespace at
{
	class Camera;
	class Renderable;
	class FrameGraphBuilder;
	class DescriptorSetLayout;
	class Sampler;
	class VertexBuffer;

	class ATEMA_GRAPHICS_API ScreenPass : public AbstractRenderPass
	{
	public:
		struct Settings
		{
			// Input texture
			FrameGraphTextureHandle input = FrameGraph::InvalidTextureHandle;
		};

		ScreenPass();
		ScreenPass(const ScreenPass& other) = default;
		ScreenPass(ScreenPass&& other) noexcept = default;
		~ScreenPass() = default;

		const char* getName() const noexcept override;

		// Default : true
		void showUI(bool show);

		FrameGraphPass& addToFrameGraph(FrameGraphBuilder& frameGraphBuilder, const Settings& settings);

		void execute(FrameGraphContext& context, const Settings& settings);

		ScreenPass& operator=(const ScreenPass& other) = default;
		ScreenPass& operator=(ScreenPass&& other) noexcept = default;

	protected:
		void doBeginFrame() override;
		void doEndFrame() override;

	private:
		Ptr<DescriptorSetLayout> m_setLayout;
		Ptr<GraphicsPipeline> m_pipeline;
		Ptr<Sampler> m_sampler;
		Ptr<VertexBuffer> m_quadMesh;
		bool m_showUI;
	};
}

#endif