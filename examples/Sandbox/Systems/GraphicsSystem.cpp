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
	const Vector3f lightPosition = { -20.0f, 0.0f, 20.0f };
	const Vector3f lightDirection = Vector3f(1.0f, 0.0f, -1.0f).normalize();

	const uint32_t shadowMapSize = 2048;

	//-----

	const auto shaderPath = rscPath / "Shaders";

	const auto gbufferVS = shaderPath / "GBufferVS.spv";
	const auto gbufferFS = shaderPath / "GBufferFS.spv";
	const auto shadowMapVS = shaderPath / "ShadowMapVS.spv";
	const auto shadowMapFS = shaderPath / "ShadowMapFS.spv";
	const auto postProcessVS = shaderPath / "PostProcessVS.spv";
	const auto phongLightingDirectionalVS = shaderPath / "PhongLightingDirectionalVS.spv";
	const auto phongLightingDirectionalFS = shaderPath / "PhongLightingDirectionalFS.spv";
	const auto screenFS = shaderPath / "ScreenFS.spv";

	const std::vector<std::filesystem::path> shaderPaths =
	{
		gbufferVS,
		gbufferFS,
		shadowMapVS,
		shadowMapFS,
		postProcessVS,
		phongLightingDirectionalVS,
		phongLightingDirectionalFS,
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

	const uint32_t shadowMapBindingIndex = static_cast<uint32_t>(gBuffer.size());

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

	Ptr<Buffer> createQuad(Ptr<CommandPool> commandPool, Vector2f center = { 0.0f, 0.0f }, Vector2f size = { 1.0f, 1.0f })
	{
		auto vertices = quadVertices;

		for (auto& vertex : vertices)
		{
			auto& pos = vertex.position;

			pos.x *= size.x;
			pos.y *= size.y;

			pos.x += center.x;
			pos.y += center.y;
		}

		// Fill staging buffer
		size_t bufferSize = sizeof(vertices[0]) * vertices.size();

		auto stagingBuffer = Buffer::create({ BufferUsage::Transfer, bufferSize, true });

		auto bufferData = stagingBuffer->map();

		memcpy(bufferData, static_cast<void*>(vertices.data()), static_cast<size_t>(bufferSize));

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
		Vector4f cameraPosition;
		Vector4f lightPosition;
		Vector4f lightColor;
		float ambientStrength;
	};

	struct UniformShadowMappingData
	{
		Matrix4f depthMVP;
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

	//m_depthFormat = renderWindow->getDepthFormat();
	m_depthFormat = ImageFormat::D32F_S8U;

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

	// Create Layout
	{
		DescriptorSetLayout::Settings descriptorSetLayoutSettings;
		descriptorSetLayoutSettings.pageSize = 1;
		
		for (uint32_t bindingIndex = 0; bindingIndex < gBuffer.size(); bindingIndex++)
			descriptorSetLayoutSettings.bindings.emplace_back(DescriptorType::CombinedImageSampler, bindingIndex, 1, ShaderStage::Fragment);

		m_ppLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);

		descriptorSetLayoutSettings.bindings.emplace_back(DescriptorType::CombinedImageSampler, shadowMapBindingIndex, 1, ShaderStage::Fragment);

		m_gbufferShadowMapLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);
	}

	// Create screen descriptor set layout
	{
		DescriptorSetLayout::Settings descriptorSetLayoutSettings;
		descriptorSetLayoutSettings.bindings =
		{
			{ DescriptorType::CombinedImageSampler, 0, 1, ShaderStage::Fragment }
		};
		descriptorSetLayoutSettings.pageSize = 1;

		m_screenLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);
	}

	// Create screen pipeline
	{
		GraphicsPipeline::Settings pipelineSettings;
		pipelineSettings.vertexShader = shaders[postProcessVS.string()];
		pipelineSettings.fragmentShader = shaders[screenFS.string()];
		pipelineSettings.descriptorSetLayouts = { m_screenLayout };
		// Position / TexCoords
		pipelineSettings.vertexInput.inputs = postProcessVertexInput;
		pipelineSettings.depth.test = false;
		pipelineSettings.depth.write = false;

		m_screenPipeline = GraphicsPipeline::create(pipelineSettings);
	}

	// Create quad
	{
		m_ppQuad = createQuad(renderer.getCommandPool(QueueType::Graphics));
		m_ppDebugQuad = createQuad(renderer.getCommandPool(QueueType::Graphics), { -0.75f, 0.75f }, { 0.25f, 0.25f });
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

		m_materialLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);
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

		m_frameLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);
	}

	// Gbuffer pipeline
	{
		GraphicsPipeline::Settings pipelineSettings;
		pipelineSettings.vertexShader = shaders[gbufferVS.string()];
		pipelineSettings.fragmentShader = shaders[gbufferFS.string()];
		pipelineSettings.descriptorSetLayouts = { m_frameLayout, m_materialLayout };
		pipelineSettings.vertexInput.inputs =
		{
			{ 0, 0, VertexFormat::RGB32_SFLOAT },
			{ 0, 1, VertexFormat::RGB32_SFLOAT },
			{ 0, 2, VertexFormat::RGB32_SFLOAT },
			{ 0, 3, VertexFormat::RGB32_SFLOAT },
			{ 0, 4, VertexFormat::RG32_SFLOAT }
		};
		pipelineSettings.stencil = true;
		pipelineSettings.stencilFront.compareOperation = CompareOperation::Equal;
		pipelineSettings.stencilFront.writeMask = 1;
		pipelineSettings.stencilFront.passOperation = StencilOperation::Replace;
		pipelineSettings.stencilFront.reference = 1;

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
		frameData.descriptorSet = m_frameLayout->createSet();
		frameData.descriptorSet->update(0, frameData.frameUniformBuffer);
		frameData.descriptorSet->update(1, frameData.objectsUniformBuffer, sizeof(UniformObjectElement));
	}

	//----- SHADOW MAPPING -----//

	// Fill pass data buffer
	m_shadowViewport.size = { shadowMapSize, shadowMapSize };

	{
		Buffer::Settings bufferSettings;
		bufferSettings.usage = BufferUsage::Uniform;
		bufferSettings.byteSize = sizeof(UniformShadowMappingData);
		bufferSettings.mappable = true;

		for (auto& frameData : m_frameDatas)
			frameData.shadowBuffer = Buffer::create(bufferSettings);
	}

	{
		DescriptorSetLayout::Settings descriptorSetLayoutSettings;
		descriptorSetLayoutSettings.pageSize = 1;
		descriptorSetLayoutSettings.bindings =
		{
			{ DescriptorType::UniformBuffer, 0, 1, ShaderStage::Vertex }
		};

		m_shadowLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);

		for (auto& frameData : m_frameDatas)
		{
			frameData.shadowSet = m_shadowLayout->createSet();

			frameData.shadowSet->update(0, frameData.shadowBuffer);
		}
	}

	{
		GraphicsPipeline::Settings graphicsPipelineSettings;
		graphicsPipelineSettings.vertexShader = shaders[shadowMapVS.string()];
		graphicsPipelineSettings.fragmentShader = shaders[shadowMapFS.string()];
		graphicsPipelineSettings.descriptorSetLayouts = { m_frameLayout, m_shadowLayout };
		graphicsPipelineSettings.vertexInput.inputs =
		{
			{ 0, 0, VertexFormat::RGB32_SFLOAT },
			{ 0, 1, VertexFormat::RGB32_SFLOAT },
			{ 0, 2, VertexFormat::RGB32_SFLOAT },
			{ 0, 3, VertexFormat::RGB32_SFLOAT },
			{ 0, 4, VertexFormat::RG32_SFLOAT }
		};;

		m_shadowPipeline = GraphicsPipeline::create(graphicsPipelineSettings);
	}

	{
		Sampler::Settings samplerSettings(SamplerFilter::Linear);
		samplerSettings.addressModeU = SamplerAddressMode::ClampToBorder;
		samplerSettings.addressModeV = SamplerAddressMode::ClampToBorder;
		samplerSettings.addressModeW = SamplerAddressMode::ClampToBorder;
		samplerSettings.borderColor = SamplerBorderColor::WhiteFloat;
		samplerSettings.enableCompare = true;
		samplerSettings.compareOperation = CompareOperation::Less;

		m_shadowMapSampler = Sampler::create(samplerSettings);
	}

	//----- PHONG LIGHTING -----//

	// Fill pass data buffer
	{
		Buffer::Settings bufferSettings;
		bufferSettings.usage = BufferUsage::Uniform;
		bufferSettings.byteSize = sizeof(UniformPhongLightingData);
		bufferSettings.mappable = true;

		for (auto& frameData : m_frameDatas)
			frameData.phongBuffer = Buffer::create(bufferSettings);
	}

	{
		DescriptorSetLayout::Settings descriptorSetLayoutSettings;
		descriptorSetLayoutSettings.pageSize = 1;
		descriptorSetLayoutSettings.bindings =
		{
			{ DescriptorType::UniformBuffer, 0, 1, ShaderStage::Fragment },
			{ DescriptorType::UniformBuffer, 1, 1, ShaderStage::Fragment }
		};

		m_phongLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);
	}

	{
		GraphicsPipeline::Settings graphicsPipelineSettings;
		//graphicsPipelineSettings.vertexShader = shaders[postProcessVS.string()];
		graphicsPipelineSettings.vertexShader = shaders[phongLightingDirectionalVS.string()];
		graphicsPipelineSettings.fragmentShader = shaders[phongLightingDirectionalFS.string()];
		graphicsPipelineSettings.descriptorSetLayouts = { m_gbufferShadowMapLayout, m_phongLayout };
		graphicsPipelineSettings.vertexInput.inputs = postProcessVertexInput;
		graphicsPipelineSettings.depth.test = false;
		graphicsPipelineSettings.depth.write = false;
		graphicsPipelineSettings.stencil = true;
		graphicsPipelineSettings.stencilFront.compareOperation = CompareOperation::Equal;
		graphicsPipelineSettings.stencilFront.compareMask = 1;
		graphicsPipelineSettings.stencilFront.reference = 1;

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
	m_ppLayout.reset();

	m_ppSampler.reset();

	m_screenPipeline.reset();
	m_screenLayout.reset();
	
	m_phongPipeline.reset();
	m_phongLayout.reset();

	m_shadowMapSampler.reset();
	m_shadowPipeline.reset();
	m_shadowLayout.reset();
	m_gbufferShadowMapLayout.reset();

	// Frame resources
	m_frameLayout.reset();
	m_frameDatas.clear();

	// Object resources
	m_materialSet.reset();
	m_materialLayout.reset();

	m_gbufferPipeline.reset();
}

void GraphicsSystem::update(TimeStep timeStep)
{
	// Ensure object descriptor frame exists
	if (!m_materialSet)
	{
		auto sparseUnion = getEntityManager().getUnion<GraphicsComponent>();

		if (sparseUnion.size() > 0)
		{
			auto& graphics = sparseUnion.get<GraphicsComponent>(*sparseUnion.begin());

			m_materialSet = m_materialLayout->createSet();
			m_materialSet->update(0, graphics.color, graphics.sampler);
			m_materialSet->update(1, graphics.normal, graphics.sampler);
			m_materialSet->update(2, graphics.ambientOcclusion, graphics.sampler);
			m_materialSet->update(3, graphics.emissive, graphics.sampler);
			m_materialSet->update(4, graphics.metalness, graphics.sampler);
			m_materialSet->update(5, graphics.roughness, graphics.sampler);
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

	textureSettings.format = ImageFormat::D32F;
	textureSettings.width = shadowMapSize;
	textureSettings.height = shadowMapSize;
	auto shadowMapTexture = frameGraphBuilder.createTexture(textureSettings);

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

							commandBuffer->bindDescriptorSet(1, m_materialSet);

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

	// Shadow Map
	{
		auto& pass = frameGraphBuilder.createPass("shadow map");

		pass.setDepthTexture(shadowMapTexture, DepthStencil(1.0f, 0));

		pass.enableSecondaryCommandBuffers(true);

		pass.setExecutionCallback([this](FrameGraphContext& context)
			{
				ATEMA_BENCHMARK("CommandBuffer (shadow map)");

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

							commandBuffer->bindPipeline(m_shadowPipeline);

							commandBuffer->setViewport(m_shadowViewport);

							commandBuffer->setScissor(Vector2i(), { shadowMapSize, shadowMapSize });

							commandBuffer->bindDescriptorSet(1, frameData.shadowSet);

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

		pass.addSampledTexture(shadowMapTexture, ShaderStage::Fragment);

		pass.addOutputTexture(phongOutputTexture, 0, Color::Blue);
		pass.setDepthTexture(gbufferDepthTexture);

		pass.setExecutionCallback([this, gbufferTextures, shadowMapTexture](FrameGraphContext& context)
			{
				ATEMA_BENCHMARK("CommandBuffer (phong lighting)");

				const auto frameIndex = context.getFrameIndex();
				const auto& frameData = m_frameDatas[frameIndex];

				auto gbufferSMSet = m_gbufferShadowMapLayout->createSet();
				auto phongSet = m_phongLayout->createSet();

				// Write descriptor sets
				{
					for (uint32_t i = 0; i < gbufferTextures.size(); i++)
					{
						auto gbufferImage = context.getTexture(gbufferTextures[i]);
						gbufferSMSet->update(i, gbufferImage, m_ppSampler);
					}

					gbufferSMSet->update(shadowMapBindingIndex, context.getTexture(shadowMapTexture), m_shadowMapSampler);

					phongSet->update(0, frameData.shadowBuffer);
					phongSet->update(1, frameData.phongBuffer);
				}

				auto& commandBuffer = context.getCommandBuffer();

				commandBuffer.bindPipeline(m_phongPipeline);

				commandBuffer.setViewport(m_viewport);

				commandBuffer.setScissor(Vector2i(), m_windowSize);

				commandBuffer.bindVertexBuffer(m_ppQuad, 0);

				commandBuffer.bindDescriptorSet(0, gbufferSMSet);
				commandBuffer.bindDescriptorSet(1, phongSet);

				commandBuffer.draw(static_cast<uint32_t>(quadVertices.size()));

				context.destroyAfterUse(std::move(gbufferSMSet));
				context.destroyAfterUse(std::move(phongSet));
			});
	}

	// Screen
	{
		auto& pass = frameGraphBuilder.createPass("screen");

		pass.addSampledTexture(phongOutputTexture, ShaderStage::Fragment);
		pass.addSampledTexture(shadowMapTexture, ShaderStage::Fragment);

		pass.enableRenderFrameOutput(true);

		pass.setExecutionCallback([this, phongOutputTexture, shadowMapTexture](FrameGraphContext& context)
			{
				ATEMA_BENCHMARK("CommandBuffer (screen)");

				auto descriptorSet1 = m_screenLayout->createSet();
				auto descriptorSet2 = m_screenLayout->createSet();

				descriptorSet1->update(0, context.getTexture(phongOutputTexture), m_ppSampler);
				descriptorSet2->update(0, context.getTexture(shadowMapTexture), m_ppSampler);

				auto& commandBuffer = context.getCommandBuffer();

				commandBuffer.bindPipeline(m_screenPipeline);

				commandBuffer.setViewport(m_viewport);

				commandBuffer.setScissor(Vector2i(), m_windowSize);

				commandBuffer.bindDescriptorSet(0, descriptorSet1);

				commandBuffer.bindVertexBuffer(m_ppQuad, 0);

				commandBuffer.draw(static_cast<uint32_t>(quadVertices.size()));

				//commandBuffer.bindDescriptorSet(0, descriptorSet2);

				//commandBuffer.bindVertexBuffer(m_ppDebugQuad, 0);

				//commandBuffer.draw(static_cast<uint32_t>(quadVertices.size()));

				context.destroyAfterUse(std::move(descriptorSet1));
				context.destroyAfterUse(std::move(descriptorSet2));
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

	Vector3f cameraPos;
	Vector3f cameraTarget;

	// Update global buffers
	{
		auto selection = getEntityManager().getUnion<Transform, CameraComponent>();

		for (auto& entity : selection)
		{
			auto& camera = selection.get<CameraComponent>(entity);

			if (camera.display)
			{
				auto& transform = selection.get<Transform>(entity);

				cameraPos = transform.getTranslation();
				const Vector3f cameraUp(0.0f, 0.0f, 1.0f);
				
				cameraTarget = camera.target;

				// If the camera doesn't use target, calculate target from transform rotation
				if (!camera.useTarget)
				{
					cameraTarget = cameraPos + Matrix4f::createRotation(transform.getRotation()).transformVector({ 1.0f, 0.0f, 0.0f });
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
	auto entities = getEntityManager().getUnion<Transform, GraphicsComponent>();

	{
		auto data = static_cast<uint8_t*>(frameData.objectsUniformBuffer->map());

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

	// Update scene AABB
	{
		AABBf sceneAABB;

		for (const auto& entity : entities)
		{
			auto& graphics = entities.get<GraphicsComponent>(entity);
			auto& transform = entities.get<Transform>(entity);

			auto viewAABB = transform.getMatrix() * graphics.aabb;

			sceneAABB.extend(viewAABB);
		}

		sceneAABB = AABBf();

		float boxSize = 50.0f;

		auto center = cameraPos;
		center.x *= -1.0f;
		center.z = 0.0f;

		sceneAABB.extend(center + Vector3f(boxSize, boxSize, 5.0f));
		sceneAABB.extend(center + Vector3f( -boxSize, -boxSize, -5.0f));

		const auto view = Matrix4f::createLookAt({ 0.0f, 0.0f, 0.0f }, lightDirection, Vector3f(0, 0, 1));

		const auto viewAABB = view * sceneAABB;

		auto zNear = std::min(viewAABB.min.z, viewAABB.max.z);
		auto zFar = std::max(viewAABB.min.z, viewAABB.max.z);

		auto proj = Matrix4f::createOrtho(viewAABB.min.x, viewAABB.max.x, viewAABB.min.y, viewAABB.max.y, zNear, zFar);
		proj[1][1] *= -1;

		auto& data = *static_cast<UniformShadowMappingData*>(frameData.shadowBuffer->map());

		data.depthMVP = proj * view;

		frameData.shadowBuffer->unmap();
	}

	// Update phong buffer
	{
		auto& data = *static_cast<UniformPhongLightingData*>(frameData.phongBuffer->map());

		data.cameraPosition = { cameraPos.x, cameraPos.y, cameraPos.z, 1.0f };
		data.lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		data.lightPosition = { lightPosition.x, lightPosition.y, lightPosition.z, 1.0f };
		data.ambientStrength = 0.15f;

		frameData.shadowBuffer->unmap();
	}
}
