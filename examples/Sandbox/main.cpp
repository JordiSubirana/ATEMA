#include <Atema/Atema.hpp>

#include "BasicRenderPipeline.hpp"

#include <iostream>

using namespace at;

class TestLayer : public ApplicationLayer
{
public:
	TestLayer()
	{
		initialize();
	}

	~TestLayer()
	{
		renderPipeline.reset();

		// Window & Renderer
		window.reset();
		
		Renderer::destroy();
	}

	void initialize()
	{
		maxFramesInFlight = 2;
		currentFrame = 0;
		indexCount = 0;

		Renderer::Settings settings;
		settings.maxFramesInFlight = maxFramesInFlight;
		//settings.mainWindowSettings.width = 1920;
		//settings.mainWindowSettings.height = 1080;

		Renderer::create<VulkanRenderer>(settings);

		// Window / SwapChain
		window = Renderer::getInstance().getMainWindow();

		RenderPipeline::Settings renderPipelineSettings;
		renderPipelineSettings.window = window;

		renderPipeline = std::make_shared<BasicRenderPipeline>(renderPipelineSettings);
	}

	void onEvent(Event& event) override
	{
		
	}
	
	void update(TimeStep ms) override
	{
		totalTime += ms.getSeconds();

		if (window->shouldClose())
		{
			Application::instance().close();
			return;
		}

		window->processEvents();
		
		renderPipeline->update(ms);
		
		window->swapBuffers();
	}

	uint32_t maxFramesInFlight;
	uint32_t currentFrame;
	Ptr<Window> window;
	Ptr<SwapChain> swapChain;
	Ptr<RenderPass> renderPass;
	Ptr<Image> depthImage;
	std::vector<Ptr<Framebuffer>> framebuffers;
	Ptr<DescriptorSetLayout> descriptorSetLayout;
	Ptr<GraphicsPipeline> pipeline;
	std::vector<Ptr<CommandPool>> commandPools;
	std::vector<Ptr<CommandBuffer>> commandBuffers;
	std::vector<Ptr<Fence>> fences;
	std::vector<Ptr<Fence>> imageFences;
	std::vector<Ptr<Semaphore>> imageAvailableSemaphores;
	std::vector<Ptr<Semaphore>> renderFinishedSemaphores;
	Ptr<DescriptorPool> descriptorPool;

	Ptr<Buffer> vertexBuffer;
	Ptr<Buffer> indexBuffer;
	Ptr<Image> texture;
	Ptr<Sampler> sampler;
	std::vector<Ptr<Buffer>> uniformBuffers;
	std::vector<Ptr<DescriptorSet>> descriptorSets;

	uint32_t indexCount;

	float totalTime;

	Ptr<BasicRenderPipeline> renderPipeline;

};

void basicApplication()
{
	auto layer = new TestLayer();

	auto& app = Application::instance();

	app.addLayer(layer);

	app.run();

	delete layer;
}

// MAIN
int main(int argc, char** argv)
{
	try
	{
		basicApplication();
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return -1;
	}
	
	return 0;
}