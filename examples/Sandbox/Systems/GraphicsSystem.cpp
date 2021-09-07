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

#include "GraphicsSystem.hpp"
#include "../Resources.hpp"
#include "../Components/GraphicsComponent.hpp"
#include "../Components/CameraComponent.hpp"

using namespace at;

namespace
{
	constexpr size_t targetThreadCount = 8;

	const size_t threadCount = std::min(targetThreadCount, TaskManager::instance().getSize());

	const std::vector<ImageFormat> gBuffer =
	{
		// Position
		ImageFormat::RGBA32_SFLOAT,
		// Normal
		ImageFormat::RGBA32_SFLOAT,
		// Color
		ImageFormat::RGBA8_SRGB
	};
	
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
		auto commandBuffer = commandPool->createBuffer({ true });

		commandBuffer->begin();

		commandBuffer->copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		commandBuffer->end();

		Renderer::instance().submitAndWait({ commandBuffer });

		return vertexBuffer;
	}

	struct UniformFrameElement
	{
		Matrix4f proj;
		Matrix4f view;
	};
	
	struct UniformObjectElement
	{
		Matrix4f model;
	};
}

GraphicsSystem::GraphicsSystem() :
	System(),
	m_maxFramesInFlight(2),
	m_totalTime(0.0f)
{
	auto& renderer = Renderer::instance();
	auto window = renderer.getMainWindow();

	//----- RENDER PIPELINE -----//
	RenderPipeline::Settings renderPipelineSettings;
	renderPipelineSettings.window = window;
	renderPipelineSettings.maxFramesInFlight = m_maxFramesInFlight;
	renderPipelineSettings.resizeCallback = [this](const Vector2u& size)
	{
		onResize(size);
	};
	renderPipelineSettings.updateFrameCallback = [this](uint32_t frameIndex, Ptr<CommandBuffer> commandBuffer)
	{
		onUpdateFrame(frameIndex, commandBuffer);
	};

	m_renderPipeline = std::make_unique<RenderPipeline>(renderPipelineSettings);

	// Keep default values
	m_depthFormat = renderPipelineSettings.depthFormat;

	//----- DEFERRED RESOURCES -----//
	// Create RenderPass
	{
		RenderPass::Settings renderPassSettings;

		// Color attachments
		for (auto& format : gBuffer)
		{
			AttachmentDescription attachment;
			attachment.format = format;
			attachment.initialLayout = ImageLayout::Undefined;
			attachment.finalLayout = ImageLayout::ShaderInput;

			renderPassSettings.attachments.push_back(attachment);
		}

		// Depth attachment
		AttachmentDescription attachment;
		attachment.format = m_depthFormat;

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
		m_ppQuad = createQuad(renderer.getDefaultCommandPool());
	}

	//----- OBJECT RESOURCES -----//

	// Descriptor set layout
	{
		DescriptorSetLayout::Settings descriptorSetLayoutSettings;
		descriptorSetLayoutSettings.bindings =
		{
			{ DescriptorType::CombinedImageSampler, 0, 1, ShaderStage::Fragment }
		};

		m_materialDescriptorSetLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);
	}

	// Descriptor pool
	{
		DescriptorPool::Settings descriptorPoolSettings;
		descriptorPoolSettings.layout = m_materialDescriptorSetLayout;
		descriptorPoolSettings.pageSize = 1;

		m_materialDescriptorPool = DescriptorPool::create(descriptorPoolSettings);
	}

	//----- FRAME RESOURCES -----//

	// Descriptor set layout
	{
		DescriptorSetLayout::Settings descriptorSetLayoutSettings;
		descriptorSetLayoutSettings.bindings =
		{
			{ DescriptorType::UniformBuffer, 0, 1, ShaderStage::Vertex },
			{ DescriptorType::UniformBufferDynamic, 1, 1, ShaderStage::Vertex }
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
	m_frameObjectsUniformBuffers.reserve(m_maxFramesInFlight);

	const size_t elementByteSize = sizeof(UniformObjectElement);
	const size_t minOffset = static_cast<size_t>(Renderer::instance().getLimits().minUniformBufferOffsetAlignment);

	if (minOffset >= elementByteSize)
	{
		m_dynamicObjectBufferOffset = minOffset;
	}
	else
	{
		m_dynamicObjectBufferOffset = minOffset;

		while (m_dynamicObjectBufferOffset < elementByteSize)
		{
			m_dynamicObjectBufferOffset += minOffset;
		}
	}

	for (uint32_t j = 0; j < m_maxFramesInFlight; j++)
	{
		// Frame uniform buffers
		auto frameUniformBuffer = Buffer::create({ BufferUsage::Uniform, sizeof(UniformFrameElement), true });

		m_frameUniformBuffers.push_back(frameUniformBuffer);

		// Frame object uniform buffers
		auto objectsUniformBuffer = Buffer::create({ BufferUsage::Uniform, objectCount * m_dynamicObjectBufferOffset, true });

		m_frameObjectsUniformBuffers.push_back(objectsUniformBuffer);

		// Add descriptor set
		auto descriptorSet = m_frameDescriptorPool->createSet();

		descriptorSet->update(0, frameUniformBuffer);
		descriptorSet->update(1, objectsUniformBuffer, sizeof(UniformObjectElement));

		m_frameDescriptorSets.push_back(descriptorSet);
	}

	//----- THREAD RESOURCES -----//
	auto& taskManager = TaskManager::instance();
	const auto coreCount = taskManager.getSize();

	m_threadCommandBuffers.resize(coreCount);

	for (auto& commandBuffers : m_threadCommandBuffers)
		commandBuffers.resize(m_maxFramesInFlight);

	// Create size dependent resources
	onResize(window->getSize());
}

GraphicsSystem::~GraphicsSystem()
{
	Renderer::instance().waitForIdle();

	m_renderPipeline.reset();

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

	// Frame resources
	m_frameDescriptorSetLayout.reset();
	m_frameDescriptorSets.clear();
	m_frameDescriptorPool.reset();
	m_frameUniformBuffers.clear();

	// Object resources
	m_materialDescriptorSet.reset();
	m_materialDescriptorSetLayout.reset();
	m_materialDescriptorPool.reset();

	m_pipeline.reset();
}

void GraphicsSystem::update(TimeStep timeStep)
{
	// Ensure object descriptor frame exists
	if (!m_materialDescriptorSet)
	{
		auto sparseUnion = getEntityManager().getUnion<GraphicsComponent>();

		if (sparseUnion.size() > 0)
		{
			auto& graphics = sparseUnion.get<GraphicsComponent>(*sparseUnion.begin());

			m_materialDescriptorSet = m_materialDescriptorPool->createSet();
			m_materialDescriptorSet->update(0, graphics.texture, graphics.sampler);
		}
	}
	
	// Start frame
	m_renderPipeline->startFrame();

	// Update total time
	m_totalTime += timeStep.getSeconds();
}

void GraphicsSystem::onResize(const Vector2u& size)
{
	// Create color images
	{
		m_deferredImages.clear();

		for (auto& format : gBuffer)
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
		for (uint32_t i = 0; i < gBuffer.size(); i++)
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
		pipelineSettings.renderPass = m_renderPipeline->getRenderPass();
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
		pipelineSettings.descriptorSetLayouts = { m_frameDescriptorSetLayout, m_materialDescriptorSetLayout };
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

void GraphicsSystem::onUpdateFrame(uint32_t frameIndex, Ptr<CommandBuffer> commandBuffer)
{
	// Update scene data
	updateUniformBuffers(frameIndex);

	// Deferred pass
	std::vector<CommandBuffer::ClearValue> clearValues;

	for (auto& format : gBuffer)
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

		auto& entityManager = getEntityManager();

		auto entities = entityManager.getUnion<Transform, GraphicsComponent>();

		auto& taskManager = TaskManager::instance();

		std::vector<Ptr<Task>> tasks;
		tasks.reserve(threadCount);

		std::vector<Ptr<CommandBuffer>> commandBuffers;
		commandBuffers.resize(threadCount);

		size_t firstIndex = 0;
		const size_t size = entities.size() / threadCount;

		for (size_t taskIndex = 0; taskIndex < threadCount; taskIndex++)
		{
			auto lastIndex = firstIndex + size;

			if (taskIndex == threadCount - 1)
			{
				const auto remainingSize = entities.size() - lastIndex;

				lastIndex += remainingSize;
			}

			auto task = taskManager.createTask([this, taskIndex, firstIndex, lastIndex, &entities, &commandBuffers, globalDescriptorSet, frameIndex](size_t threadIndex)
				{
					auto commandPool = Renderer::instance().getCommandPool(threadIndex);
					auto commandBuffer = commandPool->createBuffer({ true, true });

					commandBuffer->beginSecondary(m_deferredRenderPass, m_deferredFramebuffer);

					commandBuffer->bindPipeline(m_pipeline);

					commandBuffer->bindDescriptorSet(1, m_materialDescriptorSet);

					size_t i = firstIndex;
					for (auto it = entities.begin() + firstIndex; it != entities.begin() + lastIndex; it++)
					{
						auto& graphics = entities.get<GraphicsComponent>(*it);
						
						commandBuffer->bindVertexBuffer(graphics.vertexBuffer, 0);

						commandBuffer->bindIndexBuffer(graphics.indexBuffer, IndexType::U32);
						
						commandBuffer->bindDescriptorSet(0, globalDescriptorSet, { i * m_dynamicObjectBufferOffset });

						commandBuffer->drawIndexed(graphics.indexCount);

						i++;
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
	m_renderPipeline->beginScreenRenderPass();

	commandBuffer->bindPipeline(m_ppPipeline);

	commandBuffer->bindVertexBuffer(m_ppQuad, 0);

	commandBuffer->bindDescriptorSet(0, m_ppDescriptorSet);

	commandBuffer->draw(quadVertices.size());

	m_renderPipeline->endScreenRenderPass();
}

void GraphicsSystem::updateUniformBuffers(uint32_t frameIndex)
{
	ATEMA_BENCHMARK("GraphicsSystem::updateUniformBuffers");

	// Update global buffers
	{
		auto selection = getEntityManager().getUnion<Transform, CameraComponent>();

		for (auto& entity : selection)
		{
			auto& camera = selection.get<CameraComponent>(entity);

			if (camera.display)
			{
				auto& transform = selection.get<Transform>(entity);

				const Vector3f cameraPos = transform.getTranslation();
				const Vector3f cameraUp(0.0f, 0.0f, 1.0f);
				
				Vector3f cameraTarget = camera.target;

				// If the camera doesn't use target, calculate target from transform rotation
				if (!camera.useTarget)
				{
					cameraTarget = cameraPos + Matrix4f::createRotation(transform.getRotation()) * Vector3f(1.0f, 0.0f, 0.0f);
				}

				UniformFrameElement frameTransforms;
				frameTransforms.view = Matrix4f::createLookAt(cameraPos, cameraTarget, cameraUp);
				frameTransforms.proj = Matrix4f::createPerspective(toRadians(camera.fov), camera.aspectRatio, camera.nearPlane, camera.farPlane);
				frameTransforms.proj[1][1] *= -1;

				auto buffer = m_frameUniformBuffers[frameIndex];

				void* data = buffer->map();

				memcpy(data, static_cast<void*>(&frameTransforms), sizeof(UniformFrameElement));

				buffer->unmap();

				break;
			}
		}
	}

	// Update objects buffers
	{
		auto data = static_cast<uint8_t*>(m_frameObjectsUniformBuffers[frameIndex]->map());

		const auto basisChange = Matrix4f::createRotation({ toRadians(90.0f), 0.0f, 0.0f });

		auto& entityManager = getEntityManager();

		auto entities = entityManager.getUnion<Transform, GraphicsComponent>();

		auto& taskManager = TaskManager::instance();

		// Divide the updates in max groups
		std::vector<Ptr<Task>> tasks;
		tasks.reserve(threadCount);

		size_t firstIndex = 0;
		size_t size = entities.size() / threadCount;

		for (size_t i = 0; i < threadCount; i++)
		{
			auto lastIndex = firstIndex + size;

			if (i == threadCount - 1)
			{
				const auto remainingSize = entities.size() - lastIndex;

				lastIndex += remainingSize;
			}

			auto task = taskManager.createTask([this, data, basisChange, firstIndex, lastIndex, &entities, frameIndex]()
				{
					auto it = entities.begin() + firstIndex;

					for (size_t j = firstIndex; j < lastIndex; j++)
					{
						auto& transform = entities.get<Transform>(*it);

						UniformObjectElement objectTransforms;
						objectTransforms.model = transform.getMatrix() * basisChange;

						memcpy(static_cast<void*>(&data[j * m_dynamicObjectBufferOffset]), static_cast<void*>(&objectTransforms), sizeof(UniformObjectElement));

						it++;
					}
				});

			tasks.push_back(task);

			firstIndex += size;
		}

		for (auto& task : tasks)
			task->wait();

		m_frameObjectsUniformBuffers[frameIndex]->unmap();
	}
}
