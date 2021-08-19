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

BasicRenderPipeline::BasicRenderPipeline(const RenderPipeline::Settings& settings) :
	RenderPipeline(settings),
	m_maxFramesInFlight(settings.maxFramesInFlight),
	m_totalTime(0.0f)
{
	const auto windowSize = settings.window->getSize();

	// Descriptor set layout
	DescriptorSetLayout::Settings descriptorSetLayoutSettings;
	descriptorSetLayoutSettings.bindings =
	{
		{ DescriptorType::UniformBuffer, 0, 1, ShaderStage::Vertex },
		{ DescriptorType::CombinedImageSampler, 1, 1, ShaderStage::Fragment }
	};

	m_descriptorSetLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);

	// Descriptor pool
	DescriptorPool::Settings descriptorPoolSettings;
	descriptorPoolSettings.layout = m_descriptorSetLayout;
	descriptorPoolSettings.pageSize = m_maxFramesInFlight * object_count;

	m_descriptorPool = DescriptorPool::create(descriptorPoolSettings);

	// Graphics pipeline
	GraphicsPipeline::Settings pipelineSettings;
	pipelineSettings.viewport.size.x = static_cast<float>(windowSize.x);
	pipelineSettings.viewport.size.y = static_cast<float>(windowSize.y);
	pipelineSettings.scissor.size = windowSize;
	pipelineSettings.vertexShader = Shader::create({ vert_shader_path });
	pipelineSettings.fragmentShader = Shader::create({ frag_shader_path });
	pipelineSettings.renderPass = getRenderPass();
	pipelineSettings.descriptorSetLayout = m_descriptorSetLayout;
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
	
	m_objectFrameData.clear();
	m_scene.reset();
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
	pipelineSettings.descriptorSetLayout = m_descriptorSetLayout;
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

	m_scene->updateObjects(elapsedTime);
}

void BasicRenderPipeline::setupFrame(uint32_t frameIndex, Ptr<CommandBuffer> commandBuffer)
{
	updateUniformBuffers(frameIndex);

	beginRenderPass();

	commandBuffer->bindPipeline(m_pipeline);

	{
		ATEMA_BENCHMARK("CommandBuffers fill");

		auto& objects = m_scene->getObjects();
		
		for (size_t i = 0; i < objects.size(); i++)
		{
			auto& object = objects[i];
			auto descriptorSet = m_objectFrameData[i].getDescriptorSet(frameIndex);
			
			commandBuffer->bindVertexBuffer(object.vertexBuffer, 0);

			commandBuffer->bindIndexBuffer(object.indexBuffer, IndexType::U32);

			commandBuffer->bindDescriptorSet(descriptorSet);

			commandBuffer->drawIndexed(object.indexCount);
		}
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
		ObjectFrameData objectFrameData(object, m_maxFramesInFlight, m_descriptorPool);

		m_objectFrameData.push_back(objectFrameData);
	}
}

void BasicRenderPipeline::updateUniformBuffers(uint32_t frameIndex)
{
	ATEMA_BENCHMARK("BasicRenderPipeline::updateUniformBuffers")

	const auto windowSize = getWindow()->getSize();

	const auto step = 3.14159f / 5.0f;

	const auto zoom = (std::sin(m_totalTime * step) * 400.0f + 400.0f) / 2.0f + 100.0f;
	
	TransformBufferElement transforms{};
	transforms.view = lookAt({ zoom, 0.0f, zoom }, { 0.0f, 0.0f, 15.0f }, { 0.0f, 0.0f, 1.0f });
	transforms.proj = perspective(toRadians(45.0f), static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y), 0.1f, 1000.0f);
	transforms.proj[1][1] *= -1;

	Matrix4f basisChange = rotation4f({ toRadians(90.0f), 0.0f, 0.0f });
	
	auto& objects = m_scene->getObjects();
	
	for (size_t i = 0; i < objects.size(); i++)
	{
		auto& object = objects[i];
		auto buffer = m_objectFrameData[i].getBuffer(frameIndex);

		transforms.model = object.transform;
		
		void* data = buffer->map();

		memcpy(data, static_cast<void*>(&transforms), sizeof(TransformBufferElement));

		buffer->unmap();
	}
}
