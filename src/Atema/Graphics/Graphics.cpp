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

#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/Loaders/DefaultImageLoader.hpp>
#include <Atema/Shader/Loaders/AtslLoader.hpp>
#include <Atema/Renderer/Renderer.hpp>
#include <Atema/Renderer/Buffer.hpp>
#include <Atema/Renderer/BufferLayout.hpp>
#include <Atema/Renderer/Utils.hpp>
#include <Atema/Shader/Atsl/AtslParser.hpp>
#include <Atema/Shader/Atsl/AtslToAstConverter.hpp>
#include <Atema/Graphics/VertexBuffer.hpp>
#include <Atema/Graphics/VertexTypes.hpp>

#include <filesystem>
#include <functional>
#include <map>
#include <optional>

using namespace at;

namespace
{
	const char PostProcessShader[] = R"(
option
{
	int AtPostProcessTexCoordsLocation = 0;
	int AtPostProcessOutColorLocation = 0;
}

[stage(vertex)]
input
{
	[location(0)] vec3f _atPostProcessInPosition;
	[location(1)] vec2f _atPostProcessInTexCoords;
}

[stage(vertex)]
output
{
	[location(AtPostProcessTexCoordsLocation)] vec2f _atPostProcessOutTexCoords;
}

[entry(vertex)]
void main()
{
	_atPostProcessOutTexCoords = _atPostProcessInTexCoords;
	
	setVertexPosition(vec4f(_atPostProcessInPosition, 1.0));
}

[stage(fragment)]
input
{
	[location(AtPostProcessTexCoordsLocation)] vec2f _atPostProcessInTexCoords;
}

[stage(fragment)]
output
{
	[location(AtPostProcessOutColorLocation)] vec4f _atPostProcessOutColor;
}

vec4f getPostProcessColor(vec2f uv);

[entry(fragment)]
void main()
{
	_atPostProcessOutColor = getPostProcessColor(_atPostProcessInTexCoords);
}
)";

	const char CubemapPassShader[] = R"(
option
{
	int CubemapTopIndex = 0;
	int CubemapLeftIndex = 1;
	int CubemapFrontIndex = 2;
	int CubemapRightIndex = 3;
	int CubemapBackIndex = 4;
	int CubemapBottomIndex = 5;
}

struct CubeDataStruct
{
	mat4f Transforms[6];
}

external
{
	[set(0), binding(0)] CubeDataStruct CubeData;
}

[stage(vertex)]
input
{
	[location(0)] vec3f inPosition;
}

[stage(vertex)]
output
{
	[location(CubemapTopIndex)] vec3f uvwTop;
	[location(CubemapLeftIndex)] vec3f uvwLeft;
	[location(CubemapFrontIndex)] vec3f uvwFront;
	[location(CubemapRightIndex)] vec3f uvwRight;
	[location(CubemapBackIndex)] vec3f uvwBack;
	[location(CubemapBottomIndex)] vec3f uvwBottom;
}

[entry(vertex)]
void main()
{
	vec3f pos = vec3f(inPosition.x, inPosition.z, inPosition.y);
	
	uvwTop = (CubeData.Transforms[0] * vec4f(pos, 1.0)).xzy;
	uvwLeft = (CubeData.Transforms[1] * vec4f(pos, 1.0)).xzy;
	uvwFront = (CubeData.Transforms[2] * vec4f(pos, 1.0)).xzy;
	uvwRight = (CubeData.Transforms[3] * vec4f(pos, 1.0)).xzy;
	uvwBack = (CubeData.Transforms[4] * vec4f(pos, 1.0)).xzy;
	uvwBottom = (CubeData.Transforms[5] * vec4f(pos, 1.0)).xzy;
	
	setVertexPosition(vec4f(inPosition.x, inPosition.y, 0.0, 1.0));
}

[stage(fragment)]
input
{
	[location(CubemapTopIndex)] vec3f uvwTop;
	[location(CubemapLeftIndex)] vec3f uvwLeft;
	[location(CubemapFrontIndex)] vec3f uvwFront;
	[location(CubemapRightIndex)] vec3f uvwRight;
	[location(CubemapBackIndex)] vec3f uvwBack;
	[location(CubemapBottomIndex)] vec3f uvwBottom;
}

[stage(fragment)]
output
{
	[location(CubemapTopIndex)] vec4f FaceTop;
	[location(CubemapLeftIndex)] vec4f FaceLeft;
	[location(CubemapFrontIndex)] vec4f FaceFront;
	[location(CubemapRightIndex)] vec4f FaceRight;
	[location(CubemapBackIndex)] vec4f FaceBack;
	[location(CubemapBottomIndex)] vec4f FaceBottom;
}

vec4f getCubemapFaceColor(int faceIndex, vec3f uvw);

[entry(fragment)]
void main()
{
	FaceTop = getCubemapFaceColor(CubemapTopIndex, normalize(uvwTop));
	FaceLeft = getCubemapFaceColor(CubemapLeftIndex, normalize(uvwLeft));
	FaceFront = getCubemapFaceColor(CubemapFrontIndex, normalize(uvwFront));
	FaceRight = getCubemapFaceColor(CubemapRightIndex, normalize(uvwRight));
	FaceBack = getCubemapFaceColor(CubemapBackIndex, normalize(uvwBack));
	FaceBottom = getCubemapFaceColor(CubemapBottomIndex, normalize(uvwBottom));
}
)";

	const std::unordered_map<std::string, const char*> s_shaderLibraries =
	{
		{ "Atema.PostProcess", PostProcessShader },
		{ "Atema.CubemapPass", CubemapPassShader },
	};

	struct SurfaceMaterialParameter
	{
		const Ptr<Image>& image;
		std::optional<ConstantValue> value;
	};

	BufferElementType getElementType(const ConstantValue& constantValue)
	{
		if (constantValue.is<bool>())
			return BufferElementType::Bool;
		else if (constantValue.is<int32_t>())
			return BufferElementType::Int;
		else if (constantValue.is<uint32_t>())
			return BufferElementType::UInt;
		else if (constantValue.is<float>())
			return BufferElementType::Float;
		else if (constantValue.is<Vector2i>())
			return BufferElementType::Int2;
		else if (constantValue.is<Vector2u>())
			return BufferElementType::UInt2;
		else if (constantValue.is<Vector2f>())
			return BufferElementType::Float2;
		else if (constantValue.is<Vector3i>())
			return BufferElementType::Int3;
		else if (constantValue.is<Vector3u>())
			return BufferElementType::UInt3;
		else if (constantValue.is<Vector3f>())
			return BufferElementType::Float3;
		else if (constantValue.is<Vector4i>())
			return BufferElementType::Int4;
		else if (constantValue.is<Vector4u>())
			return BufferElementType::UInt4;
		else if (constantValue.is<Vector4f>())
			return BufferElementType::Float4;

		ATEMA_ERROR("Invalid type");

		return BufferElementType::Int;
	}

	std::vector<Vertex_XYZ_UV> quadVertices =
	{
		{{ -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }},
		{{ -1.0f, +1.0f, 0.0f }, { 0.0f, 1.0f }},
		{{ +1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f }},

		{{ +1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f }},
		{{ -1.0f, +1.0f, 0.0f }, { 0.0f, 1.0f }},
		{{ +1.0f, +1.0f, 0.0f }, { 1.0f, 1.0f }}
	};
}

//-----
// UberInstanceSettings
Graphics::UberInstanceSettings::UberInstanceSettings(const UberShader* uberShader, const ShaderLibraryManager* shaderLibraryManager, const std::vector<UberShader::Option>& options) :
	uberShader(uberShader),
	shaderLibraryManager(shaderLibraryManager),
	options(options)
{
}

StdHash Graphics::UberInstanceSettings::hash(const UberInstanceSettings& settings)
{
	// We need to order the options :
	// { {"opt1", 1}, {"opt2", 2} } and { {"opt2", 2}, {"opt1", 1} } must have the same hash
	std::map<std::string_view, const ConstantValue*> orderedOptions;

	for (const auto& option : settings.options)
		orderedOptions[option.name] = &option.value;

	// Combine UberShader instance & options hashes
	size_t hash = 0;

	DefaultStdHasher::hashCombine(hash, settings.uberShader);
	DefaultStdHasher::hashCombine(hash, settings.shaderLibraryManager);

	for (const auto& kv : orderedOptions)
		DefaultStdHasher::hashCombine(hash, *kv.second);

	return hash;
}

//-----
// UberStageSettings
Graphics::UberStageSettings::UberStageSettings(const UberShader* uberShader, AstShaderStage shaderStage) :
	uberShader(uberShader),
	shaderStage(shaderStage)
{
}

//-----
// ImageSettings
Graphics::ImageSettings::ImageSettings(const std::filesystem::path& path, const ImageLoader::Settings& settings) :
	path(path),
	settings(settings)
{
}

StdHash Graphics::ImageSettings::hash(const ImageSettings& settings)
{
	StdHash hash = 0;

	DefaultStdHasher::hashCombine(hash, settings.path);
	DefaultStdHasher::hashCombine(hash, settings.settings.mipLevels);
	DefaultStdHasher::hashCombine(hash, settings.settings.samples);
	DefaultStdHasher::hashCombine(hash, settings.settings.tiling);
	DefaultStdHasher::hashCombine(hash, settings.settings.usages);

	return hash;
}

//-----
// Graphics
Graphics::Graphics()
{
	// Loaders
	m_uberShaderManager.addLoader([this](const std::filesystem::path& path)
		{
			return loadUberShader(path);
		});

	m_uberShaderOptionsManager.addLoader([this](const UberInstanceSettings& settings)
		{
			return loadUberInstance(settings);
		});

	m_uberShaderStageManager.addLoader([this](const UberStageSettings& settings)
		{
			return loadUberStage(settings);
		});

	m_shaderManager.addLoader([this](const UberShader* uberShader)
		{
			return loadShader(*uberShader);
		});

	m_descriptorSetLayoutManager.addLoader([this](const DescriptorSetLayout::Settings& settings)
		{
			return loadDescriptorSetLayout(settings);
		});

	m_graphicsPipelineSettingsManager.addLoader([this](const GraphicsPipelineSettings& settings)
		{
			return loadGraphicsPipelineSettings(settings);
		});

	m_graphicsPipelineManager.addLoader(&loadGraphicsPipeline);

	m_imageManager.addLoader(&loadImage);

	m_samplerManager.addLoader(&loadSampler);

	// Hashers
	m_uberShaderOptionsManager.setHasher(&UberInstanceSettings::hash);

	m_imageManager.setHasher(&ImageSettings::hash);

	// Before uber managers to properly destroy UberShaders after Shader deletion
	m_resourceManagers.emplace_back(&m_graphicsPipelineManager);
	m_resourceManagers.emplace_back(&m_descriptorSetLayoutManager);
	m_resourceManagers.emplace_back(&m_graphicsPipelineSettingsManager);
	m_resourceManagers.emplace_back(&m_shaderManager);
	m_resourceManagers.emplace_back(&m_uberShaderIdManager);
	m_resourceManagers.emplace_back(&m_uberShaderManager);
	m_resourceManagers.emplace_back(&m_uberShaderOptionsManager);
	m_resourceManagers.emplace_back(&m_uberShaderStageManager);
	m_resourceManagers.emplace_back(&m_imageManager);
	m_resourceManagers.emplace_back(&m_samplerManager);

	m_shaderManager.setDeleter([this](Ptr<Shader> shader)
		{
			const auto it = m_shaderToUber.find(shader.get());

			if (it != m_shaderToUber.end())
				m_shaderToUber.erase(it);

			shader.reset();
		});

	auto deleteUberShader = [this](Ptr<UberShader> uberShader)
	{
		destroy(uberShader.get());

		uberShader.reset();
	};

	m_uberShaderIdManager.setDeleter(deleteUberShader);
	m_uberShaderManager.setDeleter(deleteUberShader);
	m_uberShaderOptionsManager.setDeleter(deleteUberShader);
	m_uberShaderStageManager.setDeleter(deleteUberShader);
}

Graphics::~Graphics()
{
}

Graphics& Graphics::instance()
{
	static Graphics s_instance;

	return s_instance;
}

void Graphics::setMaxUnusedCounter(uint32_t counter)
{
	for (auto& resourceManager : m_resourceManagers)
		resourceManager->setMaxUnusedCounter(counter);
}

void Graphics::clearUnused()
{
	for (auto& resourceManager : m_resourceManagers)
		resourceManager->clearUnused();
}

void Graphics::clear()
{
	m_lightingModels.clear();
	m_lightingModelIDs.clear();

	m_quadMesh.reset();

	m_frameLayout.reset();
	m_objectLayout.reset();
	m_lightLayout.reset();
	m_lightShadowLayout.reset();
	
	for (auto& resourceManager : m_resourceManagers)
		resourceManager->clear();
}

void Graphics::initializeShaderLibraries(ShaderLibraryManager& libraryManager)
{
	for (const auto& [libName, lib] : s_shaderLibraries)
	{
		AtslParser parser;

		AtslToAstConverter converter;

		auto ast = converter.createAst(parser.createTokens(lib));

		libraryManager.setLibrary(libName, std::move(ast));
	}
}

void Graphics::addLightingModel(const LightingModel& lightingModel)
{
	ATEMA_ASSERT(m_lightingModels.find(lightingModel.name) == m_lightingModels.end(), "Lighting model already defined");

	m_lightingModels[lightingModel.name] = lightingModel;
	m_lightingModelIDs[lightingModel.name] = lightingModel.id;

	// Initialize default shader library
	const auto& libAst = lightingModel.materialLibrary->getAst();
	const auto& lightLibAst = lightingModel.lightLibrary->getAst();

	AstCloner cloner;

	ShaderLibraryManager::instance().setLibrary("Atema.LightingModel." + lightingModel.name, cloner.clone(*libAst));
	ShaderLibraryManager::instance().setLibrary("Atema.LightingModel." + lightingModel.name + "LightMaterial", cloner.clone(*lightLibAst));
}

const LightingModel& Graphics::getLightingModel(const std::string& name) const
{
	const auto it = m_lightingModels.find(name);

	ATEMA_ASSERT(it != m_lightingModels.end(), "Undefined lighting model");

	return it->second;
}

size_t Graphics::getLightingModelID(const std::string& name) const
{
	const auto it = m_lightingModelIDs.find(name);

	ATEMA_ASSERT(it != m_lightingModelIDs.end(), "Undefined lighting model");

	return it->second;
}

Ptr<VertexBuffer> Graphics::getQuadMesh()
{
	if (!m_quadMesh)
	{
		// Create vertex buffer
		VertexBuffer::Settings vertexBufferSettings;
		vertexBufferSettings.vertexFormat = VertexFormat::create(Vertex_XYZ_UV::VertexFormat);
		vertexBufferSettings.usages = BufferUsage::Vertex | BufferUsage::TransferDst;
		vertexBufferSettings.size = quadVertices.size();
		m_quadMesh = VertexBuffer::create(vertexBufferSettings);

		// Fill staging buffer
		const size_t bufferSize = m_quadMesh->getByteSize();

		auto stagingBuffer = Buffer::create({ BufferUsage::TransferSrc | BufferUsage::Map, bufferSize });

		auto bufferData = stagingBuffer->map();

		memcpy(bufferData, static_cast<void*>(quadVertices.data()), static_cast<size_t>(bufferSize));

		stagingBuffer->unmap();

		// Copy staging buffer to vertex buffer
		auto commandBuffer = Renderer::instance().getCommandPool(QueueType::Graphics)->createBuffer({true});

		commandBuffer->begin();

		commandBuffer->copyBuffer(*stagingBuffer, *m_quadMesh->getBuffer(), bufferSize);

		commandBuffer->end();

		Renderer::instance().submitAndWait({ commandBuffer });
	}

	return m_quadMesh;
}

Ptr<DescriptorSetLayout> Graphics::getFrameLayout()
{
	if (!m_frameLayout)
	{
		DescriptorSetLayout::Settings layoutSettings;
		layoutSettings.bindings =
		{
			{ DescriptorType::UniformBuffer, 0, 1, ShaderStage::Vertex }
		};

		m_frameLayout = getDescriptorSetLayout(layoutSettings);
	}

	return m_frameLayout;
}

Ptr<DescriptorSetLayout> Graphics::getObjectLayout()
{
	if (!m_objectLayout)
	{
		DescriptorSetLayout::Settings layoutSettings;
		layoutSettings.bindings =
		{
			{ DescriptorType::UniformBuffer, 0, 1, ShaderStage::Vertex }
		};

		m_objectLayout = getDescriptorSetLayout(layoutSettings);
	}

	return m_objectLayout;
}

Ptr<DescriptorSetLayout> Graphics::getLightLayout()
{
	if (!m_lightLayout)
	{
		DescriptorSetLayout::Settings layoutSettings;
		layoutSettings.bindings =
		{
			{ DescriptorType::UniformBuffer, 0, 1, ShaderStage::Vertex | ShaderStage::Fragment }
		};

		m_lightLayout = getDescriptorSetLayout(layoutSettings);
	}

	return m_lightLayout;
}

Ptr<DescriptorSetLayout> Graphics::getLightShadowLayout()
{
	if (!m_lightShadowLayout)
	{
		DescriptorSetLayout::Settings layoutSettings;
		layoutSettings.bindings =
		{
			{ DescriptorType::UniformBuffer, 0, 1, ShaderStage::Fragment },
			{ DescriptorType::CombinedImageSampler, 1, 1, ShaderStage::Fragment }
		};

		m_lightShadowLayout = getDescriptorSetLayout(layoutSettings);
	}

	return m_lightShadowLayout;
}

void Graphics::setUberShader(const std::string& identifier, const std::string& shaderCode)
{
	AtslParser parser;

	AtslToAstConverter converter;

	auto ast = converter.createAst(parser.createTokens(shaderCode));

	auto uberShader = std::make_shared<UberShader>(std::move(ast));

	m_uberShaderIdManager.set(identifier, uberShader);

	m_uberShaders[uberShader.get()] = uberShader;

	initializeResourceHandle(uberShader.get());
}

bool Graphics::uberShaderExists(const std::string& identifier) const
{
	return m_uberShaderIdManager.contains(identifier);
}

Ptr<UberShader> Graphics::getUberShader(const std::string& identifier)
{
	return m_uberShaderIdManager.get(identifier);
}

Ptr<UberShader> Graphics::getUberShader(const std::filesystem::path& path)
{
	return m_uberShaderManager.get(path);
}

Ptr<UberShader> Graphics::getUberShader(const UberShader& baseUberShader, const std::vector<UberShader::Option>& options, const ShaderLibraryManager* shaderLibraryManager)
{
	return m_uberShaderOptionsManager.get({ &baseUberShader, shaderLibraryManager, options });
}

Ptr<UberShader> Graphics::getUberShader(const UberShader& baseUberShader, AstShaderStage shaderStage)
{
	return m_uberShaderStageManager.get({ &baseUberShader, shaderStage });
}

Ptr<UberShader> Graphics::getUberShader(const std::filesystem::path& path, AstShaderStage shaderStage, const std::vector<UberShader::Option>& options, const ShaderLibraryManager* shaderLibraryManager)
{
	// Get the raw shader
	auto baseUberShader = getUberShader(path);

	// Preprocess the shader
	baseUberShader = getUberShader(*baseUberShader, options, shaderLibraryManager);

	// Extract the desired stage
	return getUberShader(*baseUberShader, shaderStage);
}

Ptr<UberShader> Graphics::getUberShaderFromString(const std::string& identifier, AstShaderStage shaderStage, const std::vector<UberShader::Option>& options, const ShaderLibraryManager* shaderLibraryManager)
{
	// Get the raw shader
	auto baseUberShader = getUberShader(identifier);

	// Preprocess the shader
	baseUberShader = getUberShader(*baseUberShader, options, shaderLibraryManager);

	// Extract the desired stage
	return getUberShader(*baseUberShader, shaderStage);
}

Ptr<Shader> Graphics::getShader(const UberShader& uberShader)
{
	return m_shaderManager.get(&uberShader);
}

Ptr<DescriptorSetLayout> Graphics::getDescriptorSetLayout(const DescriptorSetLayout::Settings& settings)
{
	return m_descriptorSetLayoutManager.get(settings);
}

Ptr<GraphicsPipeline::Settings> Graphics::getGraphicsPipelineSettings(const UberShader& vertexShader, const UberShader& fragmentShader)
{
	return m_graphicsPipelineSettingsManager.get({ &vertexShader, &fragmentShader });
}

Ptr<GraphicsPipeline> Graphics::getGraphicsPipeline(const GraphicsPipeline::Settings& settings)
{
	return m_graphicsPipelineManager.get(settings);
}

Ptr<Image> Graphics::getImage(const std::filesystem::path& path, const ImageLoader::Settings& settings)
{
	return m_imageManager.get({ path, settings });
}

Ptr<Sampler> Graphics::getSampler(const Sampler::Settings& settings)
{
	return m_samplerManager.get(settings);
}

Ptr<UberShader> Graphics::loadUberShader(const std::filesystem::path& path)
{
	auto uberShader = AtslLoader::load(path);

	m_uberShaders[uberShader.get()] = uberShader;

	initializeResourceHandle(uberShader.get());

	return uberShader;
}

Ptr<UberShader> Graphics::loadUberInstance(const UberInstanceSettings& settings)
{
	auto uberShader = settings.uberShader->createInstance(settings.options, settings.shaderLibraryManager);

	m_uberShaders[uberShader.get()] = uberShader;

	auto& resourceHandle = initializeResourceHandle(uberShader.get());

	auto resourcePtr = uberShader.get();
	onDestroy(settings.uberShader, resourceHandle, [this, resourcePtr]()
		{
			m_uberShaderOptionsManager.remove(resourcePtr);
		});

	return uberShader;
}

Ptr<UberShader> Graphics::loadUberStage(const UberStageSettings& settings)
{
	auto uberShader = settings.uberShader->extractStage(settings.shaderStage);

	m_uberShaders[uberShader.get()] = uberShader;

	auto& resourceHandle = initializeResourceHandle(uberShader.get());

	auto resourcePtr = uberShader.get();
	onDestroy(settings.uberShader, resourceHandle, [this, resourcePtr]()
		{
			m_uberShaderStageManager.remove(resourcePtr);
		});

	return uberShader;
}

Ptr<Shader> Graphics::loadShader(const UberShader& uberShader)
{
	Shader::Settings settings;
	settings.shaderLanguage = ShaderLanguage::Ast;
	settings.shaderData = uberShader.getAst().get();
	settings.shaderDataSize = 1;

	auto shader = Shader::create(settings);

	auto& resourceHandle = initializeResourceHandle(shader.get());

	if (m_shaderToUber.find(shader.get()) == m_shaderToUber.end())
	{
		const auto it = m_uberShaders.find(&uberShader);

		if (it != m_uberShaders.end())
			m_shaderToUber[shader.get()] = it->second.lock();
	}

	auto resourcePtr = shader.get();
	onDestroy(&uberShader, resourceHandle, [this, resourcePtr]()
		{
			m_shaderManager.remove(resourcePtr);
		});

	return shader;
}

Ptr<DescriptorSetLayout> Graphics::loadDescriptorSetLayout(const DescriptorSetLayout::Settings& settings)
{
	auto descriptorSetLayout = DescriptorSetLayout::create(settings);

	initializeResourceHandle(descriptorSetLayout.get());

	return descriptorSetLayout;
}

Ptr<GraphicsPipeline::Settings> Graphics::loadGraphicsPipelineSettings(const GraphicsPipelineSettings& settings)
{
	auto pipelineSettings = std::make_shared<GraphicsPipeline::Settings>();

	auto& resourceHandle = initializeResourceHandle(pipelineSettings.get());

	const auto& vertexReflection = settings.vertexShader->getReflection(AstShaderStage::Vertex);
	const auto& fragmentReflection = settings.fragmentShader->getReflection(AstShaderStage::Fragment);

	// Shaders
	pipelineSettings->vertexShader = getShader(*settings.vertexShader);
	pipelineSettings->fragmentShader = getShader(*settings.fragmentShader);

	// VertexInputState
	for (const auto& input : vertexReflection.getInputs())
		pipelineSettings->state.vertexInput.inputs.emplace_back(0, input.location, getVertexFormat(input.type));

	// DescriptorSetLayouts

	// bindings[setIndex][bindingIndex] = DescriptorSetBinding
	std::map<uint32_t, std::map<uint32_t, DescriptorSetBinding>> bindings;

	for (const auto& external : vertexReflection.getExternals())
	{
		auto& binding = bindings[external.set][external.binding];

		binding.type = getDefaultDescriptorType(external.type);
		binding.count = getDescriptorBindingCount(external.type);
		binding.binding = external.binding;
		binding.shaderStages = ShaderStage::Vertex;
	}

	for (const auto& external : fragmentReflection.getExternals())
	{
		auto& set = bindings[external.set];
		const auto it = set.find(external.binding);

		// If the binding is in the fragment shader only, just create it
		if (it == set.end())
		{
			auto& binding = set[external.binding];

			binding.type = getDefaultDescriptorType(external.type);
			binding.count = getDescriptorBindingCount(external.type);
			binding.binding = external.binding;
			binding.shaderStages = ShaderStage::Fragment;
		}
		// Otherwise ensure the binding matches with the one defined in the vertex shader side
		else
		{
			auto& binding = it->second;

			if (binding.type != getDefaultDescriptorType(external.type))
				ATEMA_ERROR("DescriptorSetBinding (set " + std::to_string(external.set) + ", binding " + std::to_string(external.binding) + ") type must be the same in vertex & fragment shaders");

			if (binding.count != getDescriptorBindingCount(external.type))
				ATEMA_ERROR("DescriptorSetBinding (set " + std::to_string(external.set) + ", binding " + std::to_string(external.binding) + ") count must be the same in vertex & fragment shaders");

			binding.shaderStages |= ShaderStage::Fragment;
		}
	}

	if (!bindings.empty())
		pipelineSettings->descriptorSetLayouts.resize(bindings.rbegin()->first + 1);

	auto resourcePtr = pipelineSettings.get();
	auto callback = [this, resourcePtr]()
	{
		m_graphicsPipelineSettingsManager.remove(resourcePtr);
	};

	// Retrieving DescriptorSetLayouts corresponding to each set index
	// If no set is bound, the layout at this corresponding index is empty
	for (auto& [setIndex, bindingMap] : bindings)
	{
		DescriptorSetLayout::Settings layoutSettings;

		for (auto& [bindingIndex, binding] : bindingMap)
			layoutSettings.bindings.emplace_back(std::move(binding));

		auto descriptorSetLayout = getDescriptorSetLayout(layoutSettings);

		onDestroy(descriptorSetLayout.get(), resourceHandle, callback);

		pipelineSettings->descriptorSetLayouts[setIndex] = std::move(descriptorSetLayout);
	}

	onDestroy(pipelineSettings->vertexShader.get(), resourceHandle, callback);
	onDestroy(pipelineSettings->fragmentShader.get(), resourceHandle, callback);

	return pipelineSettings;
}

Ptr<GraphicsPipeline> Graphics::loadGraphicsPipeline(const GraphicsPipeline::Settings& settings)
{
	return GraphicsPipeline::create(settings);
}

Ptr<Image> Graphics::loadImage(const ImageSettings& settings)
{
	return DefaultImageLoader::load(settings.path, settings.settings);
}

Ptr<Sampler> Graphics::loadSampler(const Sampler::Settings& settings)
{
	return Sampler::create(settings);
}

void Graphics::onDestroy(const UberShader* resource, ResourceHandle& dstHandle, std::function<void()> callback)
{
	auto& srcHandle = getResourceHandle(resource);

	dstHandle.connectionGuard.connect(srcHandle.onDestroy, callback);
}

void Graphics::onDestroy(const Shader* resource, ResourceHandle& dstHandle, std::function<void()> callback)
{
	auto& srcHandle = getResourceHandle(resource);

	dstHandle.connectionGuard.connect(srcHandle.onDestroy, callback);
}

void Graphics::onDestroy(const DescriptorSetLayout* resource, ResourceHandle& dstHandle, std::function<void()> callback)
{
	auto& srcHandle = getResourceHandle(resource);

	dstHandle.connectionGuard.connect(srcHandle.onDestroy, callback);
}

Graphics::ResourceHandle& Graphics::initializeResourceHandle(const void* resource)
{
	const auto pair = m_resourceHandles.emplace(resource, std::make_shared<ResourceHandle>());

	return *(pair.first->second);
}

void Graphics::destroy(UberShader* resource)
{
	// Destroy UberShader reference
	const auto it = m_uberShaders.find(resource);

	if (it != m_uberShaders.end())
		m_uberShaders.erase(it);

	destroy(static_cast<void*>(resource));
}

void Graphics::destroy(void* resource)
{
	const auto resourceIt = m_resourceHandles.find(resource);

	resourceIt->second->onDestroy();

	m_resourceHandles.erase(resourceIt);
}

Graphics::ResourceHandle& Graphics::getResourceHandle(const void* resource)
{
	const auto it = m_resourceHandles.find(resource);

	ATEMA_ASSERT(it != m_resourceHandles.end(), "Requested resource was not found in Graphics instance");

	return *it->second;
}
