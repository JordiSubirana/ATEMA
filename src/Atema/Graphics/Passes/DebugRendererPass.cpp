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

#include <Atema/Graphics/DebugRenderer.hpp>
#include <Atema/Graphics/Passes/DebugRendererPass.hpp>
#include <Atema/Graphics/FrameGraphBuilder.hpp>
#include <Atema/Graphics/FrameGraphContext.hpp>
#include <Atema/Graphics/RenderData.hpp>

using namespace at;

DebugRendererPass::DebugRendererPass() :
	AbstractRenderPass()
{
	m_debugRenderer = std::make_shared<DebugRenderer>();
}

const char* DebugRendererPass::getName() const noexcept
{
	return "Debug Renderer";
}

FrameGraphPass& DebugRendererPass::addToFrameGraph(FrameGraphBuilder& frameGraphBuilder, const Settings& settings)
{
	auto& pass = frameGraphBuilder.createPass(getName());

	if (settings.outputClearValue.has_value())
		pass.addOutputTexture(settings.output, 0, settings.outputClearValue.value());
	else
		pass.addOutputTexture(settings.output, 0);

	if (settings.depthStencilClearValue.has_value())
		pass.setDepthTexture(settings.depthStencil, settings.depthStencilClearValue.value());
	else
		pass.setDepthTexture(settings.depthStencil);

	Settings settingsCopy = settings;

	pass.setExecutionCallback([this, settingsCopy](FrameGraphContext& context)
		{
			execute(context, settingsCopy);
		});

	return pass;
}

void DebugRendererPass::execute(FrameGraphContext& context, const Settings& settings)
{
	const auto& renderData = getRenderData();

	if (!renderData.isValid())
		return;

	const auto& viewport = getRenderData().getCamera().getViewport();
	const auto& scissor = getRenderData().getCamera().getScissor();

	auto& commandBuffer = context.getCommandBuffer();

	commandBuffer.setViewport(viewport);

	commandBuffer.setScissor(scissor.pos, scissor.size);

	m_debugRenderer->render(context, commandBuffer, getRenderData().getCamera().getMatrix());
}

void DebugRendererPass::doBeginFrame()
{
	const auto& renderData = getRenderData();

	if (!renderData.isValid())
		return;

	m_debugRenderer->clear();

	std::vector<RenderElement> renderElements;

	for (auto& renderable : getRenderData().getRenderables())
	{
		const auto& aabb = renderable->getAABB();
		const auto& matrix = renderable->getTransform().getMatrix();

		// Axis Aligned Bounding Box
		m_debugRenderer->draw(aabb, Color::Green);

		// Submeshes' AABBs
		/*
		{
			renderElements.clear();
			renderElements.reserve(renderable->getRenderElementsSize());

			renderable->getRenderElements(renderElements);

			for (auto& renderElement : renderElements)
				m_debugRenderer->draw(matrix * renderElement.aabb, Color::Gray);
		}
		//*/
	}
}

void DebugRendererPass::doEndFrame()
{
}
