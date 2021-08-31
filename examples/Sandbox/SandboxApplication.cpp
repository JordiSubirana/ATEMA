/*
	Copyright 2021 Jordi SUBIRANA

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

#include "SandboxApplication.hpp"

using namespace at;


SandboxApplication::SandboxApplication():
	Application(),
	m_frameCount(0),
	m_frameDuration(0.0f)
{
	initialize();
}

SandboxApplication::~SandboxApplication()
{
	m_renderPipeline.reset();

	// Window & Renderer
	m_window.reset();

	Renderer::destroy();
}

void SandboxApplication::initialize()
{
	Renderer::Settings settings;
	settings.maxFramesInFlight = 2;
	//settings.mainWindowSettings.width = 1920;
	//settings.mainWindowSettings.height = 1080;

	Renderer::create<VulkanRenderer>(settings);

	// Window / SwapChain
	m_window = Renderer::getInstance().getMainWindow();

	RenderPipeline::Settings renderPipelineSettings;
	renderPipelineSettings.window = m_window;

	m_renderPipeline = std::make_shared<TestRenderPipeline>(renderPipelineSettings);
}

void SandboxApplication::onEvent(at::Event& event)
{
}

void SandboxApplication::update(at::TimeStep ms)
{
	ATEMA_BENCHMARK("Application update")

	m_frameDuration += ms.getSeconds();

	if (m_window->shouldClose())
	{
		Application::instance().close();
		return;
	}

	m_window->processEvents();

	m_renderPipeline->update(ms);

	m_window->swapBuffers();

	m_frameCount++;

	if (m_frameDuration >= 0.5f)
	{
		const auto frameTime = m_frameDuration / static_cast<float>(m_frameCount);
		const auto fps = static_cast<unsigned>(1.0f / frameTime);

		m_window->setTitle("Atema (" + std::to_string(fps) + " fps / " + std::to_string(frameTime * 1000.0f) + " ms)");

		BenchmarkManager::getInstance().print(m_frameCount);
		BenchmarkManager::getInstance().reset();

		m_frameCount = 0;
		m_frameDuration = 0.0f;
	}
}
