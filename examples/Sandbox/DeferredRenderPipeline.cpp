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

#include "DeferredRenderPipeline.hpp"

#include "Resources.hpp"

using namespace at;

namespace
{
	constexpr size_t targetThreadCount = 4;

	const size_t threadCount = std::min(targetThreadCount, TaskManager::getInstance().getSize());

	struct PPVertex
	{
		Vector3f position;
		Vector2f texCoords;
	};

	std::vector<PPVertex> quadVertices =
	{
		{{ -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }},
		{{ -1.0f, +1.0f, 0.0f }, { 0.0f, 1.0f }},
		{{ +1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f }},

		{{ +1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f }},
		{{ -1.0f, +1.0f, 0.0f }, { 0.0f, 1.0f }},
		{{ +1.0f, +1.0f, 0.0f }, { 1.0f, 1.0f }}
	};
	
	Ptr<Buffer> createQuad(Ptr<CommandPool> commandPool)
	{
		// Fill staging buffer
		size_t bufferSize = sizeof(quadVertices[0]) * quadVertices.size();

		auto stagingBuffer = Buffer::create({ BufferUsage::Transfer, bufferSize, true });

		auto bufferData = stagingBuffer->map();

		memcpy(bufferData, static_cast<void*>(quadVertices.data()), static_cast<size_t>(bufferSize));

		stagingBuffer->unmap();

		// Create vertex buffer
		auto vertexBuffer = Buffer::create({ BufferUsage::Vertex, bufferSize });

		// Copy staging buffer to vertex buffer
		auto commandBuffer = CommandBuffer::create({ commandPool, true });

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

		return vertexBuffer;
	}
}

DeferredRenderPipeline::DeferredRenderPipeline(const RenderPipeline::Settings& settings) :
	RenderPipeline(settings),
	m_maxFramesInFlight(settings.maxFramesInFlight),
	m_totalTime(0.0f),
	m_depthFormat(settings.depthFormat)
{
	//----- DEFERRED RESOURCES -----//
	// Create RenderPass
	{
		RenderPass::Settings renderPassSettings;

		// Color attachments
		for (auto& format : gbuffer)
		{
			AttachmentDescription attachment;
			attachment.format = format;
			attachment.initialLayout = ImageLayout::Undefined;
			attachment.finalLayout = ImageLayout::ShaderInput;

			renderPassSettings.attachments.push_back(attachment);
		}

		// Depth attachment
		AttachmentDescription attachment;
		attachment.format = settings.depthFormat;

		renderPassSettings.attachments.push_back(attachment);

		// Create RenderPass
		m_deferredRenderPass = RenderPass::create(renderPassSettings);
	}

	// Create Sampler
	{
		Sampler::Settings samplerSettings(SamplerFilter::Nearest);

		m_ppSampler = Sampler::create(samplerSettings);
	}

	// Create DescriptorSetLayout
	{
		DescriptorSetLayout::Settings descriptorSetLayoutSettings;
		descriptorSetLayoutSettings.bindings =
		{
			{ DescriptorType::CombinedImageSampler, 0, 1, ShaderStage::Fragment },
			{ DescriptorType::CombinedImageSampler, 1, 1, ShaderStage::Fragment },
			{ DescriptorType::CombinedImageSampler, 2, 1, ShaderStage::Fragment }
		};

		m_ppDescriptorSetLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);
	}

	// Create DescriptorPool
	{
		DescriptorPool::Settings descriptorPoolSettings;
		descriptorPoolSettings.layout = m_ppDescriptorSetLayout;
		descriptorPoolSettings.pageSize = 1;

		m_ppDescriptorPool = DescriptorPool::create(descriptorPoolSettings);
	}

	// Create descriptor set
	{
		m_ppDescriptorSet = m_ppDescriptorPool->createSet();
	}

	// Create quad
	{
		m_ppQuad = createQuad(getCommandPools()[0]);
	}
	
	//----- OBJECT RESOURCES -----//

	// Descriptor set layout
	{
		DescriptorSetLayout::Settings descriptorSetLayoutSettings;
		descriptorSetLayoutSettings.bindings =
		{
			{ DescriptorType::UniformBuffer, 0, 1, ShaderStage::Vertex },
			{ DescriptorType::CombinedImageSampler, 1, 1, ShaderStage::Fragment }
		};

		m_objectDescriptorSetLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);
	}

	// Descriptor pool
	{
		DescriptorPool::Settings descriptorPoolSettings;
		descriptorPoolSettings.layout = m_objectDescriptorSetLayout;
		descriptorPoolSettings.pageSize = m_maxFramesInFlight * objectCount;

		m_objectDescriptorPool = DescriptorPool::create(descriptorPoolSettings);
	}

	//----- FRAME RESOURCES -----//

	// Descriptor set layout
	{
		DescriptorSetLayout::Settings descriptorSetLayoutSettings;
		descriptorSetLayoutSettings.bindings =
		{
			{ DescriptorType::UniformBuffer, 0, 1, ShaderStage::Vertex }
		};

		m_frameDescriptorSetLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);
	}

	// Descriptor pool
	{
		DescriptorPool::Settings descriptorPoolSettings;
		descriptorPoolSettings.layout = m_frameDescriptorSetLayout;
		descriptorPoolSettings.pageSize = m_maxFramesInFlight;

		m_frameDescriptorPool = DescriptorPool::create(descriptorPoolSettings);
	}

	// Uniform buffers & descriptor sets
	m_frameUniformBuffers.reserve(m_maxFramesInFlight);
	m_frameDescriptorSets.reserve(m_maxFramesInFlight);

	for (uint32_t j = 0; j < m_maxFramesInFlight; j++)
	{
		auto uniformBuffer = Buffer::create({ BufferUsage::Uniform, sizeof(UniformFrameElement), true });

		m_frameUniformBuffers.push_back(uniformBuffer);

		auto descriptorSet = m_frameDescriptorPool->createSet();

		descriptorSet->update(0, uniformBuffer);

		m_frameDescriptorSets.push_back(descriptorSet);
	}

	//----- THREAD RESOURCES -----//
	auto& taskManager = TaskManager::getInstance();
	const auto coreCount = taskManager.getSize();

	m_threadCommandBuffers.resize(coreCount);

	for (auto& commandBuffers : m_threadCommandBuffers)
		commandBuffers.resize(m_maxFramesInFlight);

	m_threadCommandPools.reserve(coreCount);
	for (size_t i = 0; i < coreCount; i++)
		m_threadCommandPools.push_back(CommandPool::create({}));

	// Create size dependent resources
	const auto windowSize = settings.window->getSize();

	resize(windowSize);

	// Load scene
	loadScene();
}

DeferredRenderPipeline::~DeferredRenderPipeline()
{
	Renderer::getInstance().waitForIdle();

	// Rendering resources
	m_ppDescriptorSetLayout.reset();
	m_ppDescriptorSet.reset();
	m_ppDescriptorPool.reset();

	m_deferredRenderPass.reset();

	m_deferredFramebuffer.reset();
	m_deferredDepthImage.reset();
	m_deferredImages.clear();
	
	m_ppSampler.reset();

	m_ppPipeline.reset();
	
	// Thread resources
	m_threadCommandBuffers.clear();
	m_threadCommandPools.clear();

	// Frame resources
	m_frameDescriptorSetLayout.reset();
	m_frameDescriptorSets.clear();
	m_frameDescriptorPool.reset();
	m_frameUniformBuffers.clear();

	// Object resources
	m_objectDescriptorSetLayout.reset();
	m_objectDescriptorPool.reset();
	m_objectFrameData.clear();
	m_scene.reset();

	m_pipeline.reset();
}

void DeferredRenderPipeline::resize(const Vector2u& size)
{
	// Create color images
	{
		m_deferredImages.clear();
		
		for (auto& format : gbuffer)
		{
			Image::Settings imageSettings;
			imageSettings.format = format;
			imageSettings.width = size.x;
			imageSettings.height = size.y;
			imageSettings.usages = ImageUsage::RenderTarget | ImageUsage::ShaderInput;

			auto image = Image::create(imageSettings);

			m_deferredImages.push_back(image);
		}
	}

	// Create depth image
	{
		Image::Settings imageSettings;
		imageSettings.format = m_depthFormat;
		imageSettings.width = size.x;
		imageSettings.height = size.y;
		imageSettings.usages = ImageUsage::RenderTarget;

		m_deferredDepthImage = Image::create(imageSettings);
	}

	// Create framebuffer
	{
		Framebuffer::Settings framebufferSettings;
		framebufferSettings.width = size.x;
		framebufferSettings.height = size.y;
		framebufferSettings.images = m_deferredImages;
		framebufferSettings.images.push_back(m_deferredDepthImage);
		framebufferSettings.renderPass = m_deferredRenderPass;

		m_deferredFramebuffer = Framebuffer::create(framebufferSettings);
	}

	// Write descriptor set
	{
		for (uint32_t i = 0; i < gbuffer.size(); i++)
			m_ppDescriptorSet->update(i, m_deferredImages[i], m_ppSampler);
	}

	// Post process pipeline
	{
		m_ppPipeline.reset();

		GraphicsPipeline::Settings pipelineSettings;
		pipelineSettings.viewport.size.x = static_cast<float>(size.x);
		pipelineSettings.viewport.size.y = static_cast<float>(size.y);
		pipelineSettings.scissor.size = size;
		pipelineSettings.vertexShader = Shader::create({ deferredPostProcessVertexPath });
		pipelineSettings.fragmentShader = Shader::create({ deferredPostProcessFragmentPath });
		pipelineSettings.renderPass = getRenderPass();
		pipelineSettings.descriptorSetLayouts = { m_ppDescriptorSetLayout };
		// Position / TexCoords
		pipelineSettings.vertexInput.attributes =
		{
			{ VertexAttribute::Format::RGB32_SFLOAT },
			{ VertexAttribute::Format::RG32_SFLOAT }
		};
		pipelineSettings.vertexInput.inputs =
		{
			{ 0, 0 },
			{ 0, 1 }
		};

		m_ppPipeline = GraphicsPipeline::create(pipelineSettings);
	}
	
	// Deferred pipeline
	{
		m_pipeline.reset();

		GraphicsPipeline::Settings pipelineSettings;
		pipelineSettings.viewport.size.x = static_cast<float>(size.x);
		pipelineSettings.viewport.size.y = static_cast<float>(size.y);
		pipelineSettings.scissor.size = size;
		pipelineSettings.vertexShader = Shader::create({ deferredMaterialVertexPath });
		pipelineSettings.fragmentShader = Shader::create({ deferredMaterialFragmentPath });
		pipelineSettings.renderPass = m_deferredRenderPass;
		pipelineSettings.descriptorSetLayouts = { m_frameDescriptorSetLayout, m_objectDescriptorSetLayout };
		pipelineSettings.vertexInput.attributes =
		{
			{ VertexAttribute::Format::RGB32_SFLOAT },
			{ VertexAttribute::Format::RGB32_SFLOAT },
			{ VertexAttribute::Format::RG32_SFLOAT }
		};
		pipelineSettings.vertexInput.inputs =
		{
			{ 0, 0 },
			{ 0, 1 },
			{ 0, 2 }
		};

		m_pipeline = GraphicsPipeline::create(pipelineSettings);
	}
}

void DeferredRenderPipeline::updateFrame(at::TimeStep elapsedTime)
{
	ATEMA_BENCHMARK("Scene::updateObjects");

	m_totalTime += elapsedTime.getSeconds();

	m_scene->updateObjects(elapsedTime, threadCount);
}

void DeferredRenderPipeline::setupFrame(uint32_t frameIndex, Ptr<CommandBuffer> commandBuffer)
{
	// Update scene data
	updateUniformBuffers(frameIndex);

	// Deferred pass
	std::vector<CommandBuffer::ClearValue> clearValues;

	for (auto& format : gbuffer)
		clearValues.push_back({ 0.0f, 0.0f, 0.0f, 1.0f });

	clearValues.push_back({ 1.0f, 0 });

	commandBuffer->beginRenderPass(m_deferredRenderPass, m_deferredFramebuffer, clearValues, true);

	auto& globalDescriptorSet = m_frameDescriptorSets[frameIndex];

	// Update objects buffers
	{
		ATEMA_BENCHMARK("CommandBuffers fill");

		// Clear previous command buffers
		for (auto& threadCommandBuffers : m_threadCommandBuffers)
		{
			threadCommandBuffers[frameIndex].clear();
		}

		auto& objects = m_scene->getObjects();

		auto& taskManager = TaskManager::getInstance();

		std::vector<Ptr<Task>> tasks;
		tasks.reserve(threadCount);

		std::vector<Ptr<CommandBuffer>> commandBuffers;
		commandBuffers.resize(threadCount);

		size_t firstIndex = 0;
		size_t size = objects.size() / threadCount;

		for (size_t taskIndex = 0; taskIndex < threadCount; taskIndex++)
		{
			auto lastIndex = firstIndex + size;

			if (taskIndex == threadCount - 1)
			{
				const auto remainingSize = objects.size() - lastIndex;

				lastIndex += remainingSize;
			}

			auto task = taskManager.createTask([this, taskIndex, firstIndex, lastIndex, &objects, &commandBuffers, globalDescriptorSet, frameIndex](size_t threadIndex)
				{
					auto& commandPool = m_threadCommandPools[threadIndex];
					auto commandBuffer = CommandBuffer::create({ commandPool, true, true });

					commandBuffer->beginSecondary(m_deferredRenderPass, m_deferredFramebuffer);

					commandBuffer->bindPipeline(m_pipeline);

					for (size_t i = firstIndex; i < lastIndex; i++)
					{
						auto& object = objects[i];
						auto descriptorSet = m_objectFrameData[i].getDescriptorSet(frameIndex);

						commandBuffer->bindVertexBuffer(object.vertexBuffer, 0);

						commandBuffer->bindIndexBuffer(object.indexBuffer, IndexType::U32);

						commandBuffer->bindDescriptorSets({ globalDescriptorSet, descriptorSet });

						commandBuffer->drawIndexed(object.indexCount);
					}

					commandBuffer->end();

					m_threadCommandBuffers[threadIndex][frameIndex].push_back(commandBuffer);

					commandBuffers[taskIndex] = commandBuffer;
				});

			tasks.push_back(task);

			firstIndex += size;
		}

		for (auto& task : tasks)
			task->wait();

		commandBuffer->executeSecondaryCommands(commandBuffers);
	}

	commandBuffer->endRenderPass();
	
	// Post process pass
	beginRenderPass();

	commandBuffer->bindPipeline(m_ppPipeline);

	commandBuffer->bindVertexBuffer(m_ppQuad, 0);

	commandBuffer->bindDescriptorSets({ m_ppDescriptorSet });

	commandBuffer->draw(6);

	endRenderPass();
}

void DeferredRenderPipeline::loadScene()
{
	ATEMA_BENCHMARK("DeferredRenderPipeline::loadScene");

	auto& commandPool = getCommandPools()[0];

	m_scene = std::make_shared<Scene>(commandPool);

	auto& objects = m_scene->getObjects();

	m_objectFrameData.reserve(objects.size());

	for (auto& object : objects)
	{
		ObjectFrameData objectFrameData(object, m_maxFramesInFlight, m_objectDescriptorPool);

		m_objectFrameData.push_back(objectFrameData);
	}
}

void DeferredRenderPipeline::updateUniformBuffers(uint32_t frameIndex)
{
	ATEMA_BENCHMARK("DeferredRenderPipeline::updateUniformBuffers")

		// Update global buffers
	{
		const auto windowSize = getWindow()->getSize();

		const auto angle = m_totalTime * zoomSpeed;

		const auto sin = std::sin(angle);
		const auto sinSlow = std::sin(angle / 2.0f + 3.14159f);

		const auto sign = (sin + 1.0f) / 2.0f;
		const auto signSlow = (sinSlow + 1.0f) / 2.0f;

		auto radius = modelScale * objectRow;
		radius = sign * radius + (1.0f - sign) * zoomOffset;

		const auto pos = toCartesian({ radius, angle / 3.0f });

		const auto z = signSlow * radius + (1.0f - signSlow) * zoomOffset;

		const Vector3f cameraPos(pos.x, pos.y, z);
		const Vector3f cameraTarget(0.0f, 0.0f, zoomOffset / 2.0f);
		const Vector3f cameraUp(0.0f, 0.0f, 1.0f);

		UniformFrameElement frameTransforms;
		frameTransforms.view = lookAt(cameraPos, cameraTarget, cameraUp);
		frameTransforms.proj = perspective(toRadians(45.0f), static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y), 0.1f, 1000.0f);
		frameTransforms.proj[1][1] *= -1;

		auto buffer = m_frameUniformBuffers[frameIndex];

		void* data = buffer->map();

		memcpy(data, static_cast<void*>(&frameTransforms), sizeof(UniformFrameElement));

		buffer->unmap();
	}

	// Update objects buffers
	{
		auto& objects = m_scene->getObjects();

		auto& taskManager = TaskManager::getInstance();

		// Divide the updates in max groups
		std::vector<Ptr<Task>> tasks;
		tasks.reserve(threadCount);

		size_t firstIndex = 0;
		size_t size = objects.size() / threadCount;

		for (size_t i = 0; i < threadCount; i++)
		{
			auto lastIndex = firstIndex + size;

			if (i == threadCount - 1)
			{
				const auto remainingSize = objects.size() - lastIndex;

				lastIndex += remainingSize;
			}

			auto task = taskManager.createTask([this, firstIndex, lastIndex, &objects, frameIndex]()
				{
					for (size_t i = firstIndex; i < lastIndex; i++)
					{
						auto& object = objects[i];
						auto buffer = m_objectFrameData[i].getBuffer(frameIndex);

						UniformObjectElement objectTransforms;
						objectTransforms.model = object.transform;

						void* data = buffer->map();

						memcpy(data, static_cast<void*>(&objectTransforms), sizeof(UniformObjectElement));

						buffer->unmap();
					}
				});

			tasks.push_back(task);

			firstIndex += size;
		}

		for (auto& task : tasks)
			task->wait();
	}
}
