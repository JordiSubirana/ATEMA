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
#include <Atema/Graphics/SurfaceMaterial.hpp>
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
	const char atGBufferWrite[] = R"(
option
{
	int AtGBufferWriteLocation = 0;
}

[stage(fragment)]
output
{
	[location(AtGBufferWriteLocation + 0)] vec4f _atGBufferWritePositionMetalness;
	[location(AtGBufferWriteLocation + 1)] vec4f _atGBufferWriteNormalRoughness;
	[location(AtGBufferWriteLocation + 2)] vec4f _atGBufferWriteAlbedoAO;
	[location(AtGBufferWriteLocation + 3)] vec4f _atGBufferWriteEmissive;
}

void atGBufferWritePosition(vec3f value)
{
	_atGBufferWritePositionMetalness.rgb = value;
}

void atGBufferWriteNormal(vec3f value)
{
	_atGBufferWriteNormalRoughness.rgb = value;
}

void atGBufferWriteAlbedo(vec3f value)
{
	_atGBufferWriteAlbedoAO.rgb = value;
}

void atGBufferWriteAO(float value)
{
	_atGBufferWriteAlbedoAO.a = value;
}

void atGBufferWriteEmissive(vec3f value)
{
	_atGBufferWriteEmissive.rgb = value;
}

void atGBufferWriteMetalness(float value)
{
	_atGBufferWritePositionMetalness.a = value;
}

void atGBufferWriteRoughness(float value)
{
	_atGBufferWriteNormalRoughness.a = value;
}
)";

	const char atGBufferRead[] = R"(
option
{
	int AtGBufferReadSet = 0;
	int AtGBufferReadBinding = 0;
}

external
{
	[set(AtGBufferReadSet), binding(AtGBufferReadBinding + 0)] sampler2Df _atGBufferReadPositionMetalness;
	[set(AtGBufferReadSet), binding(AtGBufferReadBinding + 1)] sampler2Df _atGBufferReadNormalRoughness;
	[set(AtGBufferReadSet), binding(AtGBufferReadBinding + 2)] sampler2Df _atGBufferReadAlbedoAO;
	[set(AtGBufferReadSet), binding(AtGBufferReadBinding + 3)] sampler2Df _atGBufferReadEmissive;
}

vec3f atGBufferReadPosition(vec2f uv)
{
	return sample(_atGBufferReadPositionMetalness, uv).rgb;
}

vec3f atGBufferReadNormal(vec2f uv)
{
	return sample(_atGBufferReadNormalRoughness, uv).rgb;
}

vec3f atGBufferReadAlbedo(vec2f uv)
{
	return sample(_atGBufferReadAlbedoAO, uv).rgb;
}

float atGBufferReadAO(vec2f uv)
{
	return sample(_atGBufferReadAlbedoAO, uv).a;
}

vec3f atGBufferReadEmissive(vec2f uv)
{
	return sample(_atGBufferReadEmissive, uv).rgb;
}

float atGBufferReadMetalness(vec2f uv)
{
	return sample(_atGBufferReadPositionMetalness, uv).a;
}

float atGBufferReadRoughness(vec2f uv)
{
	return sample(_atGBufferReadNormalRoughness, uv).a;
}
)";

	const std::unordered_map<std::string, const char*> s_shaderLibraries =
	{
		{ "Atema.GBufferWrite", atGBufferWrite },
		{ "Atema.GBufferRead", atGBufferRead },
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
Graphics::UberInstanceSettings::UberInstanceSettings(const UberShader* uberShader, const std::vector<UberShader::Option>& options) :
	uberShader(uberShader),
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
// DefaultSurfaceMaterialSettings
Graphics::DefaultSurfaceMaterialSettings::DefaultSurfaceMaterialSettings(const SurfaceMaterialData& materialData, uint32_t instanceLayoutPageSize) :
		materialData(materialData),
		instanceLayoutPageSize(instanceLayoutPageSize)
{

}

StdHash at::Graphics::DefaultSurfaceMaterialSettings::hash(const DefaultSurfaceMaterialSettings& settings)
{
	// We just want to know how many textures there are, to have a valid descriptor set layout
	StdHash hash = 0;

	DefaultStdHasher::hashCombine(hash, settings.materialData.colorMap ? 1 : 0);
	DefaultStdHasher::hashCombine(hash, settings.materialData.normalMap ? 1 : 0);
	DefaultStdHasher::hashCombine(hash, settings.materialData.ambientOcclusionMap ? 1 : 0);
	DefaultStdHasher::hashCombine(hash, settings.materialData.heightMap ? 1 : 0);
	DefaultStdHasher::hashCombine(hash, settings.materialData.emissiveMap ? 1 : 0);
	DefaultStdHasher::hashCombine(hash, settings.materialData.metalnessMap ? 1 : 0);
	DefaultStdHasher::hashCombine(hash, settings.materialData.roughnessMap ? 1 : 0);
	DefaultStdHasher::hashCombine(hash, settings.materialData.alphaMaskMap ? 1 : 0);
	DefaultStdHasher::hashCombine(hash, settings.instanceLayoutPageSize);

	return hash;
}

//-----
// DefaultSurfaceInstanceSettings
Graphics::DefaultSurfaceInstanceSettings::DefaultSurfaceInstanceSettings(const Ptr<SurfaceMaterial>& material, const SurfaceMaterialData& materialData) :
	material(material),
	materialData(materialData)
{

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

	m_descriptorSetLayoutManager.addLoader(&loadDescriptorSetLayout);

	m_graphicsPipelineSettingsManager.addLoader([this](const GraphicsPipelineSettings& settings)
		{
			return loadGraphicsPipelineSettings(settings);
		});

	m_graphicsPipelineManager.addLoader(&loadGraphicsPipeline);

	m_imageManager.addLoader(&loadImage);

	m_samplerManager.addLoader(&loadSampler);

	m_defaultSurfaceMaterialSettingsManager.addLoader([this](const DefaultSurfaceMaterialSettings& settings)
		{
			return loadSurfaceMaterial(settings);
		});

	m_defaultSurfaceInstanceSettingsManager.addLoader([this](const DefaultSurfaceInstanceSettings& settings)
		{
			return loadSurfaceMaterialInstance(settings);
		});

	// Hashers
	m_uberShaderOptionsManager.setHasher(&UberInstanceSettings::hash);

	m_imageManager.setHasher(&ImageSettings::hash);

	m_defaultSurfaceMaterialSettingsManager.setHasher(&DefaultSurfaceMaterialSettings::hash);

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
	m_resourceManagers.emplace_back(&m_defaultSurfaceMaterialSettingsManager);
	m_resourceManagers.emplace_back(&m_defaultSurfaceInstanceSettingsManager);

	m_shaderManager.setDeleter([&](Ptr<Shader> shader)
		{
			const auto it = m_shaderToUber.find(shader.get());

			if (it != m_shaderToUber.end())
				m_shaderToUber.erase(it);

			shader.reset();
		});

	auto deleteUberShader = [&](Ptr<UberShader> uberShader)
	{
		const auto it = m_uberShaders.find(uberShader.get());

		if (it != m_uberShaders.end())
			m_uberShaders.erase(it);

		uberShader.reset();
	};

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
	m_quadMesh.reset();
	
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

void Graphics::setUberShader(const std::string& identifier, const std::string& shaderCode)
{
	AtslParser parser;

	AtslToAstConverter converter;

	auto ast = converter.createAst(parser.createTokens(shaderCode));

	m_uberShaderIdManager.set(identifier, std::make_shared<UberShader>(std::move(ast)));
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

Ptr<UberShader> Graphics::getUberShader(const UberShader& baseUberShader, const std::vector<UberShader::Option>& options)
{
	return m_uberShaderOptionsManager.get({ &baseUberShader, options });
}

Ptr<UberShader> Graphics::getUberShader(const UberShader& baseUberShader, AstShaderStage shaderStage)
{
	return m_uberShaderStageManager.get({ &baseUberShader, shaderStage });
}

Ptr<UberShader> Graphics::getUberShader(const std::filesystem::path& path, AstShaderStage shaderStage, const std::vector<UberShader::Option>& options)
{
	// Get the raw shader
	auto baseUberShader = getUberShader(path);

	// Preprocess the shader
	baseUberShader = getUberShader(*baseUberShader, options);

	// Extract the desired stage
	return getUberShader(*baseUberShader, shaderStage);
}

Ptr<UberShader> Graphics::getUberShaderFromString(const std::string& identifier, AstShaderStage shaderStage, const std::vector<UberShader::Option>& options)
{
	// Get the raw shader
	auto baseUberShader = getUberShader(identifier);

	// Preprocess the shader
	baseUberShader = getUberShader(*baseUberShader, options);

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

Ptr<Image> Graphics::getImage(const std::filesystem::path& path, const ImageLoader::Settings& settings)
{
	return m_imageManager.get({ path, settings });
}

Ptr<Sampler> Graphics::getSampler(const Sampler::Settings& settings)
{
	return m_samplerManager.get(settings);
}

Ptr<SurfaceMaterial> Graphics::getSurfaceMaterial(const SurfaceMaterialData& materialData, uint32_t instanceLayoutPageSize)
{
	return m_defaultSurfaceMaterialSettingsManager.get({ materialData, instanceLayoutPageSize });
}

Ptr<SurfaceMaterialInstance> Graphics::getSurfaceMaterialInstance(const Ptr<SurfaceMaterial>& material, const SurfaceMaterialData& materialData)
{
	return m_defaultSurfaceInstanceSettingsManager.get({ material, materialData });
}

Ptr<UberShader> Graphics::loadUberShader(const std::filesystem::path& path)
{
	auto uberShader = AtslLoader::load(path);

	if (m_uberShaders.find(uberShader.get()) == m_uberShaders.end())
		m_uberShaders[uberShader.get()] = uberShader;

	return uberShader;
}

Ptr<UberShader> Graphics::loadUberInstance(const UberInstanceSettings& settings)
{
	auto uberShader = settings.uberShader->createInstance(settings.options);

	if (m_uberShaders.find(uberShader.get()) == m_uberShaders.end())
		m_uberShaders[uberShader.get()] = uberShader;

	return uberShader;
}

Ptr<UberShader> Graphics::loadUberStage(const UberStageSettings& settings)
{
	auto uberShader = settings.uberShader->extractStage(settings.shaderStage);

	if (m_uberShaders.find(uberShader.get()) == m_uberShaders.end())
		m_uberShaders[uberShader.get()] = uberShader;

	return uberShader;
}

Ptr<Shader> Graphics::loadShader(const UberShader& uberShader)
{
	Shader::Settings settings;
	settings.shaderLanguage = ShaderLanguage::Ast;
	settings.shaderData = uberShader.getAst().get();
	settings.shaderDataSize = 1;

	auto shader = Shader::create(settings);

	if (m_shaderToUber.find(shader.get()) == m_shaderToUber.end())
	{
		const auto it = m_uberShaders.find(&uberShader);

		if (it != m_uberShaders.end())
			m_shaderToUber[shader.get()] = it->second.lock();
	}

	return shader;
}

Ptr<DescriptorSetLayout> Graphics::loadDescriptorSetLayout(const DescriptorSetLayout::Settings& settings)
{
	return DescriptorSetLayout::create(settings);
}

Ptr<GraphicsPipeline::Settings> Graphics::loadGraphicsPipelineSettings(const GraphicsPipelineSettings& settings)
{
	auto pipelineSettings = std::make_shared<GraphicsPipeline::Settings>();

	const auto& vertexReflection = settings.vertexShader->getReflection(AstShaderStage::Vertex);
	const auto& fragmentReflection = settings.fragmentShader->getReflection(AstShaderStage::Fragment);

	// Shaders
	pipelineSettings->vertexShader = getShader(*settings.vertexShader);
	pipelineSettings->fragmentShader = getShader(*settings.fragmentShader);

	// VertexInputState
	for (const auto& input : vertexReflection->getInputs())
		pipelineSettings->state.vertexInput.inputs.emplace_back(0, input.location, getVertexFormat(input.type));

	// DescriptorSetLayouts

	// bindings[setIndex][bindingIndex] = DescriptorSetBinding
	std::map<uint32_t, std::map<uint32_t, DescriptorSetBinding>> bindings;

	for (const auto& external : vertexReflection->getExternals())
	{
		auto& binding = bindings[external.set][external.binding];

		binding.type = getDefaultDescriptorType(external.type);
		binding.count = getDescriptorBindingCount(external.type);
		binding.binding = external.binding;
		binding.shaderStages = ShaderStage::Vertex;
	}

	for (const auto& external : fragmentReflection->getExternals())
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

	// Retrieving DescriptorSetLayouts corresponding to each set index
	// If no set is bound, the layout at this corresponding index is empty
	for (auto& [setIndex, bindingMap] : bindings)
	{
		DescriptorSetLayout::Settings layoutSettings;

		for (auto& [bindingIndex, binding] : bindingMap)
			layoutSettings.bindings.emplace_back(std::move(binding));

		pipelineSettings->descriptorSetLayouts[setIndex] = getDescriptorSetLayout(layoutSettings);
	}

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

Ptr<SurfaceMaterial> Graphics::loadSurfaceMaterial(const DefaultSurfaceMaterialSettings& settings)
{
	return SurfaceMaterial::createDefault(settings.materialData, settings.instanceLayoutPageSize, *this);
}

Ptr<SurfaceMaterialInstance> Graphics::loadSurfaceMaterialInstance(const DefaultSurfaceInstanceSettings& settings)
{
	return SurfaceMaterialInstance::createDefault(settings.material, settings.materialData, *this);
}
