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

#include <Atema/Core/Benchmark.hpp>
#include <Atema/Graphics/AbstractFrameRenderer.hpp>
#include <Atema/Graphics/FrameGraph.hpp>
#include <Atema/Renderer/Renderer.hpp>

using namespace at;

AbstractFrameRenderer::AbstractFrameRenderer() :
	m_renderScene(m_resourceManager, *this),
	m_updateFrameGraph(true)
{
}

AbstractFrameRenderer::~AbstractFrameRenderer()
{
	m_renderScene.clear();
}

RenderScene& AbstractFrameRenderer::getRenderScene() noexcept
{
	return m_renderScene;
}

const RenderScene& AbstractFrameRenderer::getRenderScene() const noexcept
{
	return m_renderScene;
}

void AbstractFrameRenderer::initializeFrame()
{
	ATEMA_BENCHMARK_TAG(_1, "Initialize frame");

	if (m_updateFrameGraph)
	{
		ATEMA_BENCHMARK_TAG(_2, "Update frame graph");

		createFrameGraph();

		m_updateFrameGraph = false;
	}

	{
		ATEMA_BENCHMARK_TAG(_2, "Begin frame");

		beginFrame();
	}

	for (auto& renderPass : getRenderPasses())
	{
		ATEMA_BENCHMARK_TAG(_2, std::string(renderPass->getName()) + " (begin)");

		renderPass->initializeFrame(m_renderScene);
	}
}

void AbstractFrameRenderer::render(CommandBuffer& commandBuffer, RenderContext& renderContext, RenderFrame* renderFrame)
{
	{
		ATEMA_BENCHMARK("Transfer data");

		commandBuffer.memoryBarrier(MemoryBarrier::TransferBegin);

		m_resourceManager.beginTransfer(commandBuffer, renderContext);

		{
			ATEMA_BENCHMARK_TAG(b, "Scene");

			m_renderScene.update();
		}

		{
			ATEMA_BENCHMARK_TAG(b, "RenderPasses");

			for (auto& renderPass : getRenderPasses())
				renderPass->updateResources(commandBuffer);
		}

		{
			ATEMA_BENCHMARK_TAG(b, "Deferred update");

			m_resourceManager.endTransfer();
		}

		commandBuffer.memoryBarrier(MemoryBarrier::TransferEnd);

		{
			ATEMA_BENCHMARK_TAG(b, "Destroy resources");

			destroyResources(renderContext);
		}
	}

	// Execute FrameGraph if it is valid
	if (getFrameGraph())
	{
		ATEMA_BENCHMARK("Execute FrameGraph");

		getFrameGraph()->execute(commandBuffer, renderContext, renderFrame);
	}

	// End frame
	for (auto& renderPass : getRenderPasses())
		renderPass->finalizeFrame();
}

void AbstractFrameRenderer::resize(const Vector2u& size)
{
	m_size = size;

	m_updateFrameGraph = true;
}

void AbstractFrameRenderer::updateFrameGraph()
{
	m_updateFrameGraph = true;
}

Vector2u AbstractFrameRenderer::getSize() const noexcept
{
	return m_size;
}

void AbstractFrameRenderer::destroyResources(RenderContext& renderContext)
{
}

void AbstractFrameRenderer::beginFrame()
{
}
