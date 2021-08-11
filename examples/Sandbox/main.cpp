#include <Atema/Atema.hpp>

#include <entt/entt.hpp>

#include <map>
#include <iostream>

using namespace at;

const std::filesystem::path rsc_path = "../../examples/Sandbox/Resources/";

// #1 - BENCHMARK SPARSE SET
void benchmarkSparseSet()
{
	std::unordered_map<int, std::string> map;
	SparseSet<std::string> set;

	std::vector<int> dataInt;
	std::vector<std::string> dataStr;

	//srand(time(NULL));

	auto count = 1'000'000;
	auto maxValue = 1'000'000;
	dataInt.resize(count);
	dataStr.resize(count);
	for (int i = 0; i < count; i++)
	{
		dataInt[i] = (rand() * rand() + rand()) % maxValue;
		dataStr[i] = std::to_string(dataInt[i]);

		//std::cout << dataStr[i] << "\n";
	}

	std::cout << "Insertion\n";

	{
		ScopedTimer timer;

		for (int i = 0; i < count; i++)
		{
			map[dataInt[i]] = dataStr[i];
		}
	}

	{
		ScopedTimer timer;

		for (int i = 0; i < count; i++)
		{
			set.insert(dataInt[i], dataStr[i]);
		}
	}

	std::cout << "Traversal\n";

	{
		ScopedTimer timer;

		int total = 0;

		for (auto& v : map)
		{
			total += v.second.size();
		}

		std::cout << total << "\n";
	}

	{
		ScopedTimer timer;

		int total = 0;

		for (auto& v : set)
		{
			total += v.size();
		}

		std::cout << total << "\n";
	}

	std::cout << "Deletion\n";

	{
		ScopedTimer timer;

		for (int i = 0; i < count; i++)
		{
			map.erase(dataInt[i]);
		}
	}

	{
		ScopedTimer timer;

		for (int i = 0; i < count; i++)
		{
			set.erase(dataInt[i]);
		}
	}
}

// ECS DEFINITION
struct Position
{
	Vector3f value;
};

struct Velocity
{
	Vector3f value;
};

struct Name
{
	std::string value;
};

// #2 - ENTITY MANAGER
void entityManager()
{
	EntityManager mgr;

	auto e1 = mgr.createEntity();
	auto& n1 = mgr.createComponent<Name>(e1);
	auto& p1 = mgr.createComponent<Position>(e1);
	auto& v1 = mgr.createComponent<Velocity>(e1);

	auto e2 = mgr.createEntity();
	auto& n2 = mgr.createComponent<Name>(e2);
	auto& p2 = mgr.createComponent<Position>(e2);

	auto e3 = mgr.createEntity();
	auto& n3 = mgr.createComponent<Name>(e3);
	auto& v3 = mgr.createComponent<Velocity>(e3);

	auto u = mgr.getUnion<Name, Position>();
}

// #3 - BENCHMARK SPARSE SET UNION
void benchmarkSparseSetUnion()
{
	std::vector<int> commonData;

	int c = 1000000;
	for (int i = 0; i < c; i++)
	{
		commonData.push_back(i);
	}

	SparseSet<int> setInt;
	SparseSet<std::string> setStr;
	SparseSet<float> setFloat;

	entt::registry _reg;


	for (auto& i : commonData)
	{
		auto e = _reg.create();

		if (i > c / 4)
		{
			setInt.insert(i, i);
			_reg.emplace<int>(e, i);
		}

		if (i < 3 * (c / 4))
		{
			auto str = std::to_string(i);
			setStr.insert(i, str);
			_reg.emplace<std::string>(e, str);
		}

		//setInt.insert(i, i);
		//setStr.insert(i, std::to_string(i));
		setFloat.insert(i, (float)i);

		//_reg.emplace<int>(e, i);
		//_reg.emplace<std::string>(e, std::to_string(i));
		_reg.emplace<float>(e, (float)i);
	}

	std::cout << c << " entities\n";
	
	std::cout << "\nSparseSetUnion : ";
	{
		ScopedTimer timer;

		SparseSetUnion<int, std::string, float> view(setInt, setStr, setFloat);

		int s = 0;
		int total = 0;

		for (auto& id : view)
		{
			total += view.get<int>(id);
			s++;
		}

		std::cout << total << " - " << view.size() << " (" << s << " entities)" << std::endl;
	}

	std::cout << "\nenTT view (group not built): ";
	{
		ScopedTimer timer;

		auto view = _reg.view<int, std::string, float>();

		int s = 0;
		int total = 0;

		for (auto& id : view)
		{
			total += view.get<int>(id);
			s++;
		}

		std::cout << total << " - " << view.size_hint() << " (" << s << " entities)" << std::endl;
	}

	std::cout << "\nenTT group (group not built) : ";
	{
		ScopedTimer timer;

		auto group = _reg.group<int, std::string, float>();

		int s = 0;
		int total = 0;

		for (auto& id : group)
		{
			total += group.get<int>(id);
			s++;
		}

		std::cout << total << " - " << group.size() << " (" << s << " entities)" << std::endl;
	}

	std::cout << "\nenTT view (group built): ";
	{
		ScopedTimer timer;

		auto view = _reg.view<int, std::string, float>();

		int s = 0;
		int total = 0;

		for (auto& id : view)
		{
			total += view.get<int>(id);
			s++;
		}

		std::cout << total << " - " << view.size_hint() << " (" << s << " entities)" << std::endl;
	}
	
	std::cout << "\nenTT group (group built) : ";
	{
		ScopedTimer timer;

		auto group = _reg.group<int, std::string, float>();

		int s = 0;
		int total = 0;

		for (auto& id : group)
		{
			total += group.get<int>(id);
			s++;
		}

		std::cout << total << " - " << group.size() << " (" << s << " entities)" << std::endl;
	}
}

// Application

// #4 - Basic application
class TestLayer : public ApplicationLayer
{
public:
	TestLayer()
	{
		initialize();
	}

	~TestLayer()
	{
		Renderer::getInstance().waitForIdle();

		renderFinishedSemaphores.clear();

		imageAvailableSemaphores.clear();

		imageFences.clear();
		
		fences.clear();
		
		commandBuffers.clear();

		commandPools.clear();

		pipeline.reset();

		descriptorSetLayout.reset();

		framebuffers.clear();

		depthImage.reset();

		renderPass.reset();

		swapChain.reset();

		window.reset();
		
		Renderer::destroy();
	}

	void initialize()
	{
		maxFramesInFlight = 2;
		currentFrame = 0;

		Renderer::Settings settings;
		settings.maxFramesInFlight = maxFramesInFlight;
		//settings.mainWindowSettings.width = 1920;
		//settings.mainWindowSettings.height = 1080;

		Renderer::create<VulkanRenderer>(settings);

		// Window / SwapChain
		window = Renderer::getInstance().getMainWindow();

		auto windowSize = window->getSize();

		swapChain = SwapChain::create({ window, ImageFormat::BGRA8_SRGB });

		auto swapChainImageCount = swapChain->getImages().size();
		
		// RenderPass
		RenderPass::Settings renderPassSettings;
		renderPassSettings.attachments.resize(2);
		renderPassSettings.attachments[0].format = ImageFormat::BGRA8_SRGB;
		renderPassSettings.attachments[0].finalLayout = ImageLayout::Present;
		renderPassSettings.attachments[1].format = ImageFormat::D32F;

		renderPass = RenderPass::create(renderPassSettings);

		// Depth image
		Image::Settings depthSettings;
		depthSettings.width = windowSize.x;
		depthSettings.height = windowSize.y;
		depthSettings.format = ImageFormat::D32F;
		depthSettings.usages = ImageUsage::RenderTarget;

		depthImage = Image::create(depthSettings);

		// Framebuffers (one per swapchain image)
		Framebuffer::Settings framebufferSettings;
		framebufferSettings.renderPass = renderPass;
		framebufferSettings.width = windowSize.x;
		framebufferSettings.height = windowSize.y;

		for (auto& image : swapChain->getImages())
		{
			framebufferSettings.images =
			{
				image,
				depthImage
			};

			framebuffers.push_back(Framebuffer::create(framebufferSettings));
		}

		// Descriptor set layout
		DescriptorSetLayout::Settings descriptorSetLayoutSettings;
		descriptorSetLayoutSettings.bindings =
		{
			{ DescriptorType::UniformBuffer, 0, 1, ShaderStage::Vertex },
			{ DescriptorType::CombinedImageSampler, 1, 1, ShaderStage::Fragment }
		};

		descriptorSetLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);

		// Graphics pipeline
		GraphicsPipeline::Settings pipelineSettings;
		pipelineSettings.viewport.size.x = static_cast<float>(windowSize.x);
		pipelineSettings.viewport.size.y = static_cast<float>(windowSize.y);
		pipelineSettings.scissor.size = windowSize;
		pipelineSettings.vertexShader = Shader::create({ rsc_path / "Shaders/vert.spv" });
		pipelineSettings.fragmentShader = Shader::create({ rsc_path / "Shaders/frag.spv" });
		pipelineSettings.renderPass = renderPass;
		pipelineSettings.descriptorSetLayout = descriptorSetLayout;
		pipelineSettings.vertexInput.attributes =
		{
			{ VertexAttribute::Role::Position, VertexAttribute::Format::RGB32_SFLOAT },
			{ VertexAttribute::Role::Color, VertexAttribute::Format::RGB32_SFLOAT },
			{ VertexAttribute::Role::Texture, VertexAttribute::Format::RG32_SFLOAT }
		};
		pipelineSettings.vertexInput.inputs =
		{
			{ 0, 0 },
			{ 0, 1 },
			{ 0, 2 }
		};

		pipeline = GraphicsPipeline::create(pipelineSettings);

		// Command pools (1 per frame in flight per thread)
		for (uint32_t i = 0; i < maxFramesInFlight; i++)
		{
			commandPools.push_back(CommandPool::create({}));
		}

		// Command buffers
		commandBuffers.resize(maxFramesInFlight);

		// Fences (1 per frame in flight per thread)
		for (uint32_t i = 0; i < maxFramesInFlight; i++)
		{
			fences.push_back(Fence::create({ true }));
		}

		imageFences.resize(swapChainImageCount);

		// Semaphores (1 per frame in flight per thread)
		for (uint32_t i = 0; i < maxFramesInFlight; i++)
		{
			imageAvailableSemaphores.push_back(Semaphore::create());
			renderFinishedSemaphores.push_back(Semaphore::create());
		}
	}

	void recreateSwapChain()
	{
		
	}
	
	void onEvent(Event& event) override
	{
		
	}

	void drawFrame()
	{
		// Wait on fence to be signaled (max frames in flight)
		auto& fence = fences[currentFrame];

		fence->wait();

		// Acquire next available swapchain image
		auto& imageAvailableSemaphore = imageAvailableSemaphores[currentFrame];
		
		uint32_t imageIndex;
		auto acquireResult = swapChain->acquireNextImage(imageIndex, imageAvailableSemaphore);

		if (acquireResult == SwapChain::AcquireResult::OutOfDate ||
			acquireResult == SwapChain::AcquireResult::Suboptimal)
		{
			recreateSwapChain();
			return;
		}
		else if (acquireResult != SwapChain::AcquireResult::Success)
		{
			ATEMA_ERROR("Failed to acquire a valid swapchain image");
		}
		
		// Check if a previous frame is using this image (i.e. there is its fence to wait on)
		if (imageFences[imageIndex])
			imageFences[imageIndex]->wait();

		// Mark the image as now being in use by this frame
		imageFences[imageIndex] = fence;

		// Update frame data if needed
		
		// Prepare command buffer
		auto& commandPool = commandPools[currentFrame];
		auto& framebuffer = framebuffers[imageIndex];

		auto commandBuffer = CommandBuffer::create({ commandPool });

		commandBuffer->begin();

		std::vector<CommandBuffer::ClearValue> clearValues =
		{
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			{ 1.0f, 0 }
		};
		commandBuffer->beginRenderPass(renderPass, framebuffer, clearValues);

		commandBuffer->bindPipeline(pipeline);

		commandBuffer->endRenderPass();

		commandBuffer->end();

		// Submit command buffer
		auto& renderFinishedSemaphore = renderFinishedSemaphores[currentFrame];
		
		std::vector<Ptr<CommandBuffer>> submitCommandBuffers = { commandBuffer };
		std::vector<Ptr<Semaphore>> submitWaitSemaphores = { imageAvailableSemaphore };
		std::vector<Flags<PipelineStage>> submitWaitStages = { PipelineStage::ColorAttachmentOutput };
		std::vector<Ptr<Semaphore>> submitSignalSemaphores = { renderFinishedSemaphore };
		
		// Reset fence & submit command buffers to the target queue (works with arrays for performance)
		fence->reset();

		Renderer::getInstance().submit(
			submitCommandBuffers,
			submitWaitSemaphores,
			submitWaitStages,
			submitSignalSemaphores,
			fence);

		// Present swapchain image
		Renderer::getInstance().present(
			swapChain,
			imageIndex,
			submitSignalSemaphores
		);
		
		// Save command buffer
		commandBuffers[currentFrame] = commandBuffer;
		
		// Advance frame
		currentFrame = (currentFrame + 1) % maxFramesInFlight;
	}
	
	void update(TimeStep ms) override
	{
		if (window->shouldClose())
		{
			Application::instance().close();
			return;
		}

		window->processEvents();

		drawFrame();
		
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
		//benchmarkSparseSet();

		//entityManager();

		//benchmarkSparseSetUnion();

		basicApplication();
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return -1;
	}
	
	return 0;
}