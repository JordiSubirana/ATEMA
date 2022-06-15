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

#include <fstream>
#include <Atema/Graphics/FrameGraphBuilder.hpp>
#include <Atema/Graphics/FrameGraphContext.hpp>
#include <Atema/Renderer/Buffer.hpp>
#include <Atema/Renderer/GraphicsPipeline.hpp>
#include <Atema/Window/WindowResizeEvent.hpp>

#include "../Resources.hpp"
#include "../Components/GraphicsComponent.hpp"
#include "../Components/CameraComponent.hpp"

using namespace at;

namespace
{
	const auto shaderPath = rscPath / "Shaders";

	const auto gbufferVS = shaderPath / "GBufferVS.spv";
	const auto gbufferFS = shaderPath / "GBufferFS.spv";
	const auto postProcessVS = shaderPath / "PostProcessVS.spv";
	const auto phongLightingFS = shaderPath / "PhongLightingFS.spv";
	const auto screenFS = shaderPath / "ScreenFS.spv";

	const std::vector<std::filesystem::path> shaderPaths =
	{
		gbufferVS,
		gbufferFS,
		postProcessVS,
		phongLightingFS,
		screenFS
	};

	constexpr size_t targetThreadCount = 8;

	const size_t threadCount = std::min(targetThreadCount, TaskManager::instance().getSize());

	const std::vector<ImageFormat> gBuffer =
	{
		// Position
		ImageFormat::RGBA32_SFLOAT,
		// Normal
		ImageFormat::RGBA32_SFLOAT,
		// Color
		ImageFormat::RGBA8_UNORM,
		// Ambient occlusion
		ImageFormat::RGBA8_UNORM,
		// Emissive
		ImageFormat::RGBA8_UNORM,
		// Metalness
		ImageFormat::RGBA8_UNORM,
		// Roughness
		ImageFormat::RGBA8_UNORM,
	};
	
	struct PPVertex
	{
		Vector3f position;
		Vector2f texCoords;
	};

	std::vector<VertexInput> postProcessVertexInput =
	{
		{ 0, 0, VertexFormat::RGB32_SFLOAT },
		{ 0, 1, VertexFormat::RG32_SFLOAT }
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

	struct UniformPhongLightingData
	{
		Vector4f lightPosition;
		Vector4f lightColor;
		float ambientStrength;
	};
}

GraphicsSystem::GraphicsSystem(const Ptr<RenderWindow>& renderWindow) :
	System(),
	m_renderWindow(renderWindow),
	m_totalTime(0.0f)
{
	ATEMA_ASSERT(renderWindow, "Invalid RenderWindow");

	translateShaders();

	auto& renderer = Renderer::instance();

	const auto maxFramesInFlight = renderWindow->getMaxFramesInFlight();

	m_depthFormat = renderWindow->getDepthFormat();

	//----- SHADERS -----//
	std::unordered_map<std::string, Ptr<Shader>> shaders;

	for (auto& path : shaderPaths)
		shaders[path.string()] = Shader::create({ path });

	//----- DEFERRED RESOURCES -----//
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
			{ DescriptorType::CombinedImageSampler, 2, 1, ShaderStage::Fragment },
			{ DescriptorType::CombinedImageSampler, 3, 1, ShaderStage::Fragment },
			{ DescriptorType::CombinedImageSampler, 4, 1, ShaderStage::Fragment },
			{ DescriptorType::CombinedImageSampler, 5, 1, ShaderStage::Fragment },
			{ DescriptorType::CombinedImageSampler, 6, 1, ShaderStage::Fragment }
		};
		descriptorSetLayoutSettings.pageSize = 1;

		m_ppDescriptorSetLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);
	}

	// Create screen descriptor set layout
	{
		DescriptorSetLayout::Settings descriptorSetLayoutSettings;
		descriptorSetLayoutSettings.bindings =
		{
			{ DescriptorType::CombinedImageSampler, 0, 1, ShaderStage::Fragment }
		};
		descriptorSetLayoutSettings.pageSize = 1;

		m_screenDescriptorSetLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);
	}

	// Create screen pipeline
	{
		GraphicsPipeline::Settings pipelineSettings;
		pipelineSettings.vertexShader = shaders[postProcessVS.string()];
		pipelineSettings.fragmentShader = shaders[screenFS.string()];
		pipelineSettings.descriptorSetLayouts = { m_screenDescriptorSetLayout };
		// Position / TexCoords
		pipelineSettings.vertexInput.inputs = postProcessVertexInput;

		m_screenPipeline = GraphicsPipeline::create(pipelineSettings);
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
			{ DescriptorType::CombinedImageSampler, 0, 1, ShaderStage::Fragment },
			{ DescriptorType::CombinedImageSampler, 1, 1, ShaderStage::Fragment },
			{ DescriptorType::CombinedImageSampler, 2, 1, ShaderStage::Fragment },
			{ DescriptorType::CombinedImageSampler, 3, 1, ShaderStage::Fragment },
			{ DescriptorType::CombinedImageSampler, 4, 1, ShaderStage::Fragment },
			{ DescriptorType::CombinedImageSampler, 5, 1, ShaderStage::Fragment }
		};
		descriptorSetLayoutSettings.pageSize = 1;

		m_materialDescriptorSetLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);
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
		descriptorSetLayoutSettings.pageSize = maxFramesInFlight;

		m_frameDescriptorSetLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);
	}

	// Gbuffer pipeline
	{
		GraphicsPipeline::Settings pipelineSettings;
		pipelineSettings.vertexShader = shaders[gbufferVS.string()];
		pipelineSettings.fragmentShader = shaders[gbufferFS.string()];
		pipelineSettings.descriptorSetLayouts = { m_frameDescriptorSetLayout, m_materialDescriptorSetLayout };
		pipelineSettings.vertexInput.inputs =
		{
			{ 0, 0, VertexFormat::RGB32_SFLOAT },
			{ 0, 1, VertexFormat::RGB32_SFLOAT },
			{ 0, 2, VertexFormat::RGB32_SFLOAT },
			{ 0, 3, VertexFormat::RGB32_SFLOAT },
			{ 0, 4, VertexFormat::RG32_SFLOAT }
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
		frameData.descriptorSet = m_frameDescriptorSetLayout->createSet();
		frameData.descriptorSet->update(0, frameData.frameUniformBuffer);
		frameData.descriptorSet->update(1, frameData.objectsUniformBuffer, sizeof(UniformObjectElement));
	}

	//----- PHONG LIGHTING -----//

	// Fill pass data buffer
	{
		UniformPhongLightingData passData;
		passData.lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		passData.lightPosition = { 0.0f, 10.0f, 10.0f, 1.0f };
		passData.ambientStrength = 0.1f;

		Buffer::Settings bufferSettings;
		bufferSettings.usage = BufferUsage::Uniform;
		bufferSettings.byteSize = sizeof(passData);
		bufferSettings.mappable = true;

		m_phongBufferData = Buffer::create(bufferSettings);

		void* data = m_phongBufferData->map();

		memcpy(data, static_cast<void*>(&passData), sizeof(passData));

		m_phongBufferData->unmap();
	}

	{
		DescriptorSetLayout::Settings descriptorSetLayoutSettings;
		descriptorSetLayoutSettings.pageSize = 1;
		descriptorSetLayoutSettings.bindings =
		{
			{ DescriptorType::UniformBuffer, 0, 1, ShaderStage::Fragment }
		};

		m_phongDescriptorSetLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);

		m_phongDescriptorSet = m_phongDescriptorSetLayout->createSet();

		m_phongDescriptorSet->update(0, m_phongBufferData);
	}

	{
		GraphicsPipeline::Settings graphicsPipelineSettings;
		graphicsPipelineSettings.vertexShader = shaders[postProcessVS.string()];
		graphicsPipelineSettings.fragmentShader = shaders[phongLightingFS.string()];
		graphicsPipelineSettings.descriptorSetLayouts = { m_ppDescriptorSetLayout, m_phongDescriptorSetLayout };
		graphicsPipelineSettings.vertexInput.inputs = postProcessVertexInput;

		m_phongPipeline = GraphicsPipeline::create(graphicsPipelineSettings);
	}

	//----- MISC -----//

	// Create size dependent resources
	onResize(renderWindow->getSize());
}

GraphicsSystem::~GraphicsSystem()
{
	Renderer::instance().waitForIdle();

	// Rendering resources
	m_ppDescriptorSetLayout.reset();

	m_ppSampler.reset();

	m_screenDescriptorSetLayout.reset();

	m_screenPipeline.reset();

	// Frame resources
	m_frameDescriptorSetLayout.reset();
	m_frameDatas.clear();

	// Object resources
	m_materialDescriptorSet.reset();
	m_materialDescriptorSetLayout.reset();

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

			m_materialDescriptorSet = m_materialDescriptorSetLayout->createSet();
			m_materialDescriptorSet->update(0, graphics.color, graphics.sampler);
			m_materialDescriptorSet->update(1, graphics.normal, graphics.sampler);
			m_materialDescriptorSet->update(2, graphics.ambientOcclusion, graphics.sampler);
			m_materialDescriptorSet->update(3, graphics.emissive, graphics.sampler);
			m_materialDescriptorSet->update(4, graphics.metalness, graphics.sampler);
			m_materialDescriptorSet->update(5, graphics.roughness, graphics.sampler);
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

void GraphicsSystem::translateShaders()
{
	ATEMA_BENCHMARK("GraphicsSystem::translateShaders")

	std::filesystem::directory_iterator shaderDir(shaderPath);

	for (const auto& entry : shaderDir)
	{
		auto& path = entry.path();

		if (path.extension() == ".atsl")
		{
			std::cout << "Parsing shader : " << path.filename() << std::endl;

			std::stringstream code;

			{
				std::ifstream file(path);

				if (!file.is_open())
				{
					ATEMA_ERROR("Failed to open file '" + path.string() + "'");
				}

				code << file.rdbuf();
			}

			AtslParser parser;

			const auto atslTokens = parser.createTokens(code.str());

			AtslToAstConverter converter;

			auto ast = converter.createAst(atslTokens);

			std::vector<AstShaderStage> shaderStages = { AstShaderStage::Vertex, AstShaderStage::Fragment };
			std::vector<std::string> glslSuffixes = { ".vert", ".frag" };
			std::vector<std::string> spirvSuffixes = { "VS.spv", "FS.spv" };

			for (size_t i = 0; i < shaderStages.size(); i++)
			{
				const auto& shaderStage = shaderStages[i];
				const auto& glslSuffix = glslSuffixes[i];
				const auto& spirvSuffix = spirvSuffixes[i];

				AstStageExtractor stageExtractor;

				ast->accept(stageExtractor);

				try
				{
					auto stageAst = stageExtractor.getAst(shaderStage);

					{
						std::ofstream file(shaderPath / (path.stem().string() + glslSuffix));

						GlslShaderWriter glslWriter(shaderStage, file);

						stageAst->accept(glslWriter);
					}

					{
						std::ofstream file(shaderPath / (path.stem().string() + spirvSuffix), std::ios::binary);

						SpirvShaderWriter spvWriter(shaderStage);

						stageAst->accept(spvWriter);

						spvWriter.compile(file);
					}
				}
				catch (const std::exception& e)
				{
					std::cout << e.what() << std::endl;
				}
				catch (...)
				{
					
				}
			}
		}
	}
}

void GraphicsSystem::createFrameGraph()
{
	FrameGraphTextureSettings textureSettings;
	textureSettings.width = m_windowSize.x;
	textureSettings.height = m_windowSize.y;

	FrameGraphBuilder frameGraphBuilder;

	//-----
	// Texture setup
	std::vector<FrameGraphTextureHandle> gbufferTextures;
	gbufferTextures.reserve(gBuffer.size());

	for (auto& format : gBuffer)
	{
		textureSettings.format = format;

		auto texture = frameGraphBuilder.createTexture(textureSettings);

		gbufferTextures.emplace_back(texture);
	}

	textureSettings.format = m_depthFormat;
	auto gbufferDepthTexture = frameGraphBuilder.createTexture(textureSettings);

	textureSettings.format = gBuffer[0];
	auto phongOutputTexture = frameGraphBuilder.createTexture(textureSettings);

	//-----
	// Pass setup

	// Geometry
	{
		auto& pass = frameGraphBuilder.createPass("geometry");

		uint32_t gbufferTextureIndex = 0;
		for (const auto& texture : gbufferTextures)
			pass.addOutputTexture(texture, gbufferTextureIndex++, Color::Black);

		pass.setDepthTexture(gbufferDepthTexture, DepthStencil(1.0f, 0));

		pass.enableSecondaryCommandBuffers(true);

		pass.setExecutionCallback([this](FrameGraphContext& context)
			{
				ATEMA_BENCHMARK("CommandBuffer (scene)");

				const auto frameIndex = context.getFrameIndex();
				auto& frameData = m_frameDatas[frameIndex];

				auto& commandBuffer = context.getCommandBuffer();

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

					auto task = taskManager.createTask([this, &context, taskIndex, firstIndex, lastIndex, &entities, &commandBuffers, &frameData, frameIndex](size_t threadIndex)
						{
							auto commandBuffer = context.createSecondaryCommandBuffer(threadIndex);

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

							commandBuffers[taskIndex] = commandBuffer;
						});

					tasks.push_back(task);

					firstIndex += size;
				}

				for (auto& task : tasks)
					task->wait();

				commandBuffer.executeSecondaryCommands(commandBuffers);
			});
	}

	// Phong Lighting
	{
		auto& pass = frameGraphBuilder.createPass("phong lighting");

		for (const auto& texture : gbufferTextures)
			pass.addSampledTexture(texture, ShaderStage::Fragment);

		pass.addOutputTexture(phongOutputTexture, 0, Color::Black);

		pass.setExecutionCallback([this, gbufferTextures, phongOutputTexture](FrameGraphContext& context)
			{
				ATEMA_BENCHMARK("CommandBuffer (phong lighting)");

				auto ppDescriptorSet = m_ppDescriptorSetLayout->createSet();

				// Write descriptor set
				{
					for (uint32_t i = 0; i < gbufferTextures.size(); i++)
					{
						auto gbufferImage = context.getTexture(gbufferTextures[i]);
						ppDescriptorSet->update(i, gbufferImage, m_ppSampler);
					}
				}

				auto& commandBuffer = context.getCommandBuffer();

				commandBuffer.bindPipeline(m_phongPipeline);

				commandBuffer.setViewport(m_viewport);

				commandBuffer.setScissor(Vector2i(), m_windowSize);

				commandBuffer.bindVertexBuffer(m_ppQuad, 0);

				commandBuffer.bindDescriptorSet(0, ppDescriptorSet);
				commandBuffer.bindDescriptorSet(1, m_phongDescriptorSet);

				commandBuffer.draw(static_cast<uint32_t>(quadVertices.size()));

				context.destroyAfterUse(std::move(ppDescriptorSet));
			});
	}

	// Screen
	{
		auto& pass = frameGraphBuilder.createPass("screen");

		pass.addSampledTexture(phongOutputTexture, ShaderStage::Fragment);

		pass.enableRenderFrameOutput(true);

		pass.setExecutionCallback([this, phongOutputTexture](FrameGraphContext& context)
			{
				ATEMA_BENCHMARK("CommandBuffer (screen)");

				auto descriptorSet = m_screenDescriptorSetLayout->createSet();

				descriptorSet->update(0, context.getTexture(phongOutputTexture), m_ppSampler);

				auto& commandBuffer = context.getCommandBuffer();

				commandBuffer.bindPipeline(m_screenPipeline);

				commandBuffer.setViewport(m_viewport);

				commandBuffer.setScissor(Vector2i(), m_windowSize);

				commandBuffer.bindVertexBuffer(m_ppQuad, 0);

				commandBuffer.bindDescriptorSet(0, descriptorSet);

				commandBuffer.draw(static_cast<uint32_t>(quadVertices.size()));

				context.destroyAfterUse(std::move(descriptorSet));
			});
	}

	//-----
	// Build frame graph
	m_frameGraph = frameGraphBuilder.build();
}

void GraphicsSystem::onResize(const Vector2u& size)
{
	Renderer::instance().waitForIdle();

	m_viewport.size.x = static_cast<float>(size.x);
	m_viewport.size.y = static_cast<float>(size.y);
	m_windowSize = size;

	createFrameGraph();
}

void GraphicsSystem::updateFrame()
{
	Benchmark benchmark("RenderWindow::acquireFrame");

	auto& renderFrame = m_renderWindow.lock()->acquireFrame();

	benchmark.stop();

	const auto frameIndex = renderFrame.getFrameIndex();

	auto& frameData = m_frameDatas[frameIndex];

	// Update scene data
	updateUniformBuffers(frameData);

	benchmark.start("FrameGraph::execute");

	m_frameGraph->execute(renderFrame);
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

			auto task = taskManager.createTask([this, data, firstIndex, lastIndex, &entities]()
				{
					auto it = entities.begin() + firstIndex;

					for (size_t j = firstIndex; j < lastIndex; j++)
					{
						auto& transform = entities.get<Transform>(*it);

						UniformObjectElement objectTransforms;
						objectTransforms.model = transform.getMatrix();

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
