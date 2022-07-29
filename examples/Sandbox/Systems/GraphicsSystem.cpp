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

#include <Atema/Core/Utils.hpp>
#include <Atema/Graphics/FrameGraphBuilder.hpp>
#include <Atema/Graphics/FrameGraphContext.hpp>
#include <Atema/Renderer/Buffer.hpp>
#include <Atema/Renderer/BufferLayout.hpp>
#include <Atema/Renderer/GraphicsPipeline.hpp>
#include <Atema/Renderer/Shader.hpp>
#include <Atema/Shader/Ast/AstPreprocessor.hpp>
#include <Atema/Window/WindowResizeEvent.hpp>
#include <Atema/UI/UiContext.hpp>

#include "../Resources.hpp"
#include "../Scene.hpp"
#include "../Settings.hpp"
#include "../Components/GraphicsComponent.hpp"
#include "../Components/CameraComponent.hpp"

#include <fstream>

using namespace at;

namespace
{
	const Vector3f lightDirection = Vector3f(1.0f, 1.0f, -1.0f).normalize();

	//-----

	const auto gbufferShaderPath = shaderPath / "GBuffer.atsl";
	const auto shadowMapShaderPath = shaderPath / "ShadowMap.atsl";
	const auto postProcessShaderPath = shaderPath / "PostProcess.atsl";
	const auto phongLightingDirectionalShaderPath = shaderPath / "PhongLightingDirectional.atsl";
	const auto screenShaderPath = shaderPath / "Screen.atsl";

	struct ShaderData
	{
		std::filesystem::path path;
		Flags<AstShaderStage> shaderStages;
	};

	const std::vector<ShaderData> shaderDatas =
	{
		{ gbufferShaderPath, AstShaderStage::Vertex | AstShaderStage::Fragment },
		{ shadowMapShaderPath, AstShaderStage::Vertex | AstShaderStage::Fragment },
		{ postProcessShaderPath, AstShaderStage::Vertex },
		{ phongLightingDirectionalShaderPath, AstShaderStage::Vertex | AstShaderStage::Fragment },
		{ screenShaderPath, AstShaderStage::Fragment }
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
}

GraphicsSystem::GraphicsSystem(const Ptr<RenderWindow>& renderWindow) :
	System(),
	m_renderWindow(renderWindow),
	m_totalTime(0.0f),
	m_updateFrameGraph(true),
	m_shadowMapSize(0),
	m_debugViewMode(Settings::DebugViewMode::Disabled)
{
	ATEMA_ASSERT(renderWindow, "Invalid RenderWindow");

	translateShaders();

	auto& renderer = Renderer::instance();

	const auto maxFramesInFlight = renderWindow->getMaxFramesInFlight();

	//m_depthFormat = renderWindow->getDepthFormat();
	m_depthFormat = ImageFormat::D32F_S8U;

	//----- SHADERS -----//
	std::unordered_map<std::string, Ptr<Shader>> vertexShaders;
	std::unordered_map<std::string, Ptr<Shader>> fragmentShaders;

	for (const auto& shaderData : shaderDatas)
	{
		std::stringstream code;

		{
			std::ifstream file(shaderData.path);

			if (!file.is_open())
			{
				ATEMA_ERROR("Failed to open file '" + shaderData.path.string() + "'");
			}

			code << file.rdbuf();
		}

		AtslParser atslParser;

		const auto atslTokens = atslParser.createTokens(code.str());

		AtslToAstConverter converter;

		auto ast = converter.createAst(atslTokens);

		AstStageExtractor stageExtractor;

		ast->accept(stageExtractor);

		Shader::Settings shaderSettings;
		shaderSettings.shaderLanguage = ShaderLanguage::Ast;
		shaderSettings.shaderDataSize = 1; // First AST element

		if (shaderData.shaderStages & AstShaderStage::Vertex)
		{
			auto stageAst = stageExtractor.getAst(AstShaderStage::Vertex);

			shaderSettings.shaderData = stageAst.get();

			vertexShaders[shaderData.path.string()] = Shader::create(shaderSettings);
		}

		if (shaderData.shaderStages & AstShaderStage::Fragment)
		{
			auto stageAst = stageExtractor.getAst(AstShaderStage::Fragment);

			shaderSettings.shaderData = stageAst.get();

			fragmentShaders[shaderData.path.string()] = Shader::create(shaderSettings);
		}
	}

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
		pipelineSettings.vertexShader = vertexShaders[postProcessShaderPath.string()];
		pipelineSettings.fragmentShader = fragmentShaders[screenShaderPath.string()];
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
	}

	//----- OBJECT RESOURCES -----//

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

	// Uniform buffers & descriptor sets
	/*struct UniformFrameElement
	{
		mat4f proj;
		mat4f view;
		vec3f cameraPosition;
	}*/
	BufferLayout frameLayout(StructLayout::Default);
	frameLayout.addMatrix(BufferElementType::Float, 4, 4);
	frameLayout.addMatrix(BufferElementType::Float, 4, 4);
	frameLayout.add(BufferElementType::Float3);

	/*struct UniformObjectElement
	{
		Matrix4f model;
	}*/
	BufferLayout objectLayout(StructLayout::Default);
	objectLayout.addMatrix(BufferElementType::Float, 4, 4);

	m_elementByteSize = static_cast<uint32_t>(objectLayout.getSize());
	const auto minOffset = static_cast<uint32_t>(Renderer::instance().getLimits().minUniformBufferOffsetAlignment);

	m_dynamicObjectBufferOffset = Math::getNextMultiple(m_elementByteSize, minOffset);

	m_frameDatas.resize(maxFramesInFlight);
	for (auto& frameData : m_frameDatas)
	{
		// Frame uniform buffers
		frameData.frameUniformBuffer = Buffer::create({ BufferUsage::Uniform, frameLayout.getSize(), true });

		// Frame object uniform buffers
		frameData.objectCount = 1;
		frameData.objectsUniformBuffer = Buffer::create({ BufferUsage::Uniform, static_cast<size_t>(1 * m_dynamicObjectBufferOffset), true });

		// Add descriptor set
		frameData.descriptorSet = m_frameLayout->createSet();
		frameData.descriptorSet->update(0, frameData.frameUniformBuffer);
		frameData.descriptorSet->update(1, frameData.objectsUniformBuffer, m_elementByteSize);
	}

	//----- SHADOW MAPPING -----//

	// Fill pass data buffer
	{
		/*struct UniformShadowMappingData
		{
			Matrix4f depthMVP;
		}*/
		BufferLayout shadowLayout(StructLayout::Default);
		shadowLayout.addMatrix(BufferElementType::Float, 4, 4);

		Buffer::Settings bufferSettings;
		bufferSettings.usage = BufferUsage::Uniform;
		bufferSettings.byteSize = shadowLayout.getSize();
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
		graphicsPipelineSettings.vertexShader = vertexShaders[shadowMapShaderPath.string()];
		graphicsPipelineSettings.fragmentShader = fragmentShaders[shadowMapShaderPath.string()];
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
		/*struct UniformPhongLightingData
		{
			vec3f cameraPosition;
			vec3f lightDirection;
			vec3f lightColor;
			float ambientStrength;
		}*/

		BufferLayout phongLayout(StructLayout::Default);
		phongLayout.add(BufferElementType::Float3);
		phongLayout.add(BufferElementType::Float3);
		phongLayout.add(BufferElementType::Float3);
		phongLayout.add(BufferElementType::Float);

		Buffer::Settings bufferSettings;
		bufferSettings.usage = BufferUsage::Uniform;
		bufferSettings.byteSize = phongLayout.getSize();
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
		graphicsPipelineSettings.vertexShader = vertexShaders[phongLightingDirectionalShaderPath.string()];
		graphicsPipelineSettings.fragmentShader = fragmentShaders[phongLightingDirectionalShaderPath.string()];
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
	m_materials.clear();
}

void GraphicsSystem::update(TimeStep timeStep)
{
	// Ensure the settings did not change
	checkSettings();

	if (m_updateFrameGraph)
		createFrameGraph();

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

			AstPreprocessor astPreprocessor;
			astPreprocessor.setOption("MaterialColorBinding", 0);
			astPreprocessor.setOption("MaterialHeightBinding", 1);

			ast->accept(astPreprocessor);
			auto preprocessedAst = astPreprocessor.process(*ast);

			std::vector<AstShaderStage> shaderStages = { AstShaderStage::Vertex, AstShaderStage::Fragment };
			std::vector<std::string> glslSuffixes = { ".vert", ".frag" };
			std::vector<std::string> spirvSuffixes = { "VS.spv", "FS.spv" };

			for (size_t i = 0; i < shaderStages.size(); i++)
			{
				const auto& shaderStage = shaderStages[i];
				const auto& glslSuffix = glslSuffixes[i];
				const auto& spirvSuffix = spirvSuffixes[i];

				AstStageExtractor stageExtractor;

				preprocessedAst->accept(stageExtractor);

				try
				{
					auto stageAst = stageExtractor.getAst(shaderStage);

					{
						std::ofstream file(shaderPath / (path.stem().string() + glslSuffix));

						GlslShaderWriter::Settings settings;
						settings.stage = shaderStage;

						GlslShaderWriter glslWriter(file, settings);

						stageAst->accept(glslWriter);
					}

					{
						std::ofstream file(shaderPath / (path.stem().string() + spirvSuffix), std::ios::binary);

						SpirvShaderWriter::Settings settings;
						settings.stage = shaderStage;

						SpirvShaderWriter spvWriter(settings);

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

void GraphicsSystem::checkSettings()
{
	const auto& settings = Settings::instance();

	if (m_shadowMapSize != settings.shadowMapSize)
	{
		m_shadowMapSize = settings.shadowMapSize;

		m_shadowViewport.size = { m_shadowMapSize, m_shadowMapSize };

		m_updateFrameGraph = true;
	}

	if (m_debugViewMode != settings.debugViewMode)
	{
		m_debugViewMode = settings.debugViewMode;

		m_updateFrameGraph = true;
	}

	if (m_debugViews != settings.debugViews)
	{
		m_debugViews = settings.debugViews;

		m_updateFrameGraph = true;
	}
}

void GraphicsSystem::createFrameGraph()
{
	Renderer::instance().waitForIdle();

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
	auto debugOutputTexture = frameGraphBuilder.createTexture(textureSettings);

	textureSettings.format = ImageFormat::D32F;
	textureSettings.width = m_shadowMapSize;
	textureSettings.height = m_shadowMapSize;
	auto shadowMapTexture = frameGraphBuilder.createTexture(textureSettings);

	auto getDebugTexture = [&](Settings::DebugView debugView) -> FrameGraphTextureHandle
	{
		switch (debugView)
		{
			case Settings::DebugView::GBufferPosition: return gbufferTextures[0];
			case Settings::DebugView::GBufferNormal: return gbufferTextures[1];
			case Settings::DebugView::GBufferColor: return gbufferTextures[2];
			case Settings::DebugView::GBufferAO: return gbufferTextures[3];
			case Settings::DebugView::GBufferEmissive: return gbufferTextures[4];
			case Settings::DebugView::GBufferMetalness: return gbufferTextures[5];
			case Settings::DebugView::GBufferRoughness: return gbufferTextures[6];
			case Settings::DebugView::ShadowMap: return shadowMapTexture;
			default: break;
		}

		return phongOutputTexture;
	};

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

							commandBuffer->setViewport(m_viewport);

							commandBuffer->setScissor(Vector2i(), m_windowSize);

							MaterialData* currentMaterialID = nullptr;

							uint32_t i = static_cast<uint32_t>(firstIndex);
							for (auto it = entities.begin() + firstIndex; it != entities.begin() + lastIndex; it++)
							{
								auto& graphics = entities.get<GraphicsComponent>(*it);

								auto materialID = graphics.material.get();

								if (materialID != currentMaterialID)
								{
									auto& material = m_materials[materialID];

									commandBuffer->bindPipeline(material.pipeline);
									commandBuffer->bindDescriptorSet(1, material.descriptorSet);

									currentMaterialID = materialID;
								}

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

							commandBuffer->setScissor(Vector2i(), { m_shadowMapSize, m_shadowMapSize });

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

	// Debug (corner)
	if (m_debugViewMode == Settings::DebugViewMode::Corner)
	{
		auto& pass = frameGraphBuilder.createPass("debug corner");

		pass.addOutputTexture(phongOutputTexture, 0);

		const auto debugTexture = getDebugTexture(m_debugViews[0]);

		pass.addSampledTexture(debugTexture, ShaderStage::Fragment);

		pass.setExecutionCallback([this, debugTexture](FrameGraphContext& context)
			{
				ATEMA_BENCHMARK("CommandBuffer (debug corner)");

				auto descriptorSet = m_screenLayout->createSet();

				descriptorSet->update(0, context.getTexture(debugTexture), m_ppSampler);

				auto& commandBuffer = context.getCommandBuffer();

				commandBuffer.bindPipeline(m_screenPipeline);

				commandBuffer.setScissor(Vector2i(), m_windowSize);

				Viewport viewport;
				viewport.position.x = 10.0f;
				viewport.position.y = static_cast<float>(2 * m_windowSize.y / 3) - 10.0f;
				viewport.size = { m_windowSize.x / 3, m_windowSize.y / 3 };

				commandBuffer.setViewport(viewport);

				commandBuffer.bindDescriptorSet(0, descriptorSet);

				commandBuffer.bindVertexBuffer(m_ppQuad, 0);

				commandBuffer.draw(static_cast<uint32_t>(quadVertices.size()));

				context.destroyAfterUse(std::move(descriptorSet));
			});
	}

	// Debug (full)
	if (m_debugViewMode == Settings::DebugViewMode::Full)
	{
		auto& pass = frameGraphBuilder.createPass("debug full");

		pass.addOutputTexture(phongOutputTexture, 0);

		const std::vector<FrameGraphTextureHandle> debugTextures =
		{
			getDebugTexture(m_debugViews[0]),
			getDebugTexture(m_debugViews[1]),
			getDebugTexture(m_debugViews[2]),
			getDebugTexture(m_debugViews[3]),
		};

		for (const auto& debugTexture : debugTextures)
			pass.addSampledTexture(debugTexture, ShaderStage::Fragment);

		pass.setExecutionCallback([this, debugTextures](FrameGraphContext& context)
			{
				ATEMA_BENCHMARK("CommandBuffer (debug full)");

				auto& commandBuffer = context.getCommandBuffer();

				commandBuffer.bindPipeline(m_screenPipeline);

				commandBuffer.setScissor(Vector2i(), m_windowSize);

				commandBuffer.bindVertexBuffer(m_ppQuad, 0);

				for (size_t x = 0; x < 2; x++)
				{
					for (size_t y = 0; y < 2; y++)
					{
						const auto i = x + 2 * y;

						Viewport viewport;
						viewport.position.x = static_cast<float>(x * m_windowSize.x / 2);
						viewport.position.y = static_cast<float>(y * m_windowSize.y / 2);
						viewport.size = { m_windowSize.x / 2, m_windowSize.y / 2 };

						commandBuffer.setViewport(viewport);

						auto descriptorSet = m_screenLayout->createSet();

						descriptorSet->update(0, context.getTexture(debugTextures[i]), m_ppSampler);

						commandBuffer.bindDescriptorSet(0, descriptorSet);

						commandBuffer.draw(static_cast<uint32_t>(quadVertices.size()));

						context.destroyAfterUse(std::move(descriptorSet));
					}
				}
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

				// UI
				{
					ATEMA_BENCHMARK("ImGui");

					UiContext::instance().renderDrawData(ImGui::GetDrawData(), commandBuffer);
				}

				context.destroyAfterUse(std::move(descriptorSet1));
				context.destroyAfterUse(std::move(descriptorSet2));
			});
	}

	//-----
	// Build frame graph
	m_frameGraph = frameGraphBuilder.build();
	m_updateFrameGraph = false;
}

void GraphicsSystem::onResize(const Vector2u& size)
{
	m_viewport.size.x = static_cast<float>(size.x);
	m_viewport.size.y = static_cast<float>(size.y);
	m_windowSize = size;

	m_updateFrameGraph = true;
}

void GraphicsSystem::updateFrame()
{
	Benchmark benchmark("RenderWindow::acquireFrame");

	auto& renderFrame = m_renderWindow.lock()->acquireFrame();

	benchmark.stop();

	const auto frameIndex = renderFrame.getFrameIndex();

	auto& frameData = m_frameDatas[frameIndex];

	// Update material descriptor sets
	updateMaterialResources();

	// Update scene data
	updateUniformBuffers(frameData);

	benchmark.start("FrameGraph::execute");

	m_frameGraph->execute(renderFrame);
}

void GraphicsSystem::updateMaterialResources()
{
	auto entities = getEntityManager().getUnion<GraphicsComponent>();

	for (const auto& entity : entities)
	{
		auto& graphics = entities.get<GraphicsComponent>(entity);

		auto& materialData = *graphics.material;

		auto materialID = graphics.material.get();

		if (m_materials.count(materialID) > 0)
			continue;

		Material material;

		GraphicsPipeline::Settings pipelineSettings;
		pipelineSettings.vertexShader = materialData.vertexShader;
		pipelineSettings.fragmentShader = materialData.fragmentShader;
		pipelineSettings.descriptorSetLayouts = { m_frameLayout, materialData.descriptorSetLayout };
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
		pipelineSettings.rasterization.cullMode = CullMode::None;

		material.pipeline = GraphicsPipeline::create(pipelineSettings);
		material.descriptorSet = materialData.descriptorSet;

		m_materials[materialID] = material;
	}
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

				/*struct UniformFrameElement
				{
					mat4f proj;
					mat4f view;
					vec3f cameraPosition;
				}*/
				BufferLayout frameLayout(StructLayout::Default);
				const auto projOffset = frameLayout.addMatrix(BufferElementType::Float, 4, 4);
				const auto viewOffset = frameLayout.addMatrix(BufferElementType::Float, 4, 4);
				const auto cameraPositionOffset = frameLayout.add(BufferElementType::Float3);

				void* data = frameData.frameUniformBuffer->map();

				mapMemory<Matrix4f>(data, viewOffset) = Matrix4f::createLookAt(cameraPos, cameraTarget, cameraUp);
				mapMemory<Matrix4f>(data, projOffset) = Matrix4f::createPerspective(Math::toRadians(camera.fov), camera.aspectRatio, camera.nearPlane, camera.farPlane);
				mapMemory<Matrix4f>(data, projOffset)[1][1] *= -1;
				mapMemory<Vector3f>(data, cameraPositionOffset) = { cameraPos.x, cameraPos.y, cameraPos.z, 1.0f };

				frameData.frameUniformBuffer->unmap();

				break;
			}
		}
	}

	// Update objects buffers
	auto entities = getEntityManager().getUnion<Transform, GraphicsComponent>();

	if (entities.size() != frameData.objectCount)
	{
		// Frame object uniform buffers
		frameData.objectCount = entities.size();
		frameData.objectsUniformBuffer = Buffer::create({ BufferUsage::Uniform, static_cast<size_t>(frameData.objectCount * m_dynamicObjectBufferOffset), true });

		// Update descriptor set
		frameData.descriptorSet->update(1, frameData.objectsUniformBuffer, m_elementByteSize);
	}

	{
		/*struct UniformObjectElement
		{
			Matrix4f model;
		}*/
		BufferLayout objectLayout(StructLayout::Default);
		const auto modelOffset = objectLayout.addMatrix(BufferElementType::Float, 4, 4);

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

			auto task = taskManager.createTask([this, data, firstIndex, lastIndex, &entities, modelOffset]()
				{
					auto it = entities.begin() + firstIndex;

					for (size_t j = firstIndex; j < lastIndex; j++)
					{
						auto& transform = entities.get<Transform>(*it);

						mapMemory<Matrix4f>(data, j * m_dynamicObjectBufferOffset + modelOffset) = transform.getMatrix();

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
		AABBf sceneAABB = Scene::instance().getAABB();

		auto aabbSize = sceneAABB.getSize();
		aabbSize.z = 0.0f;

		auto cameraDir = cameraTarget - cameraPos;
		cameraDir.normalize();

		float cameraDirZ = std::abs(cameraDir.z);

		cameraDir.z = 0.0f;
		cameraDir.normalize();

		float minDirZ = 0.2f;
		float maxDirZ = 0.6f;

		if (cameraDirZ < minDirZ)
			cameraDirZ = minDirZ;

		if (cameraDirZ > maxDirZ)
			cameraDirZ = maxDirZ;

		auto camDirZPercent = (cameraDirZ - minDirZ) / (maxDirZ - minDirZ);

		float altitude = cameraPos.z;

		float minAlt = 2.0f;
		float maxAlt = 30.0f;

		if (altitude < minAlt)
			altitude = minAlt;

		if (altitude > maxAlt)
			altitude = maxAlt;

		// 0% : minAlt / 100% : maxAlt
		auto altitudePercent = (altitude - minAlt) / (maxAlt - minAlt);

		float minBoxSize = Settings::instance().shadowBoxMinSize / 2.0f;
		float maxBoxSize = Settings::instance().shadowBoxMaxSize / 2.0f;

		float boxSize = maxBoxSize * altitudePercent + minBoxSize * (1.0f - altitudePercent);
		boxSize = std::max(boxSize, minBoxSize * camDirZPercent + maxBoxSize * (1.0f - camDirZPercent));

		auto center = cameraPos;
		center.z = 0.0f;

		center += cameraDir * boxSize * 0.9f;

		center.x = std::clamp(center.x, sceneAABB.min.x, sceneAABB.max.x);
		center.y = std::clamp(center.y, sceneAABB.min.y, sceneAABB.max.y);

		AABBf shadowMapAABB;
		shadowMapAABB.extend(center + Vector3f(boxSize, boxSize, 5.0f));
		shadowMapAABB.extend(center + Vector3f( -boxSize, -boxSize, -5.0f));

		auto zFar = 200.0f;
		
		const auto view = Matrix4f::createLookAt(center - lightDirection * zFar, center, Vector3f(0, 0, 1));

		auto sizeS = shadowMapAABB.getSize();

		const auto viewAABB = view * shadowMapAABB;

		auto sizeB = viewAABB.getSize();

		auto zNear = -viewAABB.max.z;
		zFar = -viewAABB.min.z;

		auto proj = Matrix4f::createOrtho(viewAABB.min.x, viewAABB.max.x, viewAABB.min.y, viewAABB.max.y, zNear, zFar);
		proj[1][1] *= -1;

		/*struct UniformShadowMappingData
		{
			Matrix4f depthMVP;
		}*/
		BufferLayout shadowLayout(StructLayout::Default);
		const auto mvpOffset = shadowLayout.addMatrix(BufferElementType::Float, 4, 4);

		auto data = frameData.shadowBuffer->map();

		mapMemory<Matrix4f>(data, mvpOffset) = proj * view;

		frameData.shadowBuffer->unmap();
	}

	// Update phong buffer
	{
		/*struct UniformPhongLightingData
		{
			vec3f cameraPosition;
			vec3f lightDirection;
			vec3f lightColor;
			float ambientStrength;
		}*/

		BufferLayout phongLayout(StructLayout::Default);
		const auto cameraPositionOffset = phongLayout.add(BufferElementType::Float3);
		const auto lightDirectionOffset = phongLayout.add(BufferElementType::Float3);
		const auto lightColorOffset = phongLayout.add(BufferElementType::Float3);
		const auto ambientStrengthOffset = phongLayout.add(BufferElementType::Float);

		auto data = frameData.phongBuffer->map();

		mapMemory<Vector3f>(data, cameraPositionOffset) = { cameraPos.x, cameraPos.y, cameraPos.z };
		mapMemory<Vector3f>(data, lightDirectionOffset) = { lightDirection.x, lightDirection.y, lightDirection.z };
		mapMemory<Vector4f>(data, lightColorOffset) = { 1.0f, 1.0f, 1.0f };
		mapMemory<float>(data, ambientStrengthOffset) = 0.3f;

		frameData.shadowBuffer->unmap();
	}
}
