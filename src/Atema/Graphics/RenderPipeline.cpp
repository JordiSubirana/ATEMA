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
#include <Atema/Graphics/RenderPipeline.hpp>
#include <Atema/Window/Window.hpp>
#include <Atema/Renderer/CommandPool.hpp>
#include <Atema/Renderer/Fence.hpp>
#include <Atema/Renderer/Framebuffer.hpp>
#include <Atema/Renderer/Image.hpp>
#include <Atema/Renderer/Renderer.hpp>
#include <Atema/Renderer/RenderPass.hpp>
#include <Atema/Renderer/Semaphore.hpp>
#include <Atema/Renderer/SwapChain.hpp>

using namespace at;

RenderPipeline::RenderPipeline(const Settings& settings) :
	NonCopyable(),
	m_window(settings.window),
	m_maxFramesInFlight(settings.maxFramesInFlight),
	m_colorFormat(settings.colorFormat),
	m_depthFormat(settings.depthFormat),
	m_currentFrame(0),
	m_currentSwapChainImage(0)
{
	ATEMA_ASSERT(settings.resizeCallback, "Invalid resize callback");
	ATEMA_ASSERT(settings.updateFrameCallback, "Invalid update frame callback");

	// RenderPass
	RenderPass::Settings renderPassSettings;
	renderPassSettings.attachments.resize(2);
	renderPassSettings.attachments[0].format = m_colorFormat;
	renderPassSettings.attachments[0].finalLayout = ImageLayout::Present;
	renderPassSettings.attachments[1].format = m_depthFormat;

	renderPassSettings.subpasses.resize(1);
	renderPassSettings.subpasses[0].color = { 0 };
	renderPassSettings.subpasses[0].depthStencil = 1;

	m_renderPass = RenderPass::create(renderPassSettings);

	// Frame resources (per thread)
	for (uint32_t i = 0; i < m_maxFramesInFlight; i++)
	{
		// Command pools
		m_commandPools.push_back(CommandPool::create({}));
		
		// Fences
		m_fences.push_back(Fence::create({ true }));
		
		// Semaphores
		m_imageAvailableSemaphores.push_back(Semaphore::create());
		m_renderFinishedSemaphores.push_back(Semaphore::create());
	}

	// Command buffers
	m_commandBuffers.resize(m_maxFramesInFlight);

	createSwapChainResources();

	// Set callbacks once everything is initialized
	m_resizeCallback = settings.resizeCallback;
	m_updateFrameCallback = settings.updateFrameCallback;
}

RenderPipeline::~RenderPipeline()
{
	// SwapChain resources
	destroySwapChainResources();

	m_renderFinishedSemaphores.clear();

	m_imageAvailableSemaphores.clear();

	m_imageFences.clear();

	m_fences.clear();

	m_commandBuffers.clear();

	m_commandPools.clear();

	m_renderPass.reset();
}

void RenderPipeline::startFrame()
{
	ATEMA_BENCHMARK_TAG(rootBenchmark, "RenderPipeline::update")

	m_currentCommandBuffer.reset();

	// Wait on fence to be signaled (max frames in flight)
	auto& fence = m_fences[m_currentFrame];

	{
		ATEMA_BENCHMARK("Fence::wait")
		
		fence->wait();
	}

	// Acquire next available swapchain image
	auto& imageAvailableSemaphore = m_imageAvailableSemaphores[m_currentFrame];

	SwapChainResult acquireResult = m_swapChain->acquireNextImage(m_currentSwapChainImage, imageAvailableSemaphore);

	if (acquireResult == SwapChainResult::OutOfDate ||
		acquireResult == SwapChainResult::Suboptimal)
	{
		destroySwapChainResources();
		createSwapChainResources();
		return;
	}
	else if (acquireResult != SwapChainResult::Success)
	{
		ATEMA_ERROR("Failed to acquire a valid swapchain image");
	}

	// Check if a previous frame is using this image (i.e. there is its fence to wait on)
	if (m_imageFences[m_currentSwapChainImage])
	{
		m_imageFences[m_currentSwapChainImage]->wait();
	}

	// Mark the image as now being in use by this frame
	m_imageFences[m_currentSwapChainImage] = fence;

	// Update frame data if needed

	// Prepare command buffer
	const auto& commandPool = m_commandPools[m_currentFrame];

	m_currentCommandBuffer = commandPool->createBuffer({ true });

	m_currentCommandBuffer->begin();

	{
		ATEMA_BENCHMARK("RenderPipeline::updateFrame")

		m_updateFrameCallback(m_currentFrame, m_currentCommandBuffer);
	}

	m_currentCommandBuffer->end();

	// Submit command buffer
	auto& renderFinishedSemaphore = m_renderFinishedSemaphores[m_currentFrame];

	const std::vector<Ptr<CommandBuffer>> submitCommandBuffers = { m_currentCommandBuffer };
	const std::vector<Ptr<Semaphore>> submitWaitSemaphores = { imageAvailableSemaphore };
	const std::vector<Flags<PipelineStage>> submitWaitStages = { PipelineStage::ColorAttachmentOutput };
	const std::vector<Ptr<Semaphore>> submitSignalSemaphores = { renderFinishedSemaphore };

	// Reset fence & submit command buffers to the target queue (works with arrays for performance)
	fence->reset();

	Renderer::instance().submit(
		submitCommandBuffers,
		submitWaitSemaphores,
		submitWaitStages,
		submitSignalSemaphores,
		fence);
	
	{
		ATEMA_BENCHMARK("Renderer::present")

		// Present swapchain image
		acquireResult = Renderer::instance().present(
			m_swapChain,
			m_currentSwapChainImage,
			submitSignalSemaphores
		);
	}
	
	// Save command buffer
	m_commandBuffers[m_currentFrame] = m_currentCommandBuffer;

	m_currentCommandBuffer.reset();

	// Advance frame
	m_currentFrame = (m_currentFrame + 1) % m_maxFramesInFlight;

	if (acquireResult == SwapChainResult::OutOfDate ||
		acquireResult == SwapChainResult::Suboptimal)
	{
		destroySwapChainResources();
		createSwapChainResources();
	}
	else if (acquireResult != SwapChainResult::Success)
	{
		ATEMA_ERROR("SwapChain presentation failed");
	}
}

Ptr<Window> RenderPipeline::getWindow() const noexcept
{
	return m_window.lock();
}

const Ptr<SwapChain>& RenderPipeline::getSwapChain() const noexcept
{
	return m_swapChain;
}

const Ptr<RenderPass>& RenderPipeline::getRenderPass() const noexcept
{
	return m_renderPass;
}

const Ptr<Framebuffer>& RenderPipeline::getCurrentFramebuffer() const noexcept
{
	return m_framebuffers[m_currentSwapChainImage];
}

const std::vector<Ptr<CommandPool>>& RenderPipeline::getCommandPools() const noexcept
{
	return m_commandPools;
}

void RenderPipeline::beginScreenRenderPass(bool useSecondaryBuffers)
{
	static const std::vector<CommandBuffer::ClearValue> clearValues =
	{
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ 1.0f, 0 }
	};

	const auto& framebuffer = m_framebuffers[m_currentSwapChainImage];
	
	m_currentCommandBuffer->beginRenderPass(m_renderPass, framebuffer, clearValues, useSecondaryBuffers);
}

void RenderPipeline::endScreenRenderPass()
{
	m_currentCommandBuffer->endRenderPass();
}

void RenderPipeline::createSwapChainResources()
{
	const auto window = m_window.lock();
	
	const auto windowSize = window->getSize();

	m_swapChain = SwapChain::create({ window, m_colorFormat });

	const auto swapChainImageCount = m_swapChain->getImages().size();

	// Depth image
	Image::Settings depthSettings;
	depthSettings.width = windowSize.x;
	depthSettings.height = windowSize.y;
	depthSettings.format = m_depthFormat;
	depthSettings.usages = ImageUsage::RenderTarget;

	m_depthImage = Image::create(depthSettings);

	// Framebuffers (one per swapchain image)
	Framebuffer::Settings framebufferSettings;
	framebufferSettings.renderPass = m_renderPass;
	framebufferSettings.width = windowSize.x;
	framebufferSettings.height = windowSize.y;

	for (auto& image : m_swapChain->getImages())
	{
		framebufferSettings.images =
		{
			image,
			m_depthImage
		};

		m_framebuffers.push_back(Framebuffer::create(framebufferSettings));
	}

	m_imageFences.resize(swapChainImageCount);

	if (m_resizeCallback)
		m_resizeCallback(windowSize);
}

void RenderPipeline::destroySwapChainResources()
{
	// Resources may be in use, wait until it's not the case anymore
	Renderer::instance().waitForIdle();

	m_framebuffers.clear();

	m_depthImage.reset();

	m_swapChain.reset();
}
