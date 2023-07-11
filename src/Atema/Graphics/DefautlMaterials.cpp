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

#include <optional>
#include <Atema/Core/Utils.hpp>
#include <Atema/Graphics/DefaultMaterials.hpp>
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/MaterialData.hpp>

#define ATEMA_DEFAULT_MATERIAL(atMaterialName, atMaterialShader) const std::string atMaterialName ## Material("Default" #atMaterialName "Material"); \
	constexpr const char atMaterialName ## MaterialShader[] = atMaterialShader;

using namespace at;

namespace
{
	ATEMA_DEFAULT_MATERIAL(Phong, R"(
option
{
	int MaterialBaseColorMapBinding = -1;
	int MaterialNormalMapBinding = -1;
	int MaterialAmbientOcclusionMapBinding = -1;
	int MaterialHeightMapBinding = -1;
	int MaterialEmissiveColorMapBinding = -1;
	int MaterialMetalnessMapBinding = -1;
	int MaterialRoughnessMapBinding = -1;
	int MaterialAlphaMapBinding = -1;
	int MaterialStructBinding = -1;
	int InstanceSetIndex = 2;
}

include Atema.GBufferWrite;

struct FrameDataStruct
{
	mat4f proj;
	mat4f view;
	vec3f cameraPosition;
}

struct TransformDataStruct
{
	mat4f model;
}

struct MaterialDataStruct
{
	[optional (MaterialBaseColorMapBinding < 0)]
	vec4f color;
	
	[optional (MaterialEmissiveColorMapBinding < 0)]
	vec3f emissive;
	
	[optional (MaterialMetalnessMapBinding < 0)]
	float metalness;
	
	[optional (MaterialRoughnessMapBinding < 0)]
	float roughness;

	[optional (MaterialAlphaMapBinding >= 0)]
	float alphaThreshold;
}

external
{
	[set(0), binding(0)] FrameDataStruct FrameData;

	[set(1), binding(0)] TransformDataStruct TransformData;
	
	[optional (MaterialBaseColorMapBinding >= 0)]
	[set(InstanceSetIndex), binding(MaterialBaseColorMapBinding)] sampler2Df BaseColorMap;
	
	[optional (MaterialNormalMapBinding >= 0)]
	[set(InstanceSetIndex), binding(MaterialNormalMapBinding)] sampler2Df NormalMap;
	
	[optional (MaterialAmbientOcclusionMapBinding >= 0)]
	[set(InstanceSetIndex), binding(MaterialAmbientOcclusionMapBinding)] sampler2Df AmbientOcclusionMap;
	
	[optional (MaterialHeightMapBinding >= 0)]
	[set(InstanceSetIndex), binding(MaterialHeightMapBinding)] sampler2Df HeightMap;
	
	[optional (MaterialEmissiveColorMapBinding >= 0)]
	[set(InstanceSetIndex), binding(MaterialEmissiveColorMapBinding)] sampler2Df EmissiveColorMap;
	
	[optional (MaterialMetalnessMapBinding >= 0)]
	[set(InstanceSetIndex), binding(MaterialMetalnessMapBinding)] sampler2Df MetalnessMap;
	
	[optional (MaterialRoughnessMapBinding >= 0)]
	[set(InstanceSetIndex), binding(MaterialRoughnessMapBinding)] sampler2Df RoughnessMap;
	
	[optional (MaterialAlphaMapBinding >= 0)]
	[set(InstanceSetIndex), binding(MaterialAlphaMapBinding)] sampler2Df AlphaMap;
	
	[optional (MaterialBaseColorMapBinding < 0 ||
		MaterialEmissiveColorMapBinding < 0 ||
		MaterialMetalnessMapBinding < 0 ||
		MaterialRoughnessMapBinding < 0 ||
		MaterialAlphaMapBinding >= 0)]
	[set(InstanceSetIndex), binding(MaterialStructBinding)] MaterialDataStruct MaterialData;
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
	vec4f worldPos = TransformData.model * vec4f(inPosition, 1.0);
	vec3f worldNormal = normalize(TransformData.model * vec4f(inNormal, 0.0)).xyz;
	vec3f worldTangent = normalize(TransformData.model * vec4f(inTangent, 0.0)).xyz;
	vec3f worldBitangent = normalize(TransformData.model * vec4f(inBitangent, 0.0)).xyz;
	
	outPosition = worldPos.xyz;
	
	outTBN = mat3f(worldTangent, worldBitangent, worldNormal);
	
	outTexCoords = inTexCoords;
	
	outTanViewDir = outTBN * (FrameData.cameraPosition - worldPos.xyz);
	
	outCameraPosition = FrameData.cameraPosition;
	
	vec4f screenPosition = FrameData.proj * FrameData.view * worldPos;
	
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
	optional (MaterialHeightMapBinding < 0)
		return inTexCoords;
	
	optional (MaterialHeightMapBinding >= 0)
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
		float currentDepthMapValue = 1.0 - sample(HeightMap, UVs).r;
		
		while (currentLayerDepth < currentDepthMapValue)
		{
			UVs = UVs - deltaUVs;
			currentDepthMapValue = 1.0 - sample(HeightMap, UVs).r;
			currentLayerDepth = currentLayerDepth + layerDepth;
		}

		vec2f prevTexCoords = UVs + deltaUVs;
		float currentDepth  = currentDepthMapValue - currentLayerDepth;
		float prevDepth = 1.0 - sample(HeightMap, prevTexCoords).r - currentLayerDepth + layerDepth;
		float weight = currentDepth / (currentDepth - prevDepth);
		UVs = mix(UVs, prevTexCoords, weight);
		
		return UVs;
	}
}

vec4f getMaterialColor()
{
	optional (MaterialBaseColorMapBinding < 0)
		return MaterialData.color;
	
	optional (MaterialBaseColorMapBinding >= 0)
		return sample(BaseColorMap, getTexCoords());
}

vec3f getMaterialNormal()
{
	optional (MaterialNormalMapBinding < 0)
		return vec3f(0.5, 0.5, 1.0);
	
	optional (MaterialNormalMapBinding >= 0)
		return sample(NormalMap, getTexCoords()).xyz;
}

float getMaterialAO()
{
	optional (MaterialAmbientOcclusionMapBinding < 0)
		return 1.0;
	
	optional (MaterialAmbientOcclusionMapBinding >= 0)
		return sample(AmbientOcclusionMap, getTexCoords()).r;
}

float getMaterialHeight()
{
	optional (MaterialHeightMapBinding < 0)
		return 1.0;
	
	optional (MaterialHeightMapBinding >= 0)
		return sample(HeightMap, getTexCoords()).r;
}

vec3f getMaterialEmissive()
{
	optional (MaterialEmissiveColorMapBinding < 0)
		return MaterialData.emissive;
	
	optional (MaterialEmissiveColorMapBinding >= 0)
		return sample(EmissiveColorMap, getTexCoords()).rgb;
}

float getMaterialMetalness()
{
	optional (MaterialMetalnessMapBinding < 0)
		return MaterialData.metalness;
	
	optional (MaterialMetalnessMapBinding >= 0)
		return sample(MetalnessMap, getTexCoords()).r;
}

float getMaterialRoughness()
{
	optional (MaterialRoughnessMapBinding < 0)
		return MaterialData.roughness;
	
	optional (MaterialRoughnessMapBinding >= 0)
		return sample(RoughnessMap, getTexCoords()).r;
}

float getMaterialAlpha()
{
	optional (MaterialAlphaMapBinding < 0)
		return 1.0;
	
	optional (MaterialAlphaMapBinding >= 0)
		return sample(AlphaMap, getTexCoords()).r;
}

[entry(fragment)]
void main()
{
	optional (MaterialAlphaMapBinding >= 0)
	{
		if (getMaterialAlpha() < MaterialData.alphaThreshold)
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
)");

	struct PhongParameter
	{
		std::string name;
		const char* textureName = nullptr;
		const char* constantName = nullptr;
		std::optional<ConstantValue> defautValue;
	};

	struct DefaultPhongData : public ShaderData
	{
		static constexpr int32_t InvalidBinding = -1;

		struct TextureBindings
		{
			int32_t BaseColorMap = InvalidBinding;
			int32_t NormalMap = InvalidBinding;
			int32_t AmbientOcclusionMap = InvalidBinding;
			int32_t HeightMap = InvalidBinding;
			int32_t EmissiveColorMap = InvalidBinding;
			int32_t MetalnessMap = InvalidBinding;
			int32_t RoughnessMap = InvalidBinding;
			int32_t AlphaMap = InvalidBinding;
		};

		struct Offsets
		{
			static constexpr size_t InvalidOffset = std::numeric_limits<size_t>::max();

			size_t baseColor = InvalidOffset;
			size_t emissiveColor = InvalidOffset;
			size_t metalness = InvalidOffset;
			size_t roughness = InvalidOffset;
			size_t alphaThreshold = InvalidOffset;
		};

		std::filesystem::path BaseColorMapPath;
		std::filesystem::path NormalMapPath;
		std::filesystem::path AmbientOcclusionMapPath;
		std::filesystem::path HeightMapPath;
		std::filesystem::path EmissiveColorMapPath;
		std::filesystem::path MetalnessMapPath;
		std::filesystem::path RoughnessMapPath;
		std::filesystem::path AlphaMapPath;

		Color BaseColor = Color::White;
		Color EmissiveColor = Color::Black;
		float Metalness = 0.0f;
		float Roughness = 0.0f;
		float AlphaThreshold = 0.5f;

		int32_t structBinding = InvalidBinding;
		TextureBindings textureBindings;
		Offsets offsets;
		size_t byteSize = 0;
		bool useUniformBuffer = false;

		DefaultPhongData() = delete;
		DefaultPhongData(const MaterialData& MaterialData, StructLayout structLayout = StructLayout::Default)
		{
			auto& graphics = Graphics::instance();

			int32_t bindingIndex = 0;

			// Load textures
#define ATEMA_PHONG_CHECK_TEXTURE(atTextureName) \
	if (MaterialData.exists(MaterialData::atTextureName)) \
	{ \
		auto& parameter = MaterialData.getValue(MaterialData::atTextureName); \
		if (parameter.is<MaterialData::Texture>() && !parameter.get<MaterialData::Texture>().path.empty()) \
		{ \
			atTextureName ## Path = parameter.get<MaterialData::Texture>().path; \
			textureBindings.atTextureName = bindingIndex++; \
		} \
	}

			ATEMA_PHONG_CHECK_TEXTURE(BaseColorMap)
			ATEMA_PHONG_CHECK_TEXTURE(NormalMap)
			ATEMA_PHONG_CHECK_TEXTURE(AmbientOcclusionMap)
			ATEMA_PHONG_CHECK_TEXTURE(HeightMap)
			ATEMA_PHONG_CHECK_TEXTURE(EmissiveColorMap)
			ATEMA_PHONG_CHECK_TEXTURE(MetalnessMap)
			ATEMA_PHONG_CHECK_TEXTURE(RoughnessMap)
			ATEMA_PHONG_CHECK_TEXTURE(AlphaMap)

#undef ATEMA_PHONG_CHECK_TEXTURE

#define ATEMA_PHONG_GET_PARAMETER(atType, atName) \
	if (MaterialData.exists(MaterialData::atName)) \
	{ \
		auto& parameter = MaterialData.getValue(MaterialData::atName); \
		if (parameter.is<atType>()) \
			atName = parameter.get<atType>(); \
	}

			ATEMA_PHONG_GET_PARAMETER(Color, BaseColor)
			ATEMA_PHONG_GET_PARAMETER(Color, EmissiveColor)
			ATEMA_PHONG_GET_PARAMETER(float, Metalness)
			ATEMA_PHONG_GET_PARAMETER(float, Roughness)
			ATEMA_PHONG_GET_PARAMETER(float, AlphaThreshold)

#undef ATEMA_PHONG_GET_PARAMETER

			// Build buffer layout

			BufferLayout layout(structLayout);

			if (BaseColorMapPath.empty())
				offsets.baseColor = layout.add(BufferElementType::Float4);

			if (EmissiveColorMapPath.empty())
				offsets.emissiveColor = layout.add(BufferElementType::Float3);

			if (MetalnessMapPath.empty())
				offsets.metalness = layout.add(BufferElementType::Float);

			if (RoughnessMapPath.empty())
				offsets.roughness = layout.add(BufferElementType::Float);

			if (!AlphaMapPath.empty())
				offsets.alphaThreshold = layout.add(BufferElementType::Float);

			byteSize = layout.getSize();

			if (byteSize > 0)
			{
				structBinding = bindingIndex;
				useUniformBuffer = true;
			}
		}

		size_t getByteSize(StructLayout structLayout) const noexcept override
		{
			return byteSize;
		}

		void copyTo(void* dstData, StructLayout structLayout) const override
		{
			if (offsets.baseColor != Offsets::InvalidOffset)
				mapMemory<Vector4f>(dstData, offsets.baseColor) = BaseColor.toVector4f();

			if (offsets.emissiveColor != Offsets::InvalidOffset)
				mapMemory<Vector3f>(dstData, offsets.emissiveColor) = EmissiveColor.toVector3f();

			if (offsets.metalness != Offsets::InvalidOffset)
				mapMemory<float>(dstData, offsets.metalness) = Metalness;

			if (offsets.roughness != Offsets::InvalidOffset)
				mapMemory<float>(dstData, offsets.roughness) = Roughness;

			if (offsets.alphaThreshold != Offsets::InvalidOffset)
				mapMemory<float>(dstData, offsets.alphaThreshold) = AlphaThreshold;
		}
	};
}

Ptr<Material> DefaultMaterials::getPhong(const MaterialData& materialData)
{
	auto& graphics = Graphics::instance();

	// Ensure the default shader is loaded
	if (!graphics.uberShaderExists(PhongMaterial))
		graphics.setUberShader(PhongMaterial, PhongMaterialShader);

	// Build shader data
	const DefaultPhongData phongData(materialData);

	// Define shader options
	const std::vector<UberShader::Option> shaderOptions =
	{
		{ "MaterialBaseColorMapBinding", phongData.textureBindings.BaseColorMap },
		{ "MaterialNormalMapBinding", phongData.textureBindings.NormalMap },
		{ "MaterialAmbientOcclusionMapBinding", phongData.textureBindings.AmbientOcclusionMap },
		{ "MaterialHeightMapBinding", phongData.textureBindings.HeightMap },
		{ "MaterialEmissiveColorMapBinding", phongData.textureBindings.EmissiveColorMap },
		{ "MaterialMetalnessMapBinding", phongData.textureBindings.MetalnessMap },
		{ "MaterialRoughnessMapBinding", phongData.textureBindings.RoughnessMap },
		{ "MaterialAlphaMapBinding", phongData.textureBindings.AlphaMap },
		{ "MaterialStructBinding", phongData.structBinding }
	};

	// Get the raw shader
	auto uberShader = graphics.getUberShader(PhongMaterial);

	// Preprocess the shader with the options we just defined
	uberShader = graphics.getUberShader(*uberShader, shaderOptions);

	return std::make_shared<Material>(uberShader);
}

Ptr<MaterialInstance> DefaultMaterials::getPhongInstance(const MaterialData& materialData)
{
	auto& graphics = Graphics::instance();

	auto material = getPhong(materialData);

	auto materialInstance = std::make_shared<MaterialInstance>(material);

	// Default texture sampler
	const Sampler::Settings samplerSettings(SamplerFilter::Linear, true);
	const auto sampler = graphics.getSampler(samplerSettings);

	const DefaultPhongData phongData(materialData);

#define ATEMA_PHONG_SET_TEXTURE(atTextureName) \
	if (!phongData.atTextureName ## Path.empty()) \
	{ \
		const auto image = graphics.getImage(phongData.atTextureName ## Path); \
		materialInstance->setParameter(#atTextureName, image, sampler); \
	}

	ATEMA_PHONG_SET_TEXTURE(BaseColorMap)
	ATEMA_PHONG_SET_TEXTURE(NormalMap)
	ATEMA_PHONG_SET_TEXTURE(AmbientOcclusionMap)
	ATEMA_PHONG_SET_TEXTURE(HeightMap)
	ATEMA_PHONG_SET_TEXTURE(EmissiveColorMap)
	ATEMA_PHONG_SET_TEXTURE(MetalnessMap)
	ATEMA_PHONG_SET_TEXTURE(RoughnessMap)
	ATEMA_PHONG_SET_TEXTURE(AlphaMap)

#undef ATEMA_PHONG_SET_TEXTURE

	if (phongData.useUniformBuffer)
		materialInstance->setParameter("MaterialData", phongData);

	return materialInstance;
}
