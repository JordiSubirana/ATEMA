/*
	Copyright 2023 Jordi SUBIRANA

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

#include <Atema/Graphics/SurfaceMaterial.hpp>
#include <Atema/Core/Utils.hpp>
#include <Atema/Renderer/Buffer.hpp>
#include <Atema/Renderer/BufferLayout.hpp>
#include <Atema/Renderer/CommandBuffer.hpp>
#include <Atema/Renderer/DescriptorSetLayout.hpp>
#include <Atema/Shader/Ast/Constant.hpp>

#include <optional>

#define ATEMA_DEFAULT_SURFACE_SHADER_ID "AtemaDefaultSurfaceShader"

using namespace at;

namespace
{
	const char atDefaultSurfaceShader[] = R"(
option
{
	int MaterialColorBinding = -1;
	int MaterialNormalBinding = -1;
	int MaterialAOBinding = -1;
	int MaterialHeightBinding = -1;
	int MaterialEmissiveBinding = -1;
	int MaterialMetalnessBinding = -1;
	int MaterialRoughnessBinding = -1;
	int MaterialAlphaMaskBinding = -1;
	int InstanceSetIndex = 3;
}

include Atema.GBufferWrite;

struct FrameData
{
	mat4f proj;
	mat4f view;
	vec3f cameraPosition;
}

struct ObjectData
{
	mat4f model;
}

struct MaterialData
{
	[optional (MaterialColorBinding < 0)]
	vec4f color;
	
	[optional (MaterialEmissiveBinding < 0)]
	vec3f emissive;
	
	[optional (MaterialMetalnessBinding < 0)]
	float metalness;
	
	[optional (MaterialRoughnessBinding < 0)]
	float roughness;
}

external
{
	[set(0), binding(0)] FrameData frameData;
	[set(1), binding(0)] ObjectData objectData;
	
	[optional (MaterialColorBinding < 0 ||
		MaterialEmissiveBinding < 0 ||
		MaterialMetalnessBinding < 0 ||
		MaterialRoughnessBinding < 0)]
	[set(InstanceSetIndex), binding(0)] MaterialData materialData;
	
	[optional (MaterialColorBinding >= 0)]
	[set(InstanceSetIndex), binding(MaterialColorBinding)] sampler2Df materialColor;
	
	[optional (MaterialNormalBinding >= 0)]
	[set(InstanceSetIndex), binding(MaterialNormalBinding)] sampler2Df materialNormal;
	
	[optional (MaterialAOBinding >= 0)]
	[set(InstanceSetIndex), binding(MaterialAOBinding)] sampler2Df materialAO;
	
	[optional (MaterialHeightBinding >= 0)]
	[set(InstanceSetIndex), binding(MaterialHeightBinding)] sampler2Df materialHeight;
	
	[optional (MaterialEmissiveBinding >= 0)]
	[set(InstanceSetIndex), binding(MaterialEmissiveBinding)] sampler2Df materialEmissive;
	
	[optional (MaterialMetalnessBinding >= 0)]
	[set(InstanceSetIndex), binding(MaterialMetalnessBinding)] sampler2Df materialMetalness;
	
	[optional (MaterialRoughnessBinding >= 0)]
	[set(InstanceSetIndex), binding(MaterialRoughnessBinding)] sampler2Df materialRoughness;
	
	[optional (MaterialAlphaMaskBinding >= 0)]
	[set(InstanceSetIndex), binding(MaterialAlphaMaskBinding)] sampler2Df materialAlphaMask;
}

[stage(vertex)]
input
{
	[location(0)] vec3f inPosition;
	[location(1)] vec2f inTexCoords;
	[location(2)] vec3f inNormal;
	[location(3)] vec3f inTangent;
	[location(4)] vec3f inBitangent;
}

[stage(vertex)]
output
{
	[location(0)] vec3f outPosition;
	[location(1)] vec2f outTexCoords;
	[location(2)] mat3f outTBN;
	[location(5)] vec3f outTanViewDir;
	[location(6)] vec3f outCameraPosition;
}

[entry(vertex)]
void main()
{
	vec4f worldPos = objectData.model * vec4f(inPosition, 1.0);
	vec3f worldNormal = normalize(objectData.model * vec4f(inNormal, 0.0)).xyz;
	vec3f worldTangent = normalize(objectData.model * vec4f(inTangent, 0.0)).xyz;
	vec3f worldBitangent = normalize(objectData.model * vec4f(inBitangent, 0.0)).xyz;
	
	outPosition = worldPos.xyz;
	
	outTBN = mat3f(worldTangent, worldBitangent, worldNormal);
	
	outTexCoords = inTexCoords;
	
	outTanViewDir = outTBN * (frameData.cameraPosition - worldPos.xyz);
	
	outCameraPosition = frameData.cameraPosition;
	
	vec4f screenPosition = frameData.proj * frameData.view * worldPos;
	
	setVertexPosition(screenPosition);
}

[stage(fragment)]
input
{
	[location(0)] vec3f inPosition;
	[location(1)] vec2f inTexCoords;
	[location(2)] mat3f inTBN;
	[location(5)] vec3f inTanViewDir;
	[location(6)] vec3f inCameraPosition;
}

vec2f getTexCoords()
{ 
	optional (MaterialHeightBinding < 0)
		return inTexCoords;
	
	optional (MaterialHeightBinding >= 0)
	{
		float heightScale = 0.05;
		float minLayers = 8.0;
		float maxLayers = 64.0;
		float minDistance = 40.0;
		float maxDistance = 80.0;
		
		vec3f viewDir = normalize(inTanViewDir);
		
		float distanceFactor = clamp(distance(inCameraPosition, inPosition), minDistance, maxDistance + minDistance) - minDistance;
		distanceFactor = 1.0 - distanceFactor / maxDistance;
		
		minLayers = mix(1.0, minLayers, distanceFactor);
		maxLayers = mix(1.0, maxLayers, distanceFactor);
		float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
		float layerDepth = 1.0 / numLayers;
		float currentLayerDepth = 0.0;
		
		vec2f S = viewDir.xy / viewDir.z * heightScale; 
		vec2f deltaUVs = S / numLayers;
		
		vec2f UVs = inTexCoords;
		float currentDepthMapValue = 1.0 - sample(materialHeight, UVs).r;
		
		while (currentLayerDepth < currentDepthMapValue)
		{
			UVs = UVs - deltaUVs;
			currentDepthMapValue = 1.0 - sample(materialHeight, UVs).r;
			currentLayerDepth = currentLayerDepth + layerDepth;
		}

		vec2f prevTexCoords = UVs + deltaUVs;
		float currentDepth  = currentDepthMapValue - currentLayerDepth;
		float prevDepth = 1.0 - sample(materialHeight, prevTexCoords).r - currentLayerDepth + layerDepth;
		float weight = currentDepth / (currentDepth - prevDepth);
		UVs = mix(UVs, prevTexCoords, weight);
		
		return UVs;
	}
}

vec4f getMaterialColor()
{
	optional (MaterialColorBinding < 0)
		return materialData.color;
	
	optional (MaterialColorBinding >= 0)
		return sample(materialColor, getTexCoords());
}

vec3f getMaterialNormal()
{
	optional (MaterialNormalBinding < 0)
		return vec3f(0.5, 0.5, 1.0);
	
	optional (MaterialNormalBinding >= 0)
		return sample(materialNormal, getTexCoords()).xyz;
}

float getMaterialAO()
{
	optional (MaterialAOBinding < 0)
		return 1.0;
	
	optional (MaterialAOBinding >= 0)
		return sample(materialAO, getTexCoords()).r;
}

float getMaterialHeight()
{
	optional (MaterialHeightBinding < 0)
		return 1.0;
	
	optional (MaterialHeightBinding >= 0)
		return sample(materialHeight, getTexCoords()).r;
}

vec3f getMaterialEmissive()
{
	optional (MaterialEmissiveBinding < 0)
		return materialData.emissive;
	
	optional (MaterialEmissiveBinding >= 0)
		return sample(materialEmissive, getTexCoords()).rgb;
}

float getMaterialMetalness()
{
	optional (MaterialMetalnessBinding < 0)
		return materialData.metalness;
	
	optional (MaterialMetalnessBinding >= 0)
		return sample(materialMetalness, getTexCoords()).r;
}

float getMaterialRoughness()
{
	optional (MaterialRoughnessBinding < 0)
		return materialData.roughness;
	
	optional (MaterialRoughnessBinding >= 0)
		return sample(materialRoughness, getTexCoords()).r;
}

float getMaterialAlphaMask()
{
	optional (MaterialAlphaMaskBinding < 0)
		return 1.0;
	
	optional (MaterialAlphaMaskBinding >= 0)
		return sample(materialAlphaMask, getTexCoords()).r;
}

[entry(fragment)]
void main()
{
	optional (MaterialAlphaMaskBinding >= 0)
	{
		if (getMaterialAlphaMask() < 0.5)
			discard;
	}
	
	vec4f matColor = getMaterialColor();
	vec3f matNormal = getMaterialNormal();
	float matAO = getMaterialAO();
	vec3f matEmissive = getMaterialEmissive();
	float matMetalness = getMaterialMetalness();
	float matRoughness = getMaterialRoughness();
	
	vec3f normal = (matNormal * 2.0) - 1.0;
	normal = normalize(inTBN * normal);
	
	atGBufferWritePosition(inPosition);
	atGBufferWriteNormal(normal);
	atGBufferWriteAlbedo(matColor.rgb);
	atGBufferWriteAO(matAO);
	atGBufferWriteEmissive(matEmissive);
	atGBufferWriteMetalness(matMetalness);
	atGBufferWriteRoughness(matRoughness);
}
)";

	IdManager<SurfaceMaterial::ID> s_surfaceIdManager;
}

namespace
{
	struct SurfaceMaterialParameter
	{
		std::string name;
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

	void writeBufferElement(void* ptr, size_t byteOffset, const ConstantValue& constantValue)
	{
		if (constantValue.is<bool>())
			mapMemory<bool>(ptr, byteOffset) = constantValue.get<bool>();
		else if (constantValue.is<int32_t>())
			mapMemory<int32_t>(ptr, byteOffset) = constantValue.get<int32_t>();
		else if (constantValue.is<uint32_t>())
			mapMemory<uint32_t>(ptr, byteOffset) = constantValue.get<uint32_t>();
		else if (constantValue.is<float>())
			mapMemory<float>(ptr, byteOffset) = constantValue.get<float>();
		else if (constantValue.is<Vector2i>())
			mapMemory<Vector2i>(ptr, byteOffset) = constantValue.get<Vector2i>();
		else if (constantValue.is<Vector2u>())
			mapMemory<Vector2u>(ptr, byteOffset) = constantValue.get<Vector2u>();
		else if (constantValue.is<Vector2f>())
			mapMemory<Vector2f>(ptr, byteOffset) = constantValue.get<Vector2f>();
		else if (constantValue.is<Vector3i>())
			mapMemory<Vector3i>(ptr, byteOffset) = constantValue.get<Vector3i>();
		else if (constantValue.is<Vector3u>())
			mapMemory<Vector3u>(ptr, byteOffset) = constantValue.get<Vector3u>();
		else if (constantValue.is<Vector3f>())
			mapMemory<Vector3f>(ptr, byteOffset) = constantValue.get<Vector3f>();
		else if (constantValue.is<Vector4i>())
			mapMemory<Vector4i>(ptr, byteOffset) = constantValue.get<Vector4i>();
		else if (constantValue.is<Vector4u>())
			mapMemory<Vector4u>(ptr, byteOffset) = constantValue.get<Vector4u>();
		else if (constantValue.is<Vector4f>())
			mapMemory<Vector4f>(ptr, byteOffset) = constantValue.get<Vector4f>();
		else
			ATEMA_ERROR("Invalid type");
	}
}

//-----
// SurfaceMaterialData
SurfaceMaterialData::SurfaceMaterialData()
{

}

SurfaceMaterialData::SurfaceMaterialData(SurfaceMaterialData&& other) noexcept :
	colorMap(std::move(other.colorMap)),
	normalMap(std::move(other.normalMap)),
	ambientOcclusionMap(std::move(other.ambientOcclusionMap)),
	heightMap(std::move(other.heightMap)),
	emissiveMap(std::move(other.emissiveMap)),
	metalnessMap(std::move(other.metalnessMap)),
	roughnessMap(std::move(other.roughnessMap)),
	alphaMaskMap(std::move(other.alphaMaskMap)),
	color(std::move(other.color)),
	emissive(std::move(other.emissive)),
	metalness(other.metalness),
	roughness(other.roughness)
{

}

SurfaceMaterialData::~SurfaceMaterialData()
{
}

//-----
// SurfaceMaterial
SurfaceMaterial::SurfaceMaterial(const SurfaceMaterial::Settings& settings) :
	m_layout(settings.layout),
	m_instanceLayout(settings.instanceLayout),
	m_id(s_surfaceIdManager.get())
{
	Ptr<DescriptorSetLayout> frameLayout;
	{
		DescriptorSetLayout::Settings layoutSettings;
		layoutSettings.bindings =
		{
			{ DescriptorType::UniformBuffer, 0, 1, ShaderStage::Vertex }
		};
		layoutSettings.pageSize = 1;

		frameLayout = settings.graphics.getDescriptorSetLayout(layoutSettings);
	}

	Ptr<DescriptorSetLayout> objectLayout;
	{
		DescriptorSetLayout::Settings layoutSettings;
		layoutSettings.bindings =
		{
			{ DescriptorType::UniformBufferDynamic, 0, 1, ShaderStage::Vertex }
		};
		layoutSettings.pageSize = 1;

		objectLayout = settings.graphics.getDescriptorSetLayout(layoutSettings);
	}

	// If a material layout is used, allocate a new set (and let the user fill it)
	if (m_layout)
	{
		m_descriptorSet = m_layout->createSet();
	}
	// If no layout is used, create a empty one to match with different set indices
	else
	{
		DescriptorSetLayout::Settings layoutSettings;
		layoutSettings.bindings = {};
		layoutSettings.pageSize = 1;

		m_layout = settings.graphics.getDescriptorSetLayout(layoutSettings);
	}

	GraphicsPipeline::Settings pipelineSettings;
	pipelineSettings.state.vertexInput.inputs = settings.vertexInput;
	pipelineSettings.vertexShader = settings.vertexShader;
	pipelineSettings.fragmentShader = settings.fragmentShader;
	pipelineSettings.descriptorSetLayouts =
	{
		frameLayout,
		objectLayout,
		m_layout,
		m_instanceLayout
	};

	pipelineSettings.state.stencil = true;
	pipelineSettings.state.stencilFront.compareOperation = CompareOperation::Equal;
	pipelineSettings.state.stencilFront.writeMask = 1;
	pipelineSettings.state.stencilFront.passOperation = StencilOperation::Replace;
	pipelineSettings.state.stencilFront.reference = 1;

	m_graphicsPipeline = GraphicsPipeline::create(pipelineSettings);
}

SurfaceMaterial::~SurfaceMaterial()
{
	m_descriptorSet.reset();
	m_layout.reset();
	m_instanceLayout.reset();
	m_graphicsPipeline.reset();

	s_surfaceIdManager.release(m_id);
}

SurfaceMaterial::ID SurfaceMaterial::getID() const noexcept
{
	return m_id;
}

Ptr<SurfaceMaterial> SurfaceMaterial::createDefault(const SurfaceMaterialData& materialData, uint32_t instanceLayoutPageSize, Graphics& graphics)
{
	Settings settings;

	// Default vertex input for built-in surface shader
	settings.vertexInput =
	{
		{ 0, 0, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 1, VertexInputFormat::RG32_SFLOAT },
		{ 0, 2, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 3, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 4, VertexInputFormat::RGB32_SFLOAT }
	};

	std::vector<UberShader::Option> shaderOptions;

	// No global descriptor set layout
	
	// Get instance descriptor set layout
	{
		const std::array<SurfaceMaterialParameter, 8> materialParameters =
		{ {
			{ "Color", materialData.colorMap, ConstantValue(materialData.color) },
			{ "Normal", materialData.normalMap, {} },
			{ "AO", materialData.ambientOcclusionMap, {} },
			{ "Height", materialData.heightMap, {} },
			{ "Emissive", materialData.emissiveMap, ConstantValue(materialData.emissive) },
			{ "Metalness", materialData.metalnessMap, ConstantValue(materialData.metalness) },
			{ "Roughness", materialData.roughnessMap, ConstantValue(materialData.roughness) },
			{ "AlphaMask", materialData.alphaMaskMap, {} }
		} };

		DescriptorSetLayout::Settings layoutSettings;

		// 0 means default page size value (see DescriptorSetLayout::Settings)
		if (instanceLayoutPageSize != 0)
			layoutSettings.pageSize = instanceLayoutPageSize;

		bool useUniformBuffer = false;
		for (const auto& materialParameter : materialParameters)
		{
			if (!materialParameter.image && materialParameter.value.has_value())
				useUniformBuffer = true;
		}

		//TODO: Do we really want the parameters to be accessed in the vertex shader ?
		const auto shaderStages = ShaderStage::Vertex | ShaderStage::Fragment;

		uint32_t bindingIndex = 0;

		// Check if we have some elements in the buffer
		if (useUniformBuffer)
			layoutSettings.bindings.emplace_back(DescriptorType::UniformBuffer, bindingIndex++, 1, shaderStages);

		for (const auto& materialParameter : materialParameters)
		{
			if (materialParameter.image)
			{
				shaderOptions.emplace_back("Material" + materialParameter.name + "Binding", bindingIndex);

				layoutSettings.bindings.emplace_back(DescriptorType::CombinedImageSampler, bindingIndex++, 1, shaderStages);
			}
		}

		settings.instanceLayout = graphics.getDescriptorSetLayout(layoutSettings);
	}

	// Load shaders
	{
		// Ensure the default shader is loaded
		if (!graphics.uberShaderExists(ATEMA_DEFAULT_SURFACE_SHADER_ID))
			graphics.setUberShader(ATEMA_DEFAULT_SURFACE_SHADER_ID, atDefaultSurfaceShader);

		// Get the raw shader
		auto uberShader = graphics.getUberShader(std::string(ATEMA_DEFAULT_SURFACE_SHADER_ID));

		// Preprocess the shader with the options we just defined
		uberShader = graphics.getUberShader(*uberShader, shaderOptions);

		// Extract stages
		settings.vertexShader = graphics.getShader(*graphics.getUberShader(*uberShader, AstShaderStage::Vertex));
		settings.fragmentShader = graphics.getShader(*graphics.getUberShader(*uberShader, AstShaderStage::Fragment));
	}
	
	return std::make_shared<SurfaceMaterial>(settings);
}

void SurfaceMaterial::bindTo(CommandBuffer& commandBuffer)
{
	commandBuffer.bindPipeline(m_graphicsPipeline);
	
	if (m_descriptorSet)
		commandBuffer.bindDescriptorSet(SurfaceMaterial::MaterialSetIndex, m_descriptorSet);
}

const Ptr<GraphicsPipeline>& SurfaceMaterial::getGraphicsPipeline() const noexcept
{
	return m_graphicsPipeline;
}

const Ptr<DescriptorSetLayout>& SurfaceMaterial::getLayout() const noexcept
{
	return m_layout;
}

const Ptr<DescriptorSetLayout>& SurfaceMaterial::getInstanceLayout() const noexcept
{
	return m_instanceLayout;
}

const Ptr<DescriptorSet>& SurfaceMaterial::getDescriptorSet() const noexcept
{
	return m_descriptorSet;
}

MaterialParameters& SurfaceMaterial::getParameters() noexcept
{
	return m_parameters;
}

const MaterialParameters& SurfaceMaterial::getParameters() const noexcept
{
	return m_parameters;
}

SurfaceMaterial::ID SurfaceMaterial::getInstanceId()
{
	return m_instanceIdManager.get();
}

void SurfaceMaterial::releaseInstanceId(SurfaceMaterial::ID id)
{
	m_instanceIdManager.release(id);
}

//-----
// SurfaceMaterialInstance
SurfaceMaterialInstance::SurfaceMaterialInstance(const Ptr<SurfaceMaterial>& material) :
	m_material(material)
{
	ATEMA_ASSERT(material, "Invalid SurfaceMaterial");
	
	if (material->getInstanceLayout())
		m_descriptorSet = material->getInstanceLayout()->createSet();

	m_id = material->getInstanceId();
}

SurfaceMaterialInstance::~SurfaceMaterialInstance()
{
	m_material->releaseInstanceId(m_id);
}

SurfaceMaterial::ID SurfaceMaterialInstance::getID() const noexcept
{
	return m_id;
}

Ptr<SurfaceMaterialInstance> SurfaceMaterialInstance::createDefault(const Ptr<SurfaceMaterial>& material, const SurfaceMaterialData& materialData, Graphics& graphics)
{
	const std::array<SurfaceMaterialParameter, 8> materialParameters =
	{ {
		{ "Color", materialData.colorMap, ConstantValue(materialData.color) },
		{ "Normal", materialData.normalMap, {} },
		{ "AO", materialData.ambientOcclusionMap, {} },
		{ "Height", materialData.heightMap, {} },
		{ "Emissive", materialData.emissiveMap, ConstantValue(materialData.emissive) },
		{ "Metalness", materialData.metalnessMap, ConstantValue(materialData.metalness) },
		{ "Roughness", materialData.roughnessMap, ConstantValue(materialData.roughness) },
		{ "AlphaMask", materialData.alphaMaskMap, {} }
	} };
	
	auto materialInstance = std::make_shared<SurfaceMaterialInstance>(material);

	auto& descriptorSet = materialInstance->m_descriptorSet;
	auto& parameters = materialInstance->m_parameters;

	BufferLayout bufferLayout(StructLayout::Default);
	std::vector<size_t> bufferOffsets;

	size_t textureCount = 0;
	for (auto& materialParameter : materialParameters)
	{
		if (materialParameter.image)
		{
			textureCount++;
		}
		else if (materialParameter.value.has_value())
		{
			const auto elementOffset = bufferLayout.add(getElementType(materialParameter.value.value()));

			bufferOffsets.emplace_back(elementOffset);
		}
	}

	uint32_t bindingIndex = 0;

	// Check if we have some elements in the buffer, if so create it
	Ptr<Buffer> uniformBuffer;
	void* bufferData = nullptr;

	if (bufferLayout.getSize() > 0)
	{
		Buffer::Settings bufferSettings;
		bufferSettings.usages = BufferUsage::Uniform | BufferUsage::Map;
		bufferSettings.byteSize = bufferLayout.getSize();

		uniformBuffer = Buffer::create(bufferSettings);

		descriptorSet->update(bindingIndex++, uniformBuffer);

		bufferData = uniformBuffer->map();

		parameters.set("MaterialData", uniformBuffer);
	}

	// Create sampler
	Sampler::Settings samplerSettings(SamplerFilter::Linear, true);

	const auto sampler = graphics.getSampler(samplerSettings);

	// Fill uniform buffer if needed & update descriptor set
	size_t bufferElementIndex = 0;

	for (auto& materialParameter : materialParameters)
	{
		if (materialParameter.image)
		{
			descriptorSet->update(bindingIndex++, materialParameter.image->getView(), sampler);

			parameters.set(materialParameter.name, materialParameter.image, sampler);
		}
		else if (materialParameter.value.has_value())
		{
			writeBufferElement(bufferData, bufferOffsets[bufferElementIndex++], materialParameter.value.value());
		}
	}

	if (uniformBuffer)
		uniformBuffer->unmap();

	materialInstance->m_descriptorSet = std::move(descriptorSet);
	
	return materialInstance;
}

void SurfaceMaterialInstance::bindTo(CommandBuffer& commandBuffer)
{
	if (m_descriptorSet)
		commandBuffer.bindDescriptorSet(SurfaceMaterial::InstanceSetIndex, m_descriptorSet);
}

const Ptr<SurfaceMaterial>& SurfaceMaterialInstance::getMaterial() const noexcept
{
	return m_material;
}

const Ptr<DescriptorSet>& SurfaceMaterialInstance::getDescriptorSet() const noexcept
{
	return m_descriptorSet;
}

MaterialParameters& SurfaceMaterialInstance::getParameters() noexcept
{
	return m_parameters;
}

const MaterialParameters& SurfaceMaterialInstance::getParameters() const noexcept
{
	return m_parameters;
}
