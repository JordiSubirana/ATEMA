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

#define OBJECT_ROW_COUNT 10
#define OBJECT_COUNT (OBJECT_ROW_COUNT*OBJECT_ROW_COUNT)

using namespace at;

namespace
{
	const std::filesystem::path rsc_path = "../../examples/Sandbox/Resources/";
	const std::filesystem::path model_path = rsc_path / "Models/LampPost.obj";
	const std::filesystem::path model_texture_path = rsc_path / "Textures/LampPost_Color.png";

	float toRadians(float degrees)
	{
		return degrees * 3.14159f / 180.0f;
	}

	float toDegrees(float radians)
	{
		return radians * 180.0f / 3.14159f;
	}

	struct TransformBufferElement
	{
		Matrix4f model;
		Matrix4f view;
		Matrix4f proj;
	};
}

BasicRenderPipeline::BasicRenderPipeline(const RenderPipeline::Settings& settings) :
	RenderPipeline(settings),
	m_totalTime(0.0f)
{
	loadResources();

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
	descriptorPoolSettings.pageSize = settings.maxFramesInFlight * OBJECT_COUNT;

	m_descriptorPool = DescriptorPool::create(descriptorPoolSettings);

	// Graphics pipeline
	GraphicsPipeline::Settings pipelineSettings;
	pipelineSettings.viewport.size.x = static_cast<float>(windowSize.x);
	pipelineSettings.viewport.size.y = static_cast<float>(windowSize.y);
	pipelineSettings.scissor.size = windowSize;
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

	// Object creation
	for (auto i = 0; i < OBJECT_COUNT; i++)
	{
		auto object = std::make_shared<ObjectData>();
		object->vertexBuffer = m_modelData->vertexBuffer;
		object->indexBuffer = m_modelData->indexBuffer;
		object->indexCount = m_modelData->indexCount;
		object->texture = m_materialData->texture;
		object->sampler = m_materialData->sampler;

		for (uint32_t j = 0; j < settings.maxFramesInFlight; j++)
		{
			auto uniformBuffer = Buffer::create({ BufferUsage::Uniform, sizeof(TransformBufferElement), true });
			
			object->uniformBuffers.push_back(uniformBuffer);

			auto descriptorSet = m_descriptorPool->createSet();

			descriptorSet->update(0, uniformBuffer);
			descriptorSet->update(1, object->texture, object->sampler);

			object->descriptorSets.push_back(descriptorSet);
		}

		m_objects.push_back(object);
	}
}

BasicRenderPipeline::~BasicRenderPipeline()
{
	Renderer::getInstance().waitForIdle();
	
	m_objects.clear();
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

void BasicRenderPipeline::setupFrame(uint32_t frameIndex, TimeStep elapsedTime, Ptr<CommandBuffer> commandBuffer)
{
	m_totalTime += elapsedTime.getSeconds();
	
	updateUniformBuffers(frameIndex);

	beginRenderPass();

	{
		ATEMA_BENCHMARK("Bind Pipeline");
		
		commandBuffer->bindPipeline(m_pipeline);
	}

	{
		ATEMA_BENCHMARK("CommandBuffer objects commands");
		
		for (auto& object : m_objects)
		{
			commandBuffer->bindVertexBuffer(object->vertexBuffer, 0);

			commandBuffer->bindIndexBuffer(object->indexBuffer, IndexType::U32);

			commandBuffer->bindDescriptorSet(object->descriptorSets[frameIndex]);

			commandBuffer->drawIndexed(object->indexCount);
		}
	}

	endRenderPass();
}

void BasicRenderPipeline::loadResources()
{
	ATEMA_BENCHMARK("BasicRenderPipeline::loadResources")

	auto& commandPool = getCommandPools()[0];
	
	m_modelData = std::make_shared<ModelData>(model_path, commandPool);

	m_materialData = std::make_shared<MaterialData>(model_texture_path, commandPool);
}

void BasicRenderPipeline::updateUniformBuffers(uint32_t frameIndex)
{
	ATEMA_BENCHMARK("BasicRenderPipeline::updateUniformBuffers")

	const auto windowSize = getWindow()->getSize();

	const auto scale = 30.0f;
	const auto origin = -scale * (OBJECT_ROW_COUNT / 2.0f);

	auto step = 3.14159f / 5.0f;
	
	auto zoom = (std::sin(m_totalTime * step) * 400.0f + 400.0f) / 2.0f + 100.0f;
	
	TransformBufferElement transforms{};
	transforms.view = lookAt({ zoom, 0.0f, zoom }, { 0.0f, 0.0f, 15.0f }, { 0.0f, 0.0f, 1.0f });
	transforms.proj = perspective(toRadians(45.0f), static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y), 0.1f, 1000.0f);
	transforms.proj[1][1] *= -1;

	Matrix4f basisChange = rotation4f({ toRadians(90.0f), 0.0f, 0.0f });
	
	for (size_t i = 0; i < OBJECT_ROW_COUNT; i++)
	{
		for (size_t j = 0; j < OBJECT_ROW_COUNT; j++)
		{
			const auto index = i * OBJECT_ROW_COUNT + j;

			auto rotScale = 10.0f + 70.0f * (static_cast<float>(index) / OBJECT_COUNT);

			auto& object = m_objects[index];
			auto& buffer = object->uniformBuffers[frameIndex];

			Vector3f rotation;
			rotation.z = m_totalTime * toRadians(rotScale);

			Vector3f offset;
			offset.x = scale * i + origin;
			offset.y = scale * j + origin;
			
			auto translationMatrix = translation(offset);

			transforms.model = translationMatrix * rotation4f(rotation) * basisChange;
			
			void* data = buffer->map();

			memcpy(data, static_cast<void*>(&transforms), sizeof(TransformBufferElement));

			buffer->unmap();
		}
	}
}
