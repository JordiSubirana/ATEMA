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

#ifndef ATEMA_GRAPHICS_ABSTRACTFRAMERENDERER_HPP
#define ATEMA_GRAPHICS_ABSTRACTFRAMERENDERER_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/RenderData.hpp>
#include <Atema/Graphics/AbstractRenderPass.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Math/Vector.hpp>

#include <vector>

namespace at
{
	class FrameGraph;
	class RenderFrame;
	
	class ATEMA_GRAPHICS_API AbstractFrameRenderer
	{
	public:
		AbstractFrameRenderer();
		AbstractFrameRenderer(const AbstractFrameRenderer& other) = delete;
		AbstractFrameRenderer(AbstractFrameRenderer&& other) noexcept = default;
		virtual ~AbstractFrameRenderer() = default;

		RenderData& getRenderData() noexcept;
		const RenderData& getRenderData() const noexcept;

		// Must be called every frame before render method
		void beginFrame();

		// Renders the current frame
		// beginFrame must have been called before that
		// RenderPass::endFrame will be called in this method for every pass
		void render(RenderFrame& renderFrame);

		void resize(const Vector2u& size);

		void updateFrameGraph();

		Vector2u getSize() const noexcept;

		AbstractFrameRenderer& operator=(const AbstractFrameRenderer& other) = delete;
		AbstractFrameRenderer& operator=(AbstractFrameRenderer&& other) noexcept = default;

	protected:
		virtual void createFrameGraph() = 0;
		virtual FrameGraph& getFrameGraph() = 0;
		virtual std::vector<UPtr<AbstractRenderPass>>& getRenderPasses() = 0;
		virtual void destroyResources(RenderFrame& renderFrame);
		// Called before every AbstractRenderPass::doBeginFrame
		virtual void doBeginFrame();

	private:
		RenderData m_renderData;

		// FrameGraph
		bool m_updateFrameGraph;
		Vector2u m_size;
	};
}

#endif