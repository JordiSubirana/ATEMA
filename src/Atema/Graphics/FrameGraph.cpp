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

#include <Atema/Core/Benchmark.hpp>
#include <Atema/Graphics/FrameGraph.hpp>
#include <Atema/Graphics/FrameGraphContext.hpp>
#include <Atema/Renderer/RenderFrame.hpp>

using namespace at;

FrameGraph::FrameGraph()
{
}

FrameGraph::~FrameGraph()
{
}

void FrameGraph::execute(RenderFrame& renderFrame)
{
	CommandBuffer::Settings commandBufferSettings;
	commandBufferSettings.secondary = false;
	commandBufferSettings.singleUse = true;

	auto commandBuffer = renderFrame.createCommandBuffer(commandBufferSettings, QueueType::Graphics);

	commandBuffer->begin();

	size_t passIndex = 0;
	for (auto& pass : m_passes)
	{
		FrameGraphContext context(renderFrame, *commandBuffer, pass.textures, pass.views, pass.renderPass, pass.framebuffer);

		if (pass.useRenderFrameOutput)
			commandBuffer->beginRenderPass(*renderFrame.getRenderPass(), *renderFrame.getFramebuffer(), pass.clearValues, pass.useSecondaryCommandBuffers);
		else
			commandBuffer->beginRenderPass(*pass.renderPass, *pass.framebuffer, pass.clearValues, pass.useSecondaryCommandBuffers);

		pass.executionCallback(context);

		commandBuffer->endRenderPass();

		for (const auto& texture : m_textures)
		{
			const auto& barrier = texture.barriers[passIndex];

			if (barrier.valid)
			{
				commandBuffer->imageBarrier(
					*texture.image,
					barrier.srcPipelineStages, barrier.dstPipelineStages,
					barrier.srcMemoryAccesses, barrier.dstMemoryAccesses,
					barrier.srcLayout, barrier.dstLayout);
			}
		}

		passIndex++;
	}

	commandBuffer->end();
	
	renderFrame.getFence()->reset();

	{
		ATEMA_BENCHMARK("RenderFrame::submit");

		renderFrame.submit(
			{ commandBuffer },
			{ renderFrame.getImageAvailableWaitCondition() },
			{ renderFrame.getRenderFinishedSemaphore() },
			renderFrame.getFence());
	}

	{
		ATEMA_BENCHMARK("RenderFrame::present");

		renderFrame.present();
	}

	renderFrame.destroyAfterUse(std::move(commandBuffer));
}
