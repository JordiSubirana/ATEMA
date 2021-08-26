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

#include "BasicRenderPipeline.hpp"

#include "Resources.hpp"

using namespace at;

namespace
{
	constexpr size_t targetThreadCount = 4;

	const size_t threadCount = std::min(targetThreadCount, TaskManager::getInstance().getSize());
}

BasicRenderPipeline::BasicRenderPipeline(const RenderPipeline::Settings& settings) :
	RenderPipeline(settings),
	m_maxFramesInFlight(settings.maxFramesInFlight),
	m_totalTime(0.0f)
{
	const auto windowSize = settings.window->getSize();

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
		descriptorPoolSettings.pageSize = m_maxFramesInFlight * object_count;

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

	//----- PIPELINE RESOURCES -----//
	
	// Graphics pipeline
	GraphicsPipeline::Settings pipelineSettings;
	pipelineSettings.viewport.size.x = static_cast<float>(windowSize.x);
	pipelineSettings.viewport.size.y = static_cast<float>(windowSize.y);
	pipelineSettings.scissor.size = windowSize;
	pipelineSettings.vertexShader = Shader::create({ vert_shader_path });
	pipelineSettings.fragmentShader = Shader::create({ frag_shader_path });
	pipelineSettings.renderPass = getRenderPass();
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

	loadScene();
}

BasicRenderPipeline::~BasicRenderPipeline()
{
	Renderer::getInstance().waitForIdle();

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

void BasicRenderPipeline::resize(const Vector2u& size)
{
	m_pipeline.reset();

	GraphicsPipeline::Settings pipelineSettings;
	pipelineSettings.viewport.size.x = static_cast<float>(size.x);
	pipelineSettings.viewport.size.y = static_cast<float>(size.y);
	pipelineSettings.scissor.size = size;
	pipelineSettings.vertexShader = Shader::create({ rsc_path / "Shaders/vert.spv" });
	pipelineSettings.fragmentShader = Shader::create({ rsc_path / "Shaders/frag.spv" });
	pipelineSettings.renderPass = getRenderPass();
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

void BasicRenderPipeline::updateFrame(at::TimeStep elapsedTime)
{
	ATEMA_BENCHMARK("Scene::updateObjects")

	m_totalTime += elapsedTime.getSeconds();

	m_scene->updateObjects(elapsedTime, threadCount);
}

void BasicRenderPipeline::setupFrame(uint32_t frameIndex, Ptr<CommandBuffer> commandBuffer)
{
	updateUniformBuffers(frameIndex);

	auto& globalDescriptorSet = m_frameDescriptorSets[frameIndex];
	
	beginRenderPass(true);

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

					commandBuffer->beginSecondary(getRenderPass(), getCurrentFramebuffer());

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

	endRenderPass();
}

void BasicRenderPipeline::loadScene()
{
	ATEMA_BENCHMARK("BasicRenderPipeline::loadScene")

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

void BasicRenderPipeline::updateUniformBuffers(uint32_t frameIndex)
{
	ATEMA_BENCHMARK("BasicRenderPipeline::updateUniformBuffers")

	// Update global buffers
	{
		const auto windowSize = getWindow()->getSize();

		const auto step = 3.14159f / 5.0f;

		const auto zoom = (std::sin(m_totalTime * step) * 400.0f + 400.0f) / 2.0f + 100.0f;

		UniformFrameElement frameTransforms;
		frameTransforms.view = lookAt({ zoom, 0.0f, zoom }, { 0.0f, 0.0f, 15.0f }, { 0.0f, 0.0f, 1.0f });
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
