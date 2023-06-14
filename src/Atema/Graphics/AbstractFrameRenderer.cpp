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

#include <Atema/Graphics/AbstractFrameRenderer.hpp>
#include <Atema/Graphics/FrameGraph.hpp>
#include <Atema/Renderer/Renderer.hpp>

using namespace at;

AbstractFrameRenderer::AbstractFrameRenderer() :
	m_updateFrameGraph(true)
{
}

RenderData& AbstractFrameRenderer::getRenderData() noexcept
{
	return m_renderData;
}

const RenderData& AbstractFrameRenderer::getRenderData() const noexcept
{
	return m_renderData;
}

void AbstractFrameRenderer::beginFrame()
{
	if (m_updateFrameGraph)
	{
		Renderer::instance().waitForIdle();
		
		createFrameGraph();

		m_updateFrameGraph = false;
	}
	
	for (auto& renderPass : getRenderPasses())
		renderPass->beginFrame(m_renderData);
}

void AbstractFrameRenderer::render(RenderFrame& renderFrame)
{
	// Transfer data from CPU to GPU if needed
	CommandBuffer::Settings commandBufferSettings;
	commandBufferSettings.secondary = false;
	commandBufferSettings.singleUse = true;
	
	auto commandBuffer = renderFrame.createCommandBuffer(commandBufferSettings, QueueType::Graphics);

	commandBuffer->begin();

	commandBuffer->memoryBarrier(MemoryBarrier::TransferBegin);

	for (auto& renderable : m_renderData.getRenderables())
		renderable->updateResources(renderFrame, *commandBuffer);

	commandBuffer->memoryBarrier(MemoryBarrier::TransferEnd);

	commandBuffer->end();

	renderFrame.submit({ commandBuffer }, {}, {});

	renderFrame.destroyAfterUse(std::move(commandBuffer));

	destroyResources(renderFrame);

	// Execute FrameGraph
	getFrameGraph().execute(renderFrame);

	// End frame
	for (auto& renderPass : getRenderPasses())
		renderPass->endFrame();
}

void AbstractFrameRenderer::resize(const Vector2u& size)
{
	m_size = size;

	m_updateFrameGraph = true;
}

Vector2u AbstractFrameRenderer::getSize() const noexcept
{
	return m_size;
}

void AbstractFrameRenderer::destroyResources(RenderFrame& renderFrame)
{
}
