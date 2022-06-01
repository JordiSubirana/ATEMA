/*
	Copyright 2022 Jordi SUBIRANA

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

#include <Atema/Window/WindowResizeEvent.hpp>

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

GraphicsSystem::GraphicsSystem(const Ptr<RenderWindow>& renderWindow) :
	System(),
	m_renderWindow(renderWindow),
	m_totalTime(0.0f)
{
	ATEMA_ASSERT(renderWindow, "Invalid RenderWindow");

	auto& renderer = Renderer::instance();

	const auto maxFramesInFlight = renderWindow->getMaxFramesInFlight();

	m_depthFormat = renderWindow->getDepthFormat();

	//----- DEFERRED RESOURCES -----//
	// Create RenderPass
	{
		RenderPass::Settings renderPassSettings;
		renderPassSettings.subpasses.resize(1);

		uint32_t attachmentIndex = 0;
		
		// Color attachments
		for (auto& format : gBuffer)
		{
			AttachmentDescription attachment;
			attachment.format = format;
			attachment.initialLayout = ImageLayout::Undefined;
			attachment.finalLayout = ImageLayout::ShaderRead;

			renderPassSettings.attachments.push_back(attachment);
			
			renderPassSettings.subpasses[0].color.push_back(attachmentIndex++);
		}

		// Depth attachment
		AttachmentDescription attachment;
		attachment.format = m_depthFormat;

		renderPassSettings.attachments.push_back(attachment);

		renderPassSettings.subpasses[0].depthStencil = attachmentIndex;

		// Create RenderPass
		m_renderPass = RenderPass::create(renderPassSettings);
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

	// Create post process pipeline
	{
		GraphicsPipeline::Settings pipelineSettings;
		pipelineSettings.vertexShader = Shader::create({ ppOutputColorVS });
		pipelineSettings.fragmentShader = Shader::create({ ppOutputColorFS });
		pipelineSettings.renderPass = renderWindow->getRenderPass();
		pipelineSettings.descriptorSetLayouts = { m_ppDescriptorSetLayout };
		// Position / TexCoords
		pipelineSettings.vertexInput.inputs =
		{
			{ 0, 0, VertexFormat::RGB32_SFLOAT },
			{ 0, 1, VertexFormat::RG32_SFLOAT }
		};

		m_ppPipeline = GraphicsPipeline::create(pipelineSettings);
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
		m_ppQuad = createQuad(renderer.getCommandPool(QueueType::Graphics));
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
		descriptorPoolSettings.pageSize = maxFramesInFlight;

		m_frameDescriptorPool = DescriptorPool::create(descriptorPoolSettings);
	}

	// Gbuffer pipeline
	{
		GraphicsPipeline::Settings pipelineSettings;
		pipelineSettings.vertexShader = Shader::create({ gbufferPassVS });
		pipelineSettings.fragmentShader = Shader::create({ gbufferPassFS });
		pipelineSettings.renderPass = m_renderPass;
		pipelineSettings.descriptorSetLayouts = { m_frameDescriptorSetLayout, m_materialDescriptorSetLayout };
		pipelineSettings.vertexInput.inputs =
		{
			{ 0, 0, VertexFormat::RGB32_SFLOAT },
			{ 0, 1, VertexFormat::RGB32_SFLOAT },
			{ 0, 2, VertexFormat::RG32_SFLOAT }
		};

		m_gbufferPipeline = GraphicsPipeline::create(pipelineSettings);
	}

	// Uniform buffers & descriptor sets
	const uint32_t elementByteSize = sizeof(UniformObjectElement);
	const uint32_t minOffset = static_cast<uint32_t>(Renderer::instance().getLimits().minUniformBufferOffsetAlignment);

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

	m_frameDatas.resize(maxFramesInFlight);
	for (auto& frameData : m_frameDatas)
	{
		// Frame uniform buffers
		frameData.frameUniformBuffer = Buffer::create({ BufferUsage::Uniform, sizeof(UniformFrameElement), true });

		// Frame object uniform buffers
		frameData.objectsUniformBuffer = Buffer::create({ BufferUsage::Uniform, static_cast<size_t>(objectCount * m_dynamicObjectBufferOffset), true });

		// Add descriptor set
		frameData.descriptorSet = m_frameDescriptorPool->createSet();
		frameData.descriptorSet->update(0, frameData.frameUniformBuffer);
		frameData.descriptorSet->update(1, frameData.objectsUniformBuffer, sizeof(UniformObjectElement));
	}

	//----- THREAD RESOURCES -----//
	m_threadCommandBuffers.resize(TaskManager::instance().getSize());

	for (auto& threadCommandBuffers : m_threadCommandBuffers)
		threadCommandBuffers.resize(maxFramesInFlight);

	//----- MISC -----//

	// Create size dependent resources
	onResize(renderWindow->getSize());
}

GraphicsSystem::~GraphicsSystem()
{
	Renderer::instance().waitForIdle();

	// Rendering resources
	m_ppDescriptorSetLayout.reset();
	m_ppDescriptorSet.reset();
	m_ppDescriptorPool.reset();

	m_renderPass.reset();

	m_gbufferFramebuffer.reset();
	m_gbufferDepthImage.reset();
	m_gbufferImages.clear();

	m_ppSampler.reset();

	m_ppPipeline.reset();

	// Frame resources
	m_frameDescriptorSetLayout.reset();
	m_frameDescriptorPool.reset();
	m_frameDatas.clear();

	// Object resources
	m_materialDescriptorSet.reset();
	m_materialDescriptorSetLayout.reset();
	m_materialDescriptorPool.reset();

	m_gbufferPipeline.reset();
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
	updateFrame();

	// Update total time
	m_totalTime += timeStep.getSeconds();
}

void GraphicsSystem::onEvent(Event& event)
{
	if (event.is<WindowResizeEvent>())
	{
		const auto& windowResizeEvent = static_cast<WindowResizeEvent&>(event);

		onResize(windowResizeEvent.size);
	}
}

void GraphicsSystem::onResize(const Vector2u& size)
{
	Renderer::instance().waitForIdle();

	// Create color images
	{
		m_gbufferImages.clear();

		for (auto& format : gBuffer)
		{
			Image::Settings imageSettings;
			imageSettings.format = format;
			imageSettings.width = size.x;
			imageSettings.height = size.y;
			imageSettings.usages = ImageUsage::RenderTarget | ImageUsage::ShaderRead;

			auto image = Image::create(imageSettings);

			m_gbufferImages.push_back(image);
		}
	}

	// Create depth image
	{
		Image::Settings imageSettings;
		imageSettings.format = m_depthFormat;
		imageSettings.width = size.x;
		imageSettings.height = size.y;
		imageSettings.usages = ImageUsage::RenderTarget;

		m_gbufferDepthImage = Image::create(imageSettings);
	}

	// Create gbuffer framebuffer
	{
		Framebuffer::Settings framebufferSettings;
		framebufferSettings.width = size.x;
		framebufferSettings.height = size.y;
		framebufferSettings.images = m_gbufferImages;
		framebufferSettings.images.push_back(m_gbufferDepthImage);
		framebufferSettings.renderPass = m_renderPass;

		m_gbufferFramebuffer = Framebuffer::create(framebufferSettings);
	}

	// Write descriptor set
	{
		for (uint32_t i = 0; i < gBuffer.size(); i++)
			m_ppDescriptorSet->update(i, m_gbufferImages[i], m_ppSampler);
	}

	m_viewport.size.x = static_cast<float>(size.x);
	m_viewport.size.y = static_cast<float>(size.y);
	m_windowSize = size;

	// Clear command buffers
	for (auto& threadCommandBuffers : m_threadCommandBuffers)
		for (auto& commandBuffers : threadCommandBuffers)
			commandBuffers.clear();

	for (auto& frameData : m_frameDatas)
		frameData.commandBuffer.reset();
}

void GraphicsSystem::updateFrame()
{
	RenderFrame* _frame = nullptr;

	{
		ATEMA_BENCHMARK("RenderWindow::acquireFrame");

		_frame = &m_renderWindow.lock()->acquireFrame();
	}

	auto& renderFrame = *_frame;

	const auto frameIndex = renderFrame.getFrameIndex();

	auto& frameData = m_frameDatas[frameIndex];

	// Update scene data
	updateUniformBuffers(frameData);

	// CommandBuffer
	CommandBuffer::Settings commandBufferSettings;
	commandBufferSettings.singleUse = true;

	// Save command buffer
	frameData.commandBuffer = renderFrame.createCommandBuffer(commandBufferSettings, QueueType::Graphics);

	// Local variable for convenience
	auto& commandBuffer = frameData.commandBuffer;
	commandBuffer->begin();

	// Deferred pass
	std::vector<CommandBuffer::ClearValue> gBufferClearValues;

	for (auto& format : gBuffer)
		gBufferClearValues.push_back({ 0.0f, 0.0f, 0.0f, 1.0f });

	gBufferClearValues.push_back({ 1.0f, 0 });

	commandBuffer->beginRenderPass(m_renderPass, m_gbufferFramebuffer, gBufferClearValues, true);

	// Update objects buffers
	{
		ATEMA_BENCHMARK("CommandBuffer (scene)");

		// Clear previous command buffers
		for (auto& threadCommandBuffers : m_threadCommandBuffers)
			threadCommandBuffers[frameIndex].clear();

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

			auto task = taskManager.createTask([this, &renderFrame, taskIndex, firstIndex, lastIndex, &entities, &commandBuffers, &frameData, frameIndex](size_t threadIndex)
				{
					auto commandBuffer = renderFrame.createCommandBuffer({ true, true }, QueueType::Graphics, threadIndex);

					commandBuffer->beginSecondary(m_renderPass, m_gbufferFramebuffer);

					commandBuffer->bindPipeline(m_gbufferPipeline);

					commandBuffer->setViewport(m_viewport);

					commandBuffer->setScissor(Vector2i(), m_windowSize);

					commandBuffer->bindDescriptorSet(1, m_materialDescriptorSet);

					uint32_t i = static_cast<uint32_t>(firstIndex);
					for (auto it = entities.begin() + firstIndex; it != entities.begin() + lastIndex; it++)
					{
						auto& graphics = entities.get<GraphicsComponent>(*it);
						
						commandBuffer->bindVertexBuffer(graphics.vertexBuffer, 0);

						commandBuffer->bindIndexBuffer(graphics.indexBuffer, IndexType::U32);
						
						commandBuffer->bindDescriptorSet(0, frameData.descriptorSet, { i * m_dynamicObjectBufferOffset });

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

		commandBuffer->endRenderPass();

		for (auto& image : m_gbufferImages)
		{
			commandBuffer->imageBarrier(
				image,
				PipelineStage::ColorAttachmentOutput, MemoryAccess::ColorAttachmentWrite, ImageLayout::ShaderRead,
				PipelineStage::FragmentShader, MemoryAccess::ShaderRead, ImageLayout::ShaderRead);
		}
	}

	// Post process pass
	{
		ATEMA_BENCHMARK("CommandBuffer (post process)");

		const std::vector<CommandBuffer::ClearValue> postProcessClearValues =
		{
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			{ 1.0f, 0 }
		};

		commandBuffer->beginRenderPass(renderFrame.getRenderPass(), renderFrame.getFramebuffer(), postProcessClearValues, false);

		commandBuffer->bindPipeline(m_ppPipeline);

		commandBuffer->setViewport(m_viewport);

		commandBuffer->setScissor(Vector2i(), m_windowSize);

		commandBuffer->bindVertexBuffer(m_ppQuad, 0);

		commandBuffer->bindDescriptorSet(0, m_ppDescriptorSet);

		commandBuffer->draw(static_cast<uint32_t>(quadVertices.size()));

		commandBuffer->endRenderPass();

		commandBuffer->end();
	}

	renderFrame.getFence()->reset();

	{
		ATEMA_BENCHMARK("RenderFrame::submit");

		renderFrame.submit(
			{ commandBuffer },
			{ renderFrame.getImageAvailableWaitCondition() },
			{ renderFrame.getRenderFinishedSemaphore() },
			renderFrame.getFence());
	}

	{
		ATEMA_BENCHMARK("RenderFrame::present");

		renderFrame.present();
	}
}

void GraphicsSystem::updateUniformBuffers(FrameData& frameData)
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
				frameTransforms.proj = Matrix4f::createPerspective(Math::toRadians(camera.fov), camera.aspectRatio, camera.nearPlane, camera.farPlane);
				frameTransforms.proj[1][1] *= -1;

				void* data = frameData.frameUniformBuffer->map();

				memcpy(data, static_cast<void*>(&frameTransforms), sizeof(UniformFrameElement));

				frameData.frameUniformBuffer->unmap();

				break;
			}
		}
	}

	// Update objects buffers
	{
		auto data = static_cast<uint8_t*>(frameData.objectsUniformBuffer->map());

		const auto basisChange = Matrix4f::createRotation({ Math::toRadians(90.0f), 0.0f, 0.0f });

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

			auto task = taskManager.createTask([this, data, basisChange, firstIndex, lastIndex, &entities]()
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

		frameData.objectsUniformBuffer->unmap();
	}
}
