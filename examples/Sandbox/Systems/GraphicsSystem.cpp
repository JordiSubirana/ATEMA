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
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/IndexBuffer.hpp>
#include <Atema/Graphics/Mesh.hpp>
#include <Atema/Graphics/VertexBuffer.hpp>
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
#include "../Stats.hpp"
#include "../Components/GraphicsComponent.hpp"
#include "../Components/CameraComponent.hpp"

#include <fstream>

using namespace at;

namespace
{
	const Vector3f lightDirection = Vector3f(1.0f, 1.0f, -1.0f).normalize();

	//-----

	const auto shadowMapShaderPath = shaderPath / "ShadowMap.atsl";
	const auto postProcessShaderPath = shaderPath / "PostProcess.atsl";
	const auto phongLightingDirectionalShaderPath = shaderPath / "PhongLightingDirectional.atsl";
	const auto screenShaderPath = shaderPath / "Screen.atsl";

	struct ShaderData
	{
		std::filesystem::path path;
		Flags<AstShaderStage> shaderStages;
		std::unordered_map<std::string, ConstantValue> options;
	};

	const std::vector<ShaderData> shaderDatas =
	{
		{ shadowMapShaderPath, AstShaderStage::Vertex | AstShaderStage::Fragment, {} },
		{ postProcessShaderPath, AstShaderStage::Vertex, {} },
		{ phongLightingDirectionalShaderPath, AstShaderStage::Vertex | AstShaderStage::Fragment, {{"ShadowMapCascadeCount", SHADOW_CASCADE_COUNT}} },
		{ screenShaderPath, AstShaderStage::Fragment, {} }
	};

	constexpr size_t targetThreadCount = 8;

	const size_t threadCount = std::min(targetThreadCount, TaskManager::instance().getSize());

	const std::vector<ImageFormat> gBuffer =
	{
		// Position (RGB) + Metalness (A)
		ImageFormat::RGBA32_SFLOAT,
		// Normal (RGB) + Roughness (A)
		ImageFormat::RGBA32_SFLOAT,
		// Albedo (RGB) + AO (A)
		ImageFormat::RGBA8_UNORM,
		// Emissive
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
		{ 0, 0, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 1, VertexInputFormat::RG32_SFLOAT }
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

		auto stagingBuffer = Buffer::create({ BufferUsage::TransferSrc | BufferUsage::Map, bufferSize });

		auto bufferData = stagingBuffer->map();

		memcpy(bufferData, static_cast<void*>(vertices.data()), static_cast<size_t>(bufferSize));

		stagingBuffer->unmap();

		// Create vertex buffer
		auto vertexBuffer = Buffer::create({ BufferUsage::Vertex | BufferUsage::TransferDst, bufferSize });

		// Copy staging buffer to vertex buffer
		auto commandBuffer = commandPool->createBuffer({ true });

		commandBuffer->begin();

		commandBuffer->copyBuffer(*stagingBuffer, *vertexBuffer, bufferSize);

		commandBuffer->end();

		Renderer::instance().submitAndWait({ commandBuffer });

		return vertexBuffer;
	}

	struct MeshDrawData
	{
		MeshDrawData() = default;
		MeshDrawData(const MeshDrawData& other) = default;
		MeshDrawData(MeshDrawData&& other) noexcept = default;
		~MeshDrawData() = default;

		VertexBuffer* vertexBuffer = nullptr;
		IndexBuffer* indexBuffer = nullptr;
		SurfaceMaterialInstance* materialInstance = nullptr;
		EntityHandle entityHandle = InvalidEntityHandle;
		uint32_t index = 0;
		uint64_t renderPriority = 0;

		MeshDrawData& operator=(const MeshDrawData& other) = default;
		MeshDrawData& operator=(MeshDrawData&& other) noexcept = default;

		bool operator<(const MeshDrawData& other) const
		{
			return renderPriority < other.renderPriority;
		}

		bool operator>(const MeshDrawData& other) const
		{
			return renderPriority > other.renderPriority;
		}

		uint64_t getRenderPriority() const
		{
			if (!materialInstance || !materialInstance->getMaterial())
				return 0;

			uint64_t renderPriority = 0;
			renderPriority |= static_cast<uint64_t>(materialInstance->getMaterial()->getID() & 0xFFFF) << 16;
			renderPriority |= static_cast<uint64_t>(materialInstance->getID() & 0xFFFF) << 0;

			return renderPriority;
		}
	};
}

GraphicsSystem::GraphicsSystem(const Ptr<RenderWindow>& renderWindow) :
	System(),
	m_renderWindow(renderWindow),
	m_totalTime(0.0f),
	m_updateFrameGraph(true),
	m_currentShadowMapSize(0),
	m_shadowMapSize(0),
	m_enableDebugRenderer(false),
	m_debugViewMode(Settings::DebugViewMode::Disabled)
{
	ATEMA_ASSERT(renderWindow, "Invalid RenderWindow");

	//translateShaders();
	Graphics::instance().initializeShaderLibraries(ShaderLibraryManager::instance());

	auto& renderer = Renderer::instance();

	m_debugRenderer = std::make_shared<DebugRenderer>();

	const auto maxFramesInFlight = renderWindow->getMaxFramesInFlight();

	// Remove unused resources if they are not used during 3 cycles (arbitrary)
	Graphics::instance().setMaxUnusedCounter(static_cast<uint32_t>(maxFramesInFlight) * 3);

	//m_depthFormat = renderWindow->getDepthFormat();
	m_depthFormat = ImageFormat::D32_SFLOAT_S8_UINT;

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

		// If some options need to be overridden, use a preprocessor to do the job
		// If not we just take the AST as it is
		if (!shaderData.options.empty())
		{
			AstPreprocessor preprocessor;

			for (const auto& [optionName, optionValue] : shaderData.options)
				preprocessor.setOption(optionName, optionValue);

			ast->accept(preprocessor);
			auto processedAst = preprocessor.process(*ast);

			if (!processedAst)
				ATEMA_ERROR("An error occurred during shader preprocessing");

			UPtr<SequenceStatement> sequence;

			if (processedAst->getType() == Statement::Type::Sequence)
			{
				sequence.reset(static_cast<SequenceStatement*>(processedAst.release()));
			}
			else
			{
				sequence = std::make_unique<SequenceStatement>();
				sequence->statements.emplace_back(std::move(processedAst));
			}

			ast = std::move(sequence);
		}

		AstReflector reflector;

		ast->accept(reflector);

		Shader::Settings shaderSettings;
		shaderSettings.shaderLanguage = ShaderLanguage::Ast;
		shaderSettings.shaderDataSize = 1; // First AST element

		if (shaderData.shaderStages & AstShaderStage::Vertex)
		{
			auto stageAst = reflector.getAst(AstShaderStage::Vertex);

			shaderSettings.shaderData = stageAst.get();

			vertexShaders[shaderData.path.string()] = Shader::create(shaderSettings);
		}

		if (shaderData.shaderStages & AstShaderStage::Fragment)
		{
			auto stageAst = reflector.getAst(AstShaderStage::Fragment);

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
		pipelineSettings.state.vertexInput.inputs = postProcessVertexInput;
		pipelineSettings.state.depth.test = false;
		pipelineSettings.state.depth.write = false;

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
			{ DescriptorType::UniformBuffer, 0, 1, ShaderStage::Vertex }
		};
		descriptorSetLayoutSettings.pageSize = maxFramesInFlight;

		m_frameLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);
	}

	{
		DescriptorSetLayout::Settings descriptorSetLayoutSettings;
		descriptorSetLayoutSettings.bindings =
		{
			{ DescriptorType::UniformBufferDynamic, 0, 1, ShaderStage::Vertex }
		};
		descriptorSetLayoutSettings.pageSize = maxFramesInFlight;

		m_objectLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);
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

	{
		/*struct UniformObjectElement
		{
			Matrix4f model;
		}*/
		BufferLayout objectLayout(StructLayout::Default);
		objectLayout.addMatrix(BufferElementType::Float, 4, 4);

		m_elementByteSize = static_cast<uint32_t>(objectLayout.getSize());
		const auto minOffset = static_cast<uint32_t>(Renderer::instance().getLimits().minUniformBufferOffsetAlignment);

		m_dynamicObjectBufferOffset = Math::getNextMultiple(m_elementByteSize, minOffset);
	}

	m_frameDatas.resize(maxFramesInFlight);
	for (auto& frameData : m_frameDatas)
	{
		// Frame uniform buffers
		frameData.frameUniformBuffer = Buffer::create({ BufferUsage::Uniform | BufferUsage::Map, frameLayout.getSize() });

		// Frame object uniform buffers
		frameData.objectCount = 1;
		frameData.objectsUniformBuffer = Buffer::create({ BufferUsage::Uniform | BufferUsage::Map, static_cast<size_t>(1 * m_dynamicObjectBufferOffset) });

		// Add descriptor set
		frameData.frameDescriptorSet = m_frameLayout->createSet();
		frameData.frameDescriptorSet->update(0, *frameData.frameUniformBuffer);
		frameData.objectDescriptorSet = m_objectLayout->createSet();
		frameData.objectDescriptorSet->update(0, *frameData.objectsUniformBuffer, 0, m_elementByteSize);
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

		m_shadowElementByteSize = static_cast<uint32_t>(shadowLayout.getSize());
		const auto minOffset = static_cast<uint32_t>(Renderer::instance().getLimits().minUniformBufferOffsetAlignment);

		m_dynamicShadowBufferOffset = Math::getNextMultiple(m_shadowElementByteSize, minOffset);

		Buffer::Settings bufferSettings;
		bufferSettings.usages = BufferUsage::Uniform | BufferUsage::Map;
		bufferSettings.byteSize = SHADOW_CASCADE_COUNT * m_dynamicShadowBufferOffset;

		for (auto& frameData : m_frameDatas)
			frameData.shadowBuffer = Buffer::create(bufferSettings);
	}

	{
		DescriptorSetLayout::Settings descriptorSetLayoutSettings;
		descriptorSetLayoutSettings.pageSize = 1;
		descriptorSetLayoutSettings.bindings =
		{
			{ DescriptorType::UniformBufferDynamic, 0, 1, ShaderStage::Vertex }
		};

		m_shadowLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);

		for (auto& frameData : m_frameDatas)
		{
			frameData.shadowSet = m_shadowLayout->createSet();

			frameData.shadowSet->update(0, *frameData.shadowBuffer, 0, m_shadowElementByteSize);
		}
	}

	{
		GraphicsPipeline::Settings graphicsPipelineSettings;
		graphicsPipelineSettings.vertexShader = vertexShaders[shadowMapShaderPath.string()];
		graphicsPipelineSettings.fragmentShader = fragmentShaders[shadowMapShaderPath.string()];
		graphicsPipelineSettings.descriptorSetLayouts = { m_frameLayout, m_objectLayout, m_shadowLayout };
		graphicsPipelineSettings.state.vertexInput.inputs =
		{
			{ 0, 0, VertexInputFormat::RGB32_SFLOAT },
			{ 0, 1, VertexInputFormat::RGB32_SFLOAT },
			{ 0, 2, VertexInputFormat::RGB32_SFLOAT },
			{ 0, 3, VertexInputFormat::RGB32_SFLOAT },
			{ 0, 4, VertexInputFormat::RG32_SFLOAT }
		};
		graphicsPipelineSettings.state.rasterization.depthClamp = true;

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
		bufferSettings.usages = BufferUsage::Uniform | BufferUsage::Map;
		bufferSettings.byteSize = phongLayout.getSize();

		for (auto& frameData : m_frameDatas)
			frameData.phongBuffer = Buffer::create(bufferSettings);
	}

	{
		/*struct ShadowData
		{
			mat4f view;
			mat4f cascadeViewProj[ShadowMapCascadeCount];
			float cascadeDepth[ShadowMapCascadeCount];
			float cascadeDepthBias[ShadowMapCascadeCount];
		}*/
		BufferLayout shadowLayout(StructLayout::Default);
		shadowLayout.addMatrix(BufferElementType::Float, 4, 4);
		shadowLayout.addMatrixArray(BufferElementType::Float, 4, 4, true, SHADOW_CASCADE_COUNT);
		shadowLayout.addArray(BufferElementType::Float, SHADOW_CASCADE_COUNT);
		shadowLayout.addArray(BufferElementType::Float, SHADOW_CASCADE_COUNT);

		Buffer::Settings bufferSettings;
		bufferSettings.usages = BufferUsage::Uniform | BufferUsage::Map;
		bufferSettings.byteSize = shadowLayout.getSize();

		for (auto& frameData : m_frameDatas)
			frameData.shadowCascadesBuffer = Buffer::create(bufferSettings);
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
		graphicsPipelineSettings.state.vertexInput.inputs = postProcessVertexInput;
		graphicsPipelineSettings.state.depth.test = false;
		graphicsPipelineSettings.state.depth.write = false;
		//*
		graphicsPipelineSettings.state.stencil = true;
		graphicsPipelineSettings.state.stencilFront.compareOperation = CompareOperation::Equal;
		graphicsPipelineSettings.state.stencilFront.compareMask = 1;
		graphicsPipelineSettings.state.stencilFront.reference = 1;
		//*/

		m_phongPipeline = GraphicsPipeline::create(graphicsPipelineSettings);
	}

	//----- MISC -----//

	// Create size dependent resources
	onResize(renderWindow->getSize());
}

GraphicsSystem::~GraphicsSystem()
{
	Renderer::instance().waitForIdle();

	Graphics::instance().clear();

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
	m_objectLayout.reset();
	m_materials.clear();
}

void GraphicsSystem::update(TimeStep timeStep)
{
	// Ensure the settings did not change
	checkSettings();

	if (m_updateFrameGraph)
		createFrameGraph();

	// Update frustum rotation
	m_frustumRotation += (Math::Pi<float> / 4.0f) * timeStep.getSeconds();

	// Start frame
	updateFrame();

	// Update total time
	m_totalTime += timeStep.getSeconds();

	// Clear resources that have been unused for too long
	Graphics::instance().clearUnused();
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

				AstReflector reflector;

				preprocessedAst->accept(reflector);

				try
				{
					auto stageAst = reflector.getAst(shaderStage);

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

	if (m_enableDebugRenderer != settings.enableDebugRenderer)
	{
		m_enableDebugRenderer = settings.enableDebugRenderer;

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

	// We are creating the shadowmap here because the size can change
	if (m_currentShadowMapSize != m_shadowMapSize)
	{
		Image::Settings imageSettings;
		imageSettings.format = ImageFormat::D16_UNORM;
		imageSettings.width = m_shadowMapSize;
		imageSettings.height = m_shadowMapSize;
		imageSettings.layers = SHADOW_CASCADE_COUNT;

		m_shadowMap = Image::create(imageSettings);

		m_currentShadowMapSize = m_shadowMapSize;
	}

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

	std::array<FrameGraphTextureHandle, SHADOW_CASCADE_COUNT> shadowCascades;
	for (size_t i = 0; i < SHADOW_CASCADE_COUNT; i++)
		shadowCascades[i] = frameGraphBuilder.importTexture(m_shadowMap, i);

	auto getDebugTexture = [&](Settings::DebugView debugView) -> FrameGraphTextureHandle
	{
		switch (debugView)
		{
			case Settings::DebugView::GBufferPosition: return gbufferTextures[0];
			case Settings::DebugView::GBufferNormal: return gbufferTextures[1];
			case Settings::DebugView::GBufferColor: return gbufferTextures[2];
			case Settings::DebugView::GBufferAO: return gbufferTextures[2];
			case Settings::DebugView::GBufferEmissive: return gbufferTextures[3];
			case Settings::DebugView::GBufferMetalness: return gbufferTextures[0];
			case Settings::DebugView::GBufferRoughness: return gbufferTextures[1];
			case Settings::DebugView::ShadowCascade1: return shadowCascades[0];
			case Settings::DebugView::ShadowCascade2: return shadowCascades[1];
			case Settings::DebugView::ShadowCascade3: return shadowCascades[2];
			case Settings::DebugView::ShadowCascade4: return shadowCascades[3];
			case Settings::DebugView::ShadowCascade5: return shadowCascades[4];
			case Settings::DebugView::ShadowCascade6: return shadowCascades[5];
			case Settings::DebugView::ShadowCascade7: return shadowCascades[6];
			case Settings::DebugView::ShadowCascade8: return shadowCascades[7];
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

				std::vector<size_t> visibleEntities;
				visibleEntities.reserve(entities.size());
				std::vector<size_t> entityIndices;
				entityIndices.reserve(entities.size());
				std::vector<MeshDrawData> drawDatas;

				size_t totalMeshCount = 0;
				size_t totalTriangleCount = 0;
				size_t visibleMeshCount = 0;
				size_t visibleTriangleCount = 0;
				{
					ATEMA_BENCHMARK_TAG(btest, "Cull models");
					size_t entityIndex = 0;

					for (const auto& entity : entities)
					{
						const auto& graphics = entities.get<GraphicsComponent>(entity);

						const auto meshCount = graphics.model->getMeshes().size();
						const auto triangleCount = graphics.model->getTriangleCount();

						totalMeshCount += meshCount;
						totalTriangleCount += triangleCount;

						if (m_cullFunction(graphics.aabb))
						{
							entityIndex++;
							continue;
						}

						visibleEntities.emplace_back(entity);
						entityIndices.emplace_back(entityIndex++);

						visibleMeshCount += meshCount;
						visibleTriangleCount += triangleCount;
					}
				}

				auto& stats = Stats::instance();

				//stats["Models"] += visibleEntities.size();
				//stats["Models (culled)"] += entities.size() - visibleEntities.size();
				//stats["Meshes"] += visibleMeshCount;
				//stats["Meshes (culled)"] += totalMeshCount - visibleMeshCount;
				stats["Triangles"] += visibleTriangleCount;
				//stats["Triangles (culled)"] += totalTriangleCount - visibleTriangleCount;

				visibleTriangleCount = 0;

				{
					ATEMA_BENCHMARK_TAG(btest, "Prepare renderables");
					drawDatas.reserve(visibleMeshCount);

					size_t entityIndex = 0;
					for (const auto& entity : visibleEntities)
					{
						const auto& graphics = entities.get<GraphicsComponent>(entity);
						const auto& transform = entities.get<Transform>(entity);

						for (const auto& mesh : graphics.model->getMeshes())
						{
							/*if (m_cullFunction(transform.getMatrix() * mesh->getAABB()))
							{
								continue;
							}*/
							
							auto& drawData = drawDatas.emplace_back();

							drawData.vertexBuffer = mesh->getVertexBuffer().get();
							drawData.indexBuffer = mesh->getIndexBuffer().get();
							drawData.materialInstance = graphics.materials[mesh->getMaterialID()]->materialInstance.get();
							drawData.entityHandle = entity;
							drawData.index = entityIndices[entityIndex];
							drawData.renderPriority = drawData.getRenderPriority();

							//visibleTriangleCount += mesh->getTriangleCount();
						}

						entityIndex++;
					}
				}

				//stats["Triangles"] += visibleTriangleCount;

				{
					ATEMA_BENCHMARK_TAG(btest, "Sort draw data");
					
					std::sort(drawDatas.begin(), drawDatas.end());
				}

				if (drawDatas.empty())
					return;

				std::vector<Ptr<Task>> tasks;
				tasks.reserve(threadCount);

				std::vector<Ptr<CommandBuffer>> commandBuffers;
				commandBuffers.resize(threadCount);

				auto& taskManager = TaskManager::instance();

				size_t firstIndex = 0;
				const size_t size = drawDatas.size() / threadCount;

				for (size_t taskIndex = 0; taskIndex < threadCount; taskIndex++)
				{
					auto lastIndex = firstIndex + size;

					if (taskIndex == threadCount - 1)
					{
						const auto remainingSize = drawDatas.size() - lastIndex;

						lastIndex += remainingSize;
					}

					auto task = taskManager.createTask([this, &context, taskIndex, firstIndex, lastIndex, &drawDatas, &commandBuffers, &frameData, frameIndex](size_t threadIndex)
						{
							auto commandBuffer = context.createSecondaryCommandBuffer(threadIndex);

							commandBuffer->setViewport(m_viewport);

							commandBuffer->setScissor(Vector2i(), m_windowSize);

							auto currentMaterialID = SurfaceMaterial::InvalidID;
							auto currentMaterialInstanceID = SurfaceMaterial::InvalidID;

							// Initialize material to the first one and bind global frame data shared across all pipelines
							{
								auto& materialInstance = drawDatas[firstIndex].materialInstance;
								auto& material = materialInstance->getMaterial();

								material->bindTo(*commandBuffer);
								materialInstance->bindTo(*commandBuffer);

								currentMaterialID = material->getID();
								currentMaterialInstanceID = materialInstance->getID();
							}

							commandBuffer->bindDescriptorSet(SurfaceMaterial::FrameSetIndex, *frameData.frameDescriptorSet);

							uint32_t i = static_cast<uint32_t>(firstIndex);
							for (auto it = std::next(drawDatas.begin(), firstIndex); it != std::next(drawDatas.begin(), lastIndex); it++, i++)
							{
								auto& drawData = *it;

								commandBuffer->bindDescriptorSet(SurfaceMaterial::ObjectSetIndex, *frameData.objectDescriptorSet, { drawData.index * m_dynamicObjectBufferOffset });

								auto& materialInstance = drawData.materialInstance;
								auto& material = materialInstance->getMaterial();

								auto materialID = material->getID();
								auto materialInstanceID = materialInstance->getID();

								if (materialID != currentMaterialID)
								{
									material->bindTo(*commandBuffer);

									currentMaterialID = materialID;

									currentMaterialInstanceID = SurfaceMaterial::InvalidID;
								}

								if (materialInstanceID != currentMaterialInstanceID)
								{
									materialInstance->bindTo(*commandBuffer);

									currentMaterialInstanceID = materialInstanceID;
								}

								const auto& vertexBuffer = drawData.vertexBuffer;
								const auto& indexBuffer = drawData.indexBuffer;

								commandBuffer->bindVertexBuffer(*vertexBuffer->getBuffer(), 0);

								commandBuffer->bindIndexBuffer(*indexBuffer->getBuffer(), indexBuffer->getIndexType());

								commandBuffer->drawIndexed(indexBuffer->getSize());
							}

							commandBuffer->end();

							commandBuffers[taskIndex] = commandBuffer;
						});

					tasks.push_back(task);

					firstIndex += size;
				}

				{
					ATEMA_BENCHMARK_TAG(executeTasks, "Execute tasks")
			
					for (auto& task : tasks)
						task->wait();
				}

				commandBuffer.executeSecondaryCommands(commandBuffers);
			});
	}

	// Shadow Map
	for (size_t cascadeIndex = 0; cascadeIndex < SHADOW_CASCADE_COUNT; cascadeIndex++)
	{
		auto& pass = frameGraphBuilder.createPass("shadow cascade #" + std::to_string(cascadeIndex + 1));

		pass.setDepthTexture(shadowCascades[cascadeIndex], DepthStencil(1.0f, 0));

		pass.enableSecondaryCommandBuffers(true);

		pass.setExecutionCallback([this, cascadeIndex](FrameGraphContext& context)
			{
				ATEMA_BENCHMARK("CommandBuffer (shadows)");
				ATEMA_BENCHMARK_TAG(cascadeBenchmark, "Shadow cascade #" + std::to_string(cascadeIndex + 1) + ")");

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

					auto task = taskManager.createTask([this, &context, cascadeIndex, taskIndex, firstIndex, lastIndex, &entities, &commandBuffers, &frameData, frameIndex](size_t threadIndex)
						{
							auto commandBuffer = context.createSecondaryCommandBuffer(threadIndex);

							commandBuffer->bindPipeline(*m_shadowPipeline);

							commandBuffer->setViewport(m_shadowViewport);

							commandBuffer->setScissor(Vector2i(), { m_shadowMapSize, m_shadowMapSize });

							commandBuffer->bindDescriptorSet(SurfaceMaterial::MaterialSetIndex, *frameData.shadowSet, { static_cast<uint32_t>(cascadeIndex) * m_dynamicShadowBufferOffset });

							commandBuffer->bindDescriptorSet(SurfaceMaterial::FrameSetIndex, *frameData.frameDescriptorSet);
							
							uint32_t i = static_cast<uint32_t>(firstIndex);
							for (auto it = entities.begin() + firstIndex; it != entities.begin() + lastIndex; it++, i++)
							{
								auto& graphics = entities.get<GraphicsComponent>(*it);

								if (!graphics.castShadows)
									continue;

								// Frustum culling
								if (!m_lightFrustums[cascadeIndex].contains(graphics.aabb))
									continue;

								commandBuffer->bindDescriptorSet(SurfaceMaterial::ObjectSetIndex, *frameData.objectDescriptorSet, { i * m_dynamicObjectBufferOffset });

								for (const auto& mesh : graphics.model->getMeshes())
								{
									const auto& vertexBuffer = mesh->getVertexBuffer();
									const auto& indexBuffer = mesh->getIndexBuffer();

									commandBuffer->bindVertexBuffer(*vertexBuffer->getBuffer(), 0);

									commandBuffer->bindIndexBuffer(*indexBuffer->getBuffer(), indexBuffer->getIndexType());

									commandBuffer->drawIndexed(indexBuffer->getSize());
								}
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

		for (const auto& shadowCascade : shadowCascades)
			pass.addSampledTexture(shadowCascade, ShaderStage::Fragment);

		pass.addOutputTexture(phongOutputTexture, 0, Color::Blue);
		pass.setDepthTexture(gbufferDepthTexture);

		pass.setExecutionCallback([this, gbufferTextures](FrameGraphContext& context)
			{
				ATEMA_BENCHMARK("CommandBuffer (phong lighting)");

				const auto frameIndex = context.getFrameIndex();
				const auto& frameData = m_frameDatas[frameIndex];

				auto gbufferSMSet = m_gbufferShadowMapLayout->createSet();
				auto phongSet = m_phongLayout->createSet();

				// Write descriptor sets
				{
					for (uint32_t i = 0; i < gbufferTextures.size(); i++)
						gbufferSMSet->update(i, *context.getImageView(gbufferTextures[i]), *m_ppSampler);

					gbufferSMSet->update(shadowMapBindingIndex, *m_shadowMap->getView(), *m_shadowMapSampler);

					phongSet->update(0, *frameData.shadowCascadesBuffer);
					phongSet->update(1, *frameData.phongBuffer);
				}

				auto& commandBuffer = context.getCommandBuffer();

				commandBuffer.bindPipeline(*m_phongPipeline);

				commandBuffer.setViewport(m_viewport);

				commandBuffer.setScissor(Vector2i(), m_windowSize);

				commandBuffer.bindVertexBuffer(*m_ppQuad, 0);

				commandBuffer.bindDescriptorSet(0, *gbufferSMSet);
				commandBuffer.bindDescriptorSet(1, *phongSet);

				commandBuffer.draw(static_cast<uint32_t>(quadVertices.size()));

				context.destroyAfterUse(std::move(gbufferSMSet));
				context.destroyAfterUse(std::move(phongSet));
			});
	}

	// Debug (DebugRenderer)
	if (m_enableDebugRenderer)
	{
		auto& pass = frameGraphBuilder.createPass("debug renderer");

		pass.addOutputTexture(phongOutputTexture, 0);

		pass.setDepthTexture(gbufferDepthTexture);

		pass.setExecutionCallback([this](FrameGraphContext& context)
			{
				ATEMA_BENCHMARK("CommandBuffer (debug renderer)");

				auto& commandBuffer = context.getCommandBuffer();

				commandBuffer.setViewport(m_viewport);

				commandBuffer.setScissor(Vector2i(), m_windowSize);

				auto& entityManager = getEntityManager();

				auto entities = entityManager.getUnion<Transform, GraphicsComponent>();

				m_debugRenderer->clear();

				m_debugRenderer->draw(m_lightFrustums[0], Color(0.0f, 1.0f, 0.0f));
				m_debugRenderer->draw(m_lightFrustums[1], Color(0.0f, 0.0f, 1.0f));
				m_debugRenderer->draw(m_lightFrustums[2], Color(1.0f, 1.0f, 0.0f));
				m_debugRenderer->draw(m_lightFrustums[3], Color(1.0f, 0.0f, 0.0f));

				if (Settings::instance().customFrustumCulling)
					m_debugRenderer->draw(m_customfrustum, Color::Red);

				for (auto& entity : entities)
				{
					auto& graphics = entities.get<GraphicsComponent>(entity);
					auto& transform = entities.get<Transform>(entity);

					m_debugRenderer->draw(graphics.aabb, Color::Green);

					// Enable this to show all meshes bounding boxes for a given model
					/*
					const auto& meshes = graphics.model->getMeshes();

					if (meshes.size() > 1)
					{
						for (const auto& mesh : meshes)
							m_debugRenderer->draw(transform.getMatrix() * mesh->getAABB(), Color::Gray);
					}
					//*/
				}

				m_debugRenderer->render(context, commandBuffer, m_viewProjection);
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

				descriptorSet->update(0, *context.getImageView(debugTexture), *m_ppSampler);

				auto& commandBuffer = context.getCommandBuffer();

				commandBuffer.bindPipeline(*m_screenPipeline);

				commandBuffer.setScissor(Vector2i(), m_windowSize);

				Viewport viewport;
				viewport.position.x = 10.0f;
				viewport.position.y = static_cast<float>(2 * m_windowSize.y / 3) - 10.0f;
				viewport.size = { m_windowSize.x / 3, m_windowSize.y / 3 };

				commandBuffer.setViewport(viewport);

				commandBuffer.bindDescriptorSet(0, *descriptorSet);

				commandBuffer.bindVertexBuffer(*m_ppQuad, 0);

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

				commandBuffer.bindPipeline(*m_screenPipeline);

				commandBuffer.setScissor(Vector2i(), m_windowSize);

				commandBuffer.bindVertexBuffer(*m_ppQuad, 0);

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

						descriptorSet->update(0, *context.getImageView(debugTextures[i]), *m_ppSampler);

						commandBuffer.bindDescriptorSet(0, *descriptorSet);

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

		pass.enableRenderFrameOutput(true);

		pass.setExecutionCallback([this, phongOutputTexture](FrameGraphContext& context)
			{
				ATEMA_BENCHMARK("CommandBuffer (screen + UI)");

				auto descriptorSet1 = m_screenLayout->createSet();

				descriptorSet1->update(0, *context.getImageView(phongOutputTexture), *m_ppSampler);

				auto& commandBuffer = context.getCommandBuffer();

				commandBuffer.bindPipeline(*m_screenPipeline);

				commandBuffer.setViewport(m_viewport);

				commandBuffer.setScissor(Vector2i(), m_windowSize);

				commandBuffer.bindDescriptorSet(0, *descriptorSet1);

				commandBuffer.bindVertexBuffer(*m_ppQuad, 0);

				commandBuffer.draw(static_cast<uint32_t>(quadVertices.size()));

				// UI
				UiContext::instance().renderDrawData(ImGui::GetDrawData(), commandBuffer);

				context.destroyAfterUse(std::move(descriptorSet1));
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

	// Update bounding boxes
	updateBoundingBoxes();

	// Update scene data
	updateUniformBuffers(frameData);

	benchmark.start("FrameGraph::execute");

	m_frameGraph->execute(renderFrame);
}

void GraphicsSystem::updateBoundingBoxes()
{
	ATEMA_BENCHMARK("Update AABBs");
	
	auto& entityManager = getEntityManager();

	auto entities = entityManager.getUnion<Transform, GraphicsComponent>();

	auto& taskManager = TaskManager::instance();

	std::vector<Ptr<Task>> tasks;
	tasks.reserve(threadCount);

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

		auto task = taskManager.createTask([this, &entities, firstIndex, lastIndex](size_t threadIndex)
			{
				for (auto it = entities.begin() + firstIndex; it != entities.begin() + lastIndex; it++)
				{
					auto& transform = entities.get<Transform>(*it);
					auto& graphics = entities.get<GraphicsComponent>(*it);

					graphics.aabb = transform.getMatrix() * graphics.model->getAABB();
				}
			});

		tasks.push_back(task);

		firstIndex += size;
	}

	for (auto& task : tasks)
		task->wait();
}

void GraphicsSystem::updateUniformBuffers(FrameData& frameData)
{
	ATEMA_BENCHMARK("GraphicsSystem::updateUniformBuffers");

	Vector3f cameraPos;
	Vector3f cameraTarget;

	Matrix4f viewMatrix;
	Matrix4f projMatrix;
	Frustumf cameraFrustum;

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

				viewMatrix = Matrix4f::createLookAt(cameraPos, cameraTarget, cameraUp);
				projMatrix = Matrix4f::createPerspective(Math::toRadians(camera.fov), camera.aspectRatio, camera.nearPlane, camera.farPlane);
				projMatrix[1][1] *= -1;

				mapMemory<Matrix4f>(data, viewOffset) = viewMatrix;
				mapMemory<Matrix4f>(data, projOffset) = projMatrix;
				mapMemory<Vector3f>(data, cameraPositionOffset) = { cameraPos.x, cameraPos.y, cameraPos.z, 1.0f };

				// Update view projection matrix
				m_viewProjection = mapMemory<Matrix4f>(data, projOffset) * mapMemory<Matrix4f>(data, viewOffset);

				cameraFrustum.set(m_viewProjection);

				// Update custom frustum
				if (Settings::instance().customFrustumCulling)
				{
					auto frustumDir2D = toCartesian(Vector2f(1.0f, m_frustumRotation));

					auto origin = Vector3f(0, 0, 0);
					auto dir = Vector3f(frustumDir2D.x, frustumDir2D.y, 0).normalize();

					float nearZ = 0.1f;
					float farZ = 500.0f;

					auto center = origin + dir * (farZ - nearZ) / 2.0f;

					auto view = Matrix4f::createLookAt(origin, origin + dir, Vector3f(0, 0, 1));
					auto proj = Matrix4f::createPerspective(Math::toRadians(70.0f), camera.aspectRatio, nearZ, farZ);
					
					m_customfrustum.set(proj * view);

					m_cullFunction = [this, cameraFrustum](const AABBf& aabb)
					{
						return !cameraFrustum.contains(aabb) || !m_customfrustum.contains(aabb);
					};
				}
				else
				{
					m_cullFunction = [this, cameraFrustum](const AABBf& aabb)
					{
						return !cameraFrustum.contains(aabb);
					};
				}

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
		frameData.objectsUniformBuffer = Buffer::create({ BufferUsage::Uniform | BufferUsage::Map, static_cast<size_t>(frameData.objectCount * m_dynamicObjectBufferOffset) });

		// Update descriptor set
		frameData.objectDescriptorSet->update(0, *frameData.objectsUniformBuffer, 0, m_elementByteSize);
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

	// Update shadow data
	{
		/*struct ShadowData
		{
			mat4f view;
			mat4f cascadeViewProj[ShadowMapCascadeCount];
			float cascadeDepth[ShadowMapCascadeCount];
			float cascadeDepthBias[ShadowMapCascadeCount];
		}*/

		BufferLayout shadowLayout(StructLayout::Default);
		auto viewOffset = shadowLayout.addMatrix(BufferElementType::Float, 4, 4);
		auto cascadeMVPOffset = shadowLayout.addMatrixArray(BufferElementType::Float, 4, 4, true, SHADOW_CASCADE_COUNT);
		auto cascadeDepthOffset = shadowLayout.addArray(BufferElementType::Float, SHADOW_CASCADE_COUNT);
		auto cascadeDepthBiasOffset = shadowLayout.addArray(BufferElementType::Float, SHADOW_CASCADE_COUNT);

		auto shadowData = frameData.shadowBuffer->map();
		auto shadowCascadeData = frameData.shadowCascadesBuffer->map();

		MemoryMapper shadowMVPMapper(shadowData, 0, m_dynamicShadowBufferOffset);

		mapMemory<Matrix4f>(shadowCascadeData, viewOffset) = viewMatrix;
		MemoryMapper cascadeMVPMapper(shadowCascadeData, cascadeMVPOffset, sizeof(Matrix4f));
		MemoryMapper cascadeDepthMapper(shadowCascadeData, cascadeDepthOffset, shadowLayout.getArrayAlignment(BufferElementType::Float));
		MemoryMapper cascadeDepthBiasMapper(shadowCascadeData, cascadeDepthBiasOffset, shadowLayout.getArrayAlignment(BufferElementType::Float));
		
		const float maxFarDepth = 1000.0f;

		const float depthStep = maxFarDepth / static_cast<float>(std::pow(2, SHADOW_CASCADE_COUNT));

		float previousDepth = 0.01f;

		const auto lightView = Matrix4f::createLookAt(-lightDirection * 1000.0f, Vector3f(0, 0, 0), Vector3f(0, 0, 1));

		const auto view = Matrix4f::createLookAt(Vector3f(0, 0, 0), Vector3f(1, 0, 0), Vector3f(0, 0, 1));

		Frustumf frustumRef(projMatrix * view);

		auto frustumPlanesRef = frustumRef.getPlanes();
		auto& nearPlaneRef = frustumPlanesRef[static_cast<size_t>(FrustumPlane::Near)];
		auto& farPlaneRef = frustumPlanesRef[static_cast<size_t>(FrustumPlane::Far)];
		auto& viewDirRef = nearPlaneRef.getNormal();

		auto frustumPlanes = cameraFrustum.getPlanes();
		auto& nearPlane = frustumPlanes[static_cast<size_t>(FrustumPlane::Near)];
		auto& farPlane = frustumPlanes[static_cast<size_t>(FrustumPlane::Far)];
		auto& viewDir = nearPlane.getNormal();

		const auto& baseDepthBias = Settings::instance().baseDepthBias;

		float currentFar = 0.0f;
		for (size_t i = 0; i < SHADOW_CASCADE_COUNT; i++)
		{
			currentFar += depthStep * static_cast<float>(std::pow(2, i));
			cascadeDepthMapper.map<float>(i) = currentFar;

			const auto& depth = cascadeDepthMapper.map<float>(i);
			auto& depthBias = cascadeDepthBiasMapper.map<float>(i);
			auto& mvp = cascadeMVPMapper.map<Matrix4f>(i);

			nearPlane.set(nearPlane.getNormal(), cameraPos + viewDir * previousDepth);
			farPlane.set(farPlane.getNormal(), cameraPos + viewDir * depth);

			nearPlaneRef.set(nearPlaneRef.getNormal(), viewDirRef * previousDepth);
			farPlaneRef.set(farPlaneRef.getNormal(), viewDirRef * depth);

			Frustumf frustum(frustumPlanes);

			frustumRef.set(frustumPlanesRef);

			// Find the tightest box possible around the frustum
			Vector3f boundingBoxSize;
			boundingBoxSize.x = (frustumRef.getCorner(FrustumCorner::FarTopLeft) - frustumRef.getCorner(FrustumCorner::FarTopRight)).getNorm();
			boundingBoxSize.y = (frustumRef.getCorner(FrustumCorner::FarTopLeft) - frustumRef.getCorner(FrustumCorner::FarBottomLeft)).getNorm();
			boundingBoxSize.z = depth - previousDepth;

			// Find the bounding sphere of this box (and of the frustum)
			// This way the shadowmap remains coherent in size no matter the orientation
			const auto sphereCenter = cameraPos + viewDir * (previousDepth + depth) / 2.0f;

			auto sphereRadius = boundingBoxSize.getNorm() / 2.0f;

			const float texelSizeWorldSpace = sphereRadius / static_cast<float>(m_shadowMapSize / 2);

			const auto factor = static_cast<float>(m_shadowMapSize) / static_cast<float>(m_shadowMapSize - 1);
			sphereRadius = sphereRadius * factor;

			// We move the center of the shadowmap according to its resolution (avoid shadow shimmering)
			auto centerOffsetLightSpace = lightView.transformVector(sphereCenter);
			centerOffsetLightSpace.x = texelSizeWorldSpace * std::round(centerOffsetLightSpace.x / texelSizeWorldSpace);
			centerOffsetLightSpace.y = texelSizeWorldSpace * std::round(centerOffsetLightSpace.y / texelSizeWorldSpace);
			centerOffsetLightSpace.z = texelSizeWorldSpace * std::round(centerOffsetLightSpace.z / texelSizeWorldSpace);
			
			// Lightspace frustum bounding box
			AABBf aabb;
			for (const auto& corner : frustum.getCorners())
				aabb.extend(lightView.transformPosition(corner));

			auto c = centerOffsetLightSpace;
			const auto r = sphereRadius;

			// Keep the Z range as tight as possible
			auto proj = Matrix4f::createOrtho(c.x - r, c.x + r, -c.y - r, -c.y + r, -aabb.max.z, -aabb.min.z);
			proj[1][1] *= -1;

			mvp = proj * lightView;

			shadowMVPMapper.map<Matrix4f>(i) = mvp;

			// For the culling we use another projection :
			// - reduced X/Y size to draw less objects
			// - increased Z size in case a caster is between the view frustum & the light
			c = aabb.getCenter();
			const auto s = aabb.getSize() / 2.0f;
			auto cullingProj = Matrix4f::createOrtho(c.x - s.x, c.x + s.x, -c.y - s.y, -c.y + s.y, 0.0f, -aabb.min.z);
			cullingProj[1][1] *= -1;

			// Depth bias depends on the base depth bias and the z difference in light space
			const auto diffZ = aabb.getSize().z;
			depthBias = baseDepthBias / diffZ;

			m_lightFrustums[i].set(cullingProj * lightView);

			previousDepth = depth;
		}

		frameData.shadowBuffer->unmap();
		frameData.shadowCascadesBuffer->unmap();
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
		mapMemory<float>(data, ambientStrengthOffset) = 0.35f;

		frameData.phongBuffer->unmap();
	}
}
