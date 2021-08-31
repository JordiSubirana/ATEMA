#include <Atema/Atema.hpp>

#include "BasicRenderPipeline.hpp"
#include "DeferredRenderPipeline.hpp"

#include <iostream>

//using TestRenderPipeline = BasicRenderPipeline;
using TestRenderPipeline = DeferredRenderPipeline;

using namespace at;

class SandboxLayer : public ApplicationLayer
{
public:
	SandboxLayer() :
		frameCount(0),
		frameDuration(0.0f)
	{
		initialize();
	}

	~SandboxLayer()
	{
		renderPipeline.reset();

		// Window & Renderer
		window.reset();
		
		Renderer::destroy();
	}

	void initialize()
	{
		Renderer::Settings settings;
		settings.maxFramesInFlight = 2;
		//settings.mainWindowSettings.width = 1920;
		//settings.mainWindowSettings.height = 1080;

		Renderer::create<VulkanRenderer>(settings);

		// Window / SwapChain
		window = Renderer::getInstance().getMainWindow();

		RenderPipeline::Settings renderPipelineSettings;
		renderPipelineSettings.window = window;

		renderPipeline = std::make_shared<TestRenderPipeline>(renderPipelineSettings);
	}

	void onEvent(Event& event) override
	{
		
	}
	
	void update(TimeStep ms) override
	{
		ATEMA_BENCHMARK("Application update")
		
		frameDuration += ms.getSeconds();

		if (window->shouldClose())
		{
			Application::instance().close();
			return;
		}

		window->processEvents();
		
		renderPipeline->update(ms);
		
		window->swapBuffers();

		frameCount++;

		if (frameDuration >= 0.5f)
		{
			const auto frameTime = frameDuration / static_cast<float>(frameCount);
			const auto fps = static_cast<unsigned>(1.0f / frameTime);

			window->setTitle("Atema (" + std::to_string(fps) + " fps / " + std::to_string(frameTime * 1000.0f) + " ms)");

			BenchmarkManager::getInstance().print(frameCount);
			BenchmarkManager::getInstance().reset();

			frameCount = 0;
			frameDuration = 0.0f;
		}
	}

	Ptr<Window> window;

	Ptr<TestRenderPipeline> renderPipeline;

	int frameCount;
	float frameDuration;
};

// MAIN
int main(int argc, char** argv)
{
	try
	{
		auto sandboxLayer = std::make_unique<SandboxLayer>();

		auto& app = Application::instance();

		app.addLayer(sandboxLayer.get());

		app.run();
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		
		return -1;
	}
	
	return 0;
}