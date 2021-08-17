#include <Atema/Atema.hpp>

#include <entt/entt.hpp>

#include <map>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

using namespace at;

const std::filesystem::path rsc_path = "../../examples/Sandbox/Resources/";
const std::filesystem::path model_path = rsc_path / "Models/LampPost.obj";
const std::filesystem::path model_texture_path = rsc_path / "Textures/LampPost_Color.png";

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
struct BasicVertex
{
	Vector3f pos;
	Vector3f color;
	Vector2f texCoord;

	bool operator==(const BasicVertex& other) const
	{
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

struct TransformBufferElement
{
	Matrix4f model;
	Matrix4f view;
	Matrix4f proj;
};

float toRadians(float degrees)
{
	return degrees * 3.14159f / 180.0f;
}

float toDegrees(float radians)
{
	return radians * 180.0f / 3.14159f;
}

namespace std
{
	template<> struct hash<Vector3f>
	{
		size_t operator()(Vector3f const& vertex) const noexcept
		{
			size_t h1 = std::hash<double>()(vertex.x);
			size_t h2 = std::hash<double>()(vertex.y);
			size_t h3 = std::hash<double>()(vertex.z);
			return (h1 ^ (h2 << 1)) ^ h3;
		}
	};

	template<> struct hash<Vector2f>
	{
		size_t operator()(Vector2f const& vertex) const noexcept
		{
			return std::hash<Vector3f>()({ vertex.x, vertex.y, 0.0f });
		}
	};

	template<> struct hash<BasicVertex>
	{
		size_t operator()(BasicVertex const& vertex) const noexcept
		{
			return ((hash<Vector3f>()(vertex.pos) ^
				(hash<Vector3f>()(vertex.color) << 1)) >> 1) ^
				(hash<Vector2f>()(vertex.texCoord) << 1);
		}
	};
}

class TestLayer : public ApplicationLayer
{
public:
	TestLayer() : totalTime(0.0f)
	{
		initialize();
	}

	~TestLayer()
	{
		// SwapChain resources
		destroySwapChain();

		// Frame resources
		uniformBuffers.clear();

		sampler.reset();

		texture.reset();

		indexBuffer.reset();
		
		vertexBuffer.reset();

		descriptorSets.clear();
		
		descriptorPool.reset();

		renderFinishedSemaphores.clear();

		imageAvailableSemaphores.clear();

		imageFences.clear();
		
		fences.clear();
		
		commandBuffers.clear();

		commandPools.clear();

		descriptorSetLayout.reset();

		renderPass.reset();

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

		auto windowSize = window->getSize();

		// RenderPass
		RenderPass::Settings renderPassSettings;
		renderPassSettings.attachments.resize(2);
		renderPassSettings.attachments[0].format = ImageFormat::BGRA8_SRGB;
		renderPassSettings.attachments[0].finalLayout = ImageLayout::Present;
		renderPassSettings.attachments[1].format = ImageFormat::D32F;

		renderPass = RenderPass::create(renderPassSettings);

		// Descriptor set layout
		DescriptorSetLayout::Settings descriptorSetLayoutSettings;
		descriptorSetLayoutSettings.bindings =
		{
			{ DescriptorType::UniformBuffer, 0, 1, ShaderStage::Vertex },
			{ DescriptorType::CombinedImageSampler, 1, 1, ShaderStage::Fragment }
		};

		descriptorSetLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);

		// Descriptor pool
		DescriptorPool::Settings descriptorPoolSettings;
		descriptorPoolSettings.layout = descriptorSetLayout;
		descriptorPoolSettings.pageSize = maxFramesInFlight;
		
		descriptorPool = DescriptorPool::create(descriptorPoolSettings);
		
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

		// Semaphores (1 per frame in flight per thread)
		for (uint32_t i = 0; i < maxFramesInFlight; i++)
		{
			imageAvailableSemaphores.push_back(Semaphore::create());
			renderFinishedSemaphores.push_back(Semaphore::create());
		}

		// Loading model & texture
		loadModel();
		loadTexture();

		// Create uniform buffers (1 per frame in flight)
		for (uint32_t i = 0; i < maxFramesInFlight; i++)
		{
			uniformBuffers.push_back(Buffer::create({ BufferUsage::Uniform, sizeof(TransformBufferElement), true }));
		}

		// Create descriptor sets
		for (uint32_t i = 0; i < maxFramesInFlight; i++)
		{
			auto descriptorSet = descriptorPool->createSet();
			
			descriptorSet->update(0, uniformBuffers[i]);
			descriptorSet->update(1, texture, sampler);

			descriptorSets.push_back(descriptorSet);
		}

		createSwapChain();
	}

	void recreateSwapChain()
	{
		// If window is minimized, we just pause it until it shows again
		auto size = window->getSize();

		while (size.x == 0 || size.y == 0)
		{
			size = window->getSize();
			window->processEvents();
		}

		// Destroy swap chain resources
		destroySwapChain();

		// Create swap chain resources
		createSwapChain();
	}

	// SwapChain / DepthImage / Framebuffers / GraphicsPipeline / Image Fences
	void createSwapChain()
	{
		auto windowSize = window->getSize();

		swapChain = SwapChain::create({ window, ImageFormat::BGRA8_SRGB });

		auto swapChainImageCount = swapChain->getImages().size();

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

		imageFences.resize(swapChainImageCount);
	}

	void destroySwapChain()
	{
		// Resources may be in use, wait until it's not the case anymore
		Renderer::getInstance().waitForIdle();

		//imageFences.clear();

		pipeline.reset();

		framebuffers.clear();

		depthImage.reset();

		swapChain.reset();
	}

	void loadModel()
	{
		std::vector<BasicVertex> modelVertices;
		std::vector<uint32_t> modelIndices;

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, model_path.string().c_str()))
		{
			ATEMA_ERROR(warn + err);
		}

		std::unordered_map<BasicVertex, uint32_t> uniqueVertices{};

		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				BasicVertex vertex;

				vertex.pos =
				{
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				//vertex.texCoord =
				//{
				//	attrib.texcoords[2 * index.texcoord_index + 0],
				//	attrib.texcoords[2 * index.texcoord_index + 1]
				//};

				vertex.texCoord =
				{
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1] // Flip vertical component of texcoord
				};

				vertex.color = { 1.0f, 1.0f, 1.0f };

				// This method may duplicate vertices and we don't want that
				//m_modelVertices.push_back(vertex);
				//m_modelIndices.push_back(m_modelIndices.size());

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(modelVertices.size());
					modelVertices.push_back(vertex);
				}

				modelIndices.push_back(uniqueVertices[vertex]);
			}
		}

		// Create vertex buffer
		{
			// Fill staging buffer
			size_t bufferSize = sizeof(modelVertices[0]) * modelVertices.size();

			auto stagingBuffer = Buffer::create({ BufferUsage::Transfer, bufferSize, true });

			auto bufferData = stagingBuffer->map();

			memcpy(bufferData, static_cast<void*>(modelVertices.data()), static_cast<size_t>(bufferSize));

			stagingBuffer->unmap();

			// Create vertex buffer
			vertexBuffer = Buffer::create({ BufferUsage::Vertex, bufferSize });

			// Copy staging buffer to vertex buffer
			auto commandBuffer = CommandBuffer::create({ commandPools[0], true });

			commandBuffer->begin();

			commandBuffer->copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
			
			commandBuffer->end();
			
			auto fence = Fence::create({});

			Renderer::getInstance().submit(
				{ commandBuffer },
				{},
				{},
				{},
				fence);

			// Wait for the command to be done
			fence->wait();
		}

		// Create index buffer
		{
			// Fill staging buffer
			size_t bufferSize = sizeof(modelIndices[0]) * modelIndices.size();

			auto stagingBuffer = Buffer::create({ BufferUsage::Transfer, bufferSize, true });

			auto bufferData = stagingBuffer->map();

			memcpy(bufferData, static_cast<void*>(modelIndices.data()), static_cast<size_t>(bufferSize));

			stagingBuffer->unmap();

			// Create vertex buffer
			indexBuffer = Buffer::create({ BufferUsage::Index, bufferSize });

			// Copy staging buffer to index buffer
			auto commandBuffer = CommandBuffer::create({ commandPools[0], true });

			commandBuffer->begin();

			commandBuffer->copyBuffer(stagingBuffer, indexBuffer, bufferSize);

			commandBuffer->end();

			auto fence = Fence::create({});

			Renderer::getInstance().submit(
				{ commandBuffer },
				{},
				{},
				{},
				fence);

			// Wait for the command to be done
			fence->wait();
		}

		indexCount = modelIndices.size();
	}

	void loadTexture()
	{
		// Load the texture data
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(model_texture_path.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		size_t imageSize = texWidth * texHeight * 4;

		// std::max for the largest dimension, std::log2 to find how many times we can divide by 2
		// std::floor handles the case when the dimension is not power of 2, +1 to add a mip level to the original image
		auto textureMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

		if (!pixels)
			ATEMA_ERROR("Failed to load texture image");

		// Fill staging buffer
		auto stagingBuffer = Buffer::create({ BufferUsage::Transfer, imageSize, true });

		auto bufferData = stagingBuffer->map();

		memcpy(bufferData, pixels, imageSize);

		stagingBuffer->unmap();

		// Free image data
		stbi_image_free(pixels);

		// Create image
		Image::Settings imageSettings;
		imageSettings.format = ImageFormat::RGBA8_SRGB;
		imageSettings.width = texWidth;
		imageSettings.height = texHeight;
		imageSettings.mipLevels = textureMipLevels;
		imageSettings.usages = ImageUsage::ShaderInput | ImageUsage::TransferDst | ImageUsage::TransferSrc;

		texture = Image::create(imageSettings);

		// Copy staging buffer to index buffer
		auto commandBuffer = CommandBuffer::create({ commandPools[0], true });

		commandBuffer->begin();

		commandBuffer->setImageLayout(texture, ImageLayout::TransferDst);
		
		commandBuffer->copyBuffer(stagingBuffer, texture);

		commandBuffer->createMipmaps(texture);

		commandBuffer->setImageLayout(texture, ImageLayout::ShaderInput);

		commandBuffer->end();

		auto fence = Fence::create({});

		Renderer::getInstance().submit(
			{ commandBuffer },
			{},
			{},
			{},
			fence);

		// Wait for the command to be done
		fence->wait();

		// Create sampler
		Sampler::Settings samplerSettings(SamplerFilter::Linear, true);

		sampler = Sampler::create(samplerSettings);
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

		if (acquireResult == SwapChainResult::OutOfDate ||
			acquireResult == SwapChainResult::Suboptimal)
		{
			recreateSwapChain();
			return;
		}
		else if (acquireResult != SwapChainResult::Success)
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
		auto& uniformBuffer = uniformBuffers[currentFrame];
		auto& descriptorSet = descriptorSets[currentFrame];

		auto commandBuffer = CommandBuffer::create({ commandPool });

		commandBuffer->begin();

		std::vector<CommandBuffer::ClearValue> clearValues =
		{
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			{ 1.0f, 0 }
		};
		commandBuffer->beginRenderPass(renderPass, framebuffer, clearValues);

		commandBuffer->bindPipeline(pipeline);

		commandBuffer->bindVertexBuffer(vertexBuffer, 0);
		
		commandBuffer->bindIndexBuffer(indexBuffer, IndexType::U32);

		commandBuffer->bindDescriptorSet(descriptorSet);

		commandBuffer->drawIndexed(indexCount);

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
		acquireResult = Renderer::getInstance().present(
			swapChain,
			imageIndex,
			submitSignalSemaphores
		);
		
		// Save command buffer
		commandBuffers[currentFrame] = commandBuffer;
		
		// Advance frame
		currentFrame = (currentFrame + 1) % maxFramesInFlight;

		if (acquireResult == SwapChainResult::OutOfDate ||
			acquireResult == SwapChainResult::Suboptimal)
		{
			recreateSwapChain();
		}
		else if (acquireResult != SwapChainResult::Success)
		{
			ATEMA_ERROR("SwapChain presentation failed");
		}
	}

	void updateUniformBuffer()
	{
		auto& buffer = uniformBuffers[currentFrame];
		auto windowSize = window->getSize();

		Vector3f rotation;
		rotation.z = totalTime * toRadians(45.0f);

		Matrix4f basisChange = rotation4f({ toRadians(90.0f), 0.0f, 0.0f });

		TransformBufferElement transforms{};
		transforms.model = rotation4f(rotation) * basisChange;
		transforms.view = lookAt({ 80.0f, 0.0f, 80.0f }, { 0.0f, 0.0f, 15.0f }, { 0.0f, 0.0f, 1.0f });
		transforms.proj = perspective(toRadians(45.0f), static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y), 0.1f, 1000.0f);

		transforms.proj[1][1] *= -1;

		void* data = buffer->map();

		memcpy(data, static_cast<void*>(&transforms), sizeof(TransformBufferElement));

		buffer->unmap();
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

		updateUniformBuffer();
		
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
	Ptr<DescriptorPool> descriptorPool;

	Ptr<Buffer> vertexBuffer;
	Ptr<Buffer> indexBuffer;
	Ptr<Image> texture;
	Ptr<Sampler> sampler;
	std::vector<Ptr<Buffer>> uniformBuffers;
	std::vector<Ptr<DescriptorSet>> descriptorSets;

	uint32_t indexCount;

	float totalTime;

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