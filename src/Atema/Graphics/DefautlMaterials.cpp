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

#include <Atema/Core/Utils.hpp>
#include <Atema/Graphics/DefaultMaterials.hpp>
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/MaterialData.hpp>

#include <optional>

#define ATEMA_DEFAULT_MATERIAL(atName, atShader) const std::string atName ## Material("Default" #atName "Material"); \
	constexpr const char atName ## MaterialShader[] = atShader;

using namespace at;

namespace
{
	ATEMA_DEFAULT_MATERIAL(Emissive, R"(
option
{
	int InstanceSetIndex = 2;
}

include Atema.LightingModel.Emissive;

struct MaterialDataStruct
{
	vec4f EmissiveColor;
}

external
{
	[set(InstanceSetIndex), binding(0)] MaterialDataStruct MaterialData;
}

vec3f getEmissiveColor()
{
	return MaterialData.EmissiveColor.rgb;
}

MaterialFragmentParameters getMaterialFragmentParameters()
{
	MaterialFragmentParameters parameters = getDefaultMaterialFragmentParameters();
	
	parameters.EmissiveColor = getEmissiveColor();
	
	return parameters;
}
)");

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
	int MaterialAlphaMaskMapBinding = -1;
	int MaterialStructBinding = -1;
	int InstanceSetIndex = 2;
}

include Atema.LightingModel.Phong;

struct MaterialDataStruct
{
	[optional (MaterialBaseColorMapBinding < 0)]
	vec4f BaseColor;
	
	[optional (MaterialEmissiveColorMapBinding < 0)]
	vec3f EmissiveColor;
	
	[optional (MaterialMetalnessMapBinding < 0)]
	float Metalness;
	
	[optional (MaterialRoughnessMapBinding < 0)]
	float Roughness;

	[optional (MaterialAlphaMaskMapBinding >= 0)]
	float AlphaMaskThreshold;
}

external
{
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
	
	[optional (MaterialAlphaMaskMapBinding >= 0)]
	[set(InstanceSetIndex), binding(MaterialAlphaMaskMapBinding)] sampler2Df AlphaMaskMap;
	
	[optional (MaterialBaseColorMapBinding < 0 ||
		MaterialEmissiveColorMapBinding < 0 ||
		MaterialMetalnessMapBinding < 0 ||
		MaterialRoughnessMapBinding < 0 ||
		MaterialAlphaMaskMapBinding >= 0)]
	[set(InstanceSetIndex), binding(MaterialStructBinding)] MaterialDataStruct MaterialData;
}

vec2f getUV()
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

vec3f getBaseColor(vec2f uv)
{
	optional (MaterialBaseColorMapBinding < 0)
		return MaterialData.BaseColor.rgb;
	
	optional (MaterialBaseColorMapBinding >= 0)
		return sample(BaseColorMap, uv).rgb;
}

vec3f getNormal(vec2f uv)
{
	optional (MaterialNormalMapBinding < 0)
		return vec3f(0.5, 0.5, 1.0);
	
	optional (MaterialNormalMapBinding >= 0)
		return sample(NormalMap, uv).xyz;
}

float getAmbientOcclusion(vec2f uv)
{
	optional (MaterialAmbientOcclusionMapBinding < 0)
		return 1.0;
	
	optional (MaterialAmbientOcclusionMapBinding >= 0)
		return sample(AmbientOcclusionMap, uv).r;
}

float getHeight(vec2f uv)
{
	optional (MaterialHeightMapBinding < 0)
		return 1.0;
	
	optional (MaterialHeightMapBinding >= 0)
		return sample(HeightMap, uv).r;
}

vec3f getEmissiveColor(vec2f uv)
{
	optional (MaterialEmissiveColorMapBinding < 0)
		return MaterialData.EmissiveColor;
	
	optional (MaterialEmissiveColorMapBinding >= 0)
		return sample(EmissiveColorMap, uv).rgb;
}

float getMetalness(vec2f uv)
{
	optional (MaterialMetalnessMapBinding < 0)
		return MaterialData.Metalness;
	
	optional (MaterialMetalnessMapBinding >= 0)
		return sample(MetalnessMap, uv).r;
}

float getRoughness(vec2f uv)
{
	optional (MaterialRoughnessMapBinding < 0)
		return MaterialData.Roughness;
	
	optional (MaterialRoughnessMapBinding >= 0)
		return sample(RoughnessMap, uv).r;
}

float getAlphaMask(vec2f uv)
{
	optional (MaterialAlphaMaskMapBinding < 0)
		return 1.0;
	
	optional (MaterialAlphaMaskMapBinding >= 0)
		return sample(AlphaMaskMap, uv).r;
}

float getAlphaMaskThreshold()
{
	optional (MaterialAlphaMaskMapBinding < 0)
		return 0.5;
	
	optional (MaterialAlphaMaskMapBinding >= 0)
		return MaterialData.AlphaMaskThreshold;
}

MaterialFragmentParameters getMaterialFragmentParameters()
{
	MaterialFragmentParameters parameters = getDefaultMaterialFragmentParameters();
	
	vec2f uv = getUV();
	
	parameters.BaseColor = getBaseColor(uv);
	parameters.EmissiveColor = getEmissiveColor(uv);
	parameters.Metalness = getMetalness(uv);
	parameters.Roughness = getRoughness(uv);
	parameters.AmbientOcclusion = getAmbientOcclusion(uv);
	parameters.Normal = getNormal(uv);
	
	optional (MaterialAlphaMaskMapBinding >= 0)
	{
		parameters.AlphaMask = getAlphaMask(uv);
		parameters.AlphaMaskThreshold = getAlphaMaskThreshold();
	}
	
	return parameters;
}
)");

ATEMA_DEFAULT_MATERIAL(PBR, R"(
option
{
	int MaterialBaseColorMapBinding = -1;
	int MaterialNormalMapBinding = -1;
	int MaterialAmbientOcclusionMapBinding = -1;
	int MaterialHeightMapBinding = -1;
	int MaterialEmissiveColorMapBinding = -1;
	int MaterialMetalnessMapBinding = -1;
	int MaterialRoughnessMapBinding = -1;
	int MaterialAlphaMaskMapBinding = -1;
	int MaterialStructBinding = -1;
	int InstanceSetIndex = 2;
}

include Atema.LightingModel.PBR;

struct MaterialDataStruct
{
	[optional (MaterialBaseColorMapBinding < 0)]
	vec4f BaseColor;
	
	[optional (MaterialEmissiveColorMapBinding < 0)]
	vec3f EmissiveColor;
	
	[optional (MaterialMetalnessMapBinding < 0)]
	float Metalness;
	
	[optional (MaterialRoughnessMapBinding < 0)]
	float Roughness;

	[optional (MaterialAlphaMaskMapBinding >= 0)]
	float AlphaMaskThreshold;
}

external
{
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
	
	[optional (MaterialAlphaMaskMapBinding >= 0)]
	[set(InstanceSetIndex), binding(MaterialAlphaMaskMapBinding)] sampler2Df AlphaMaskMap;
	
	[optional (MaterialBaseColorMapBinding < 0 ||
		MaterialEmissiveColorMapBinding < 0 ||
		MaterialMetalnessMapBinding < 0 ||
		MaterialRoughnessMapBinding < 0 ||
		MaterialAlphaMaskMapBinding >= 0)]
	[set(InstanceSetIndex), binding(MaterialStructBinding)] MaterialDataStruct MaterialData;
}

vec2f getUV()
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

vec3f getBaseColor(vec2f uv)
{
	optional (MaterialBaseColorMapBinding < 0)
		return MaterialData.BaseColor.rgb;
	
	optional (MaterialBaseColorMapBinding >= 0)
		return sample(BaseColorMap, uv).rgb;
}

vec3f getNormal(vec2f uv)
{
	optional (MaterialNormalMapBinding < 0)
		return vec3f(0.5, 0.5, 1.0);
	
	optional (MaterialNormalMapBinding >= 0)
		return sample(NormalMap, uv).xyz;
}

float getAmbientOcclusion(vec2f uv)
{
	optional (MaterialAmbientOcclusionMapBinding < 0)
		return 1.0;
	
	optional (MaterialAmbientOcclusionMapBinding >= 0)
		return sample(AmbientOcclusionMap, uv).r;
}

float getHeight(vec2f uv)
{
	optional (MaterialHeightMapBinding < 0)
		return 1.0;
	
	optional (MaterialHeightMapBinding >= 0)
		return sample(HeightMap, uv).r;
}

vec3f getEmissiveColor(vec2f uv)
{
	optional (MaterialEmissiveColorMapBinding < 0)
		return MaterialData.EmissiveColor;
	
	optional (MaterialEmissiveColorMapBinding >= 0)
		return sample(EmissiveColorMap, uv).rgb;
}

float getMetalness(vec2f uv)
{
	optional (MaterialMetalnessMapBinding < 0)
		return MaterialData.Metalness;
	
	optional (MaterialMetalnessMapBinding >= 0)
		return sample(MetalnessMap, uv).r;
}

float getRoughness(vec2f uv)
{
	optional (MaterialRoughnessMapBinding < 0)
		return MaterialData.Roughness;
	
	optional (MaterialRoughnessMapBinding >= 0)
		return sample(RoughnessMap, uv).r;
}

float getAlphaMask(vec2f uv)
{
	optional (MaterialAlphaMaskMapBinding < 0)
		return 1.0;
	
	optional (MaterialAlphaMaskMapBinding >= 0)
		return sample(AlphaMaskMap, uv).r;
}

float getAlphaMaskThreshold()
{
	optional (MaterialAlphaMaskMapBinding < 0)
		return 0.5;
	
	optional (MaterialAlphaMaskMapBinding >= 0)
		return MaterialData.AlphaMaskThreshold;
}

MaterialFragmentParameters getMaterialFragmentParameters()
{
	MaterialFragmentParameters parameters = getDefaultMaterialFragmentParameters();
	
	vec2f uv = getUV();
	
	parameters.BaseColor = getBaseColor(uv);
	parameters.EmissiveColor = getEmissiveColor(uv);
	parameters.Metalness = getMetalness(uv);
	parameters.Roughness = getRoughness(uv);
	parameters.AmbientOcclusion = getAmbientOcclusion(uv);
	parameters.Normal = getNormal(uv);
	
	optional (MaterialAlphaMaskMapBinding >= 0)
	{
		parameters.AlphaMask = getAlphaMask(uv);
		parameters.AlphaMaskThreshold = getAlphaMaskThreshold();
	}
	
	return parameters;
}
)");

	struct DefaultEmissiveData : public ShaderData
	{
		Color EmissiveColor = Color::Black;

		size_t getByteSize(StructLayout structLayout) const noexcept override
		{
			BufferLayout layout(structLayout);

			layout.add(BufferElementType::Float4);

			return layout.getSize();
		}

		void copyTo(void* dstData, StructLayout structLayout) const override
		{
			mapMemory<Vector4f>(dstData, 0) = EmissiveColor.toVector4f();
		}
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
			int32_t AlphaMaskMap = InvalidBinding;
		};

		struct Offsets
		{
			static constexpr size_t InvalidOffset = std::numeric_limits<size_t>::max();

			size_t baseColor = InvalidOffset;
			size_t emissiveColor = InvalidOffset;
			size_t metalness = InvalidOffset;
			size_t roughness = InvalidOffset;
			size_t alphaMaskThreshold = InvalidOffset;
		};

		std::filesystem::path BaseColorMapPath;
		std::filesystem::path NormalMapPath;
		std::filesystem::path AmbientOcclusionMapPath;
		std::filesystem::path HeightMapPath;
		std::filesystem::path EmissiveColorMapPath;
		std::filesystem::path MetalnessMapPath;
		std::filesystem::path RoughnessMapPath;
		std::filesystem::path AlphaMaskMapPath;

		Color BaseColor = Color::White;
		Color EmissiveColor = Color::Black;
		float Metalness = 0.0f;
		float Roughness = 0.0f;
		float AlphaMaskThreshold = 0.5f;

		int32_t structBinding = InvalidBinding;
		TextureBindings textureBindings;
		Offsets offsets;
		size_t byteSize = 0;
		bool useUniformBuffer = false;

		DefaultPhongData() = delete;
		DefaultPhongData(const MaterialData& MaterialData, StructLayout structLayout = StructLayout::Default)
		{
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
			ATEMA_PHONG_CHECK_TEXTURE(AlphaMaskMap)

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
			ATEMA_PHONG_GET_PARAMETER(float, AlphaMaskThreshold)

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

			if (!AlphaMaskMapPath.empty())
				offsets.alphaMaskThreshold = layout.add(BufferElementType::Float);

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

			if (offsets.alphaMaskThreshold != Offsets::InvalidOffset)
				mapMemory<float>(dstData, offsets.alphaMaskThreshold) = AlphaMaskThreshold;
		}
	};

	struct DefaultPBRData : public ShaderData
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
			int32_t AlphaMaskMap = InvalidBinding;
		};

		struct Offsets
		{
			static constexpr size_t InvalidOffset = std::numeric_limits<size_t>::max();

			size_t baseColor = InvalidOffset;
			size_t emissiveColor = InvalidOffset;
			size_t metalness = InvalidOffset;
			size_t roughness = InvalidOffset;
			size_t alphaMaskThreshold = InvalidOffset;
		};

		std::filesystem::path BaseColorMapPath;
		std::filesystem::path NormalMapPath;
		std::filesystem::path AmbientOcclusionMapPath;
		std::filesystem::path HeightMapPath;
		std::filesystem::path EmissiveColorMapPath;
		std::filesystem::path MetalnessMapPath;
		std::filesystem::path RoughnessMapPath;
		std::filesystem::path AlphaMaskMapPath;

		Color BaseColor = Color::White;
		Color EmissiveColor = Color::Black;
		float Metalness = 0.0f;
		float Roughness = 0.0f;
		float AlphaMaskThreshold = 0.5f;

		int32_t structBinding = InvalidBinding;
		TextureBindings textureBindings;
		Offsets offsets;
		size_t byteSize = 0;
		bool useUniformBuffer = false;

		DefaultPBRData() = delete;
		DefaultPBRData(const MaterialData& MaterialData, StructLayout structLayout = StructLayout::Default)
		{
			int32_t bindingIndex = 0;

			// Load textures
#define ATEMA_PBR_CHECK_TEXTURE(atTextureName) \
	if (MaterialData.exists(MaterialData::atTextureName)) \
	{ \
		auto& parameter = MaterialData.getValue(MaterialData::atTextureName); \
		if (parameter.is<MaterialData::Texture>() && !parameter.get<MaterialData::Texture>().path.empty()) \
		{ \
			atTextureName ## Path = parameter.get<MaterialData::Texture>().path; \
			textureBindings.atTextureName = bindingIndex++; \
		} \
	}

			ATEMA_PBR_CHECK_TEXTURE(BaseColorMap)
			ATEMA_PBR_CHECK_TEXTURE(NormalMap)
			ATEMA_PBR_CHECK_TEXTURE(AmbientOcclusionMap)
			ATEMA_PBR_CHECK_TEXTURE(HeightMap)
			ATEMA_PBR_CHECK_TEXTURE(EmissiveColorMap)
			ATEMA_PBR_CHECK_TEXTURE(MetalnessMap)
			ATEMA_PBR_CHECK_TEXTURE(RoughnessMap)
			ATEMA_PBR_CHECK_TEXTURE(AlphaMaskMap)

#undef ATEMA_PBR_CHECK_TEXTURE

#define ATEMA_PBR_GET_PARAMETER(atType, atName) \
	if (MaterialData.exists(MaterialData::atName)) \
	{ \
		auto& parameter = MaterialData.getValue(MaterialData::atName); \
		if (parameter.is<atType>()) \
			atName = parameter.get<atType>(); \
	}

			ATEMA_PBR_GET_PARAMETER(Color, BaseColor)
			ATEMA_PBR_GET_PARAMETER(Color, EmissiveColor)
			ATEMA_PBR_GET_PARAMETER(float, Metalness)
			ATEMA_PBR_GET_PARAMETER(float, Roughness)
			ATEMA_PBR_GET_PARAMETER(float, AlphaMaskThreshold)

#undef ATEMA_PBR_GET_PARAMETER

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

			if (!AlphaMaskMapPath.empty())
				offsets.alphaMaskThreshold = layout.add(BufferElementType::Float);

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

			if (offsets.alphaMaskThreshold != Offsets::InvalidOffset)
				mapMemory<float>(dstData, offsets.alphaMaskThreshold) = AlphaMaskThreshold;
		}
	};
}

Ptr<Material> DefaultMaterials::getEmissive(const MaterialData& materialData)
{
	auto& graphics = Graphics::instance();

	// Ensure the default shader is loaded
	if (!graphics.uberShaderExists(EmissiveMaterial))
		graphics.setUberShader(EmissiveMaterial, EmissiveMaterialShader);

	// Build shader data
	const DefaultPhongData phongData(materialData);

	// Define shader options
	const std::vector<UberShader::Option> shaderOptions =
	{
	};

	// Get the raw shader
	auto uberShader = graphics.getUberShader(EmissiveMaterial);

	// Preprocess the shader with the options we just defined
	uberShader = graphics.getUberShader(*uberShader, shaderOptions);

	MaterialData metaData;
	metaData.set(MaterialData::LightingModel, "Emissive");

	return graphics.getMaterial(*uberShader, metaData);
}

Ptr<MaterialInstance> DefaultMaterials::getEmissiveInstance(const MaterialData& materialData)
{
	auto material = getEmissive(materialData);

	auto materialInstance = std::make_shared<MaterialInstance>(material);

	DefaultEmissiveData unlitData;
	if (materialData.exists(MaterialData::EmissiveColor) && materialData.getValue(MaterialData::EmissiveColor).is<Color>())
		unlitData.EmissiveColor = materialData.getValue(MaterialData::EmissiveColor).get<Color>();

	materialInstance->setParameter("MaterialData", unlitData);

	return materialInstance;
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
		{ "MaterialAlphaMaskMapBinding", phongData.textureBindings.AlphaMaskMap },
		{ "MaterialStructBinding", phongData.structBinding },
		{ "UseAlphaMask", phongData.textureBindings.AlphaMaskMap != DefaultPhongData::InvalidBinding }
	};

	// Get the raw shader
	auto uberShader = graphics.getUberShader(PhongMaterial);

	// Preprocess the shader with the options we just defined
	uberShader = graphics.getUberShader(*uberShader, shaderOptions);

	MaterialData metaData;
	metaData.set(MaterialData::LightingModel, "Phong");

	return graphics.getMaterial(*uberShader, metaData);
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
	ATEMA_PHONG_SET_TEXTURE(AlphaMaskMap)

#undef ATEMA_PHONG_SET_TEXTURE

	if (phongData.useUniformBuffer)
		materialInstance->setParameter("MaterialData", phongData);

	return materialInstance;
}

Ptr<Material> DefaultMaterials::getPBR(const MaterialData& materialData)
{
	auto& graphics = Graphics::instance();

	// Ensure the default shader is loaded
	if (!graphics.uberShaderExists(PBRMaterial))
		graphics.setUberShader(PBRMaterial, PBRMaterialShader);

	// Build shader data
	const DefaultPBRData pbrData(materialData);

	// Define shader options
	const std::vector<UberShader::Option> shaderOptions =
	{
		{ "MaterialBaseColorMapBinding", pbrData.textureBindings.BaseColorMap },
		{ "MaterialNormalMapBinding", pbrData.textureBindings.NormalMap },
		{ "MaterialAmbientOcclusionMapBinding", pbrData.textureBindings.AmbientOcclusionMap },
		{ "MaterialHeightMapBinding", pbrData.textureBindings.HeightMap },
		{ "MaterialEmissiveColorMapBinding", pbrData.textureBindings.EmissiveColorMap },
		{ "MaterialMetalnessMapBinding", pbrData.textureBindings.MetalnessMap },
		{ "MaterialRoughnessMapBinding", pbrData.textureBindings.RoughnessMap },
		{ "MaterialAlphaMaskMapBinding", pbrData.textureBindings.AlphaMaskMap },
		{ "MaterialStructBinding", pbrData.structBinding },
		{ "UseAlphaMask", pbrData.textureBindings.AlphaMaskMap != DefaultPBRData::InvalidBinding }
	};

	// Get the raw shader
	auto uberShader = graphics.getUberShader(PBRMaterial);

	// Preprocess the shader with the options we just defined
	uberShader = graphics.getUberShader(*uberShader, shaderOptions);

	MaterialData metaData;
	metaData.set(MaterialData::LightingModel, "PBR");

	return graphics.getMaterial(*uberShader, metaData);
}

Ptr<MaterialInstance> DefaultMaterials::getPBRInstance(const MaterialData& materialData)
{
	auto& graphics = Graphics::instance();

	auto material = getPBR(materialData);

	auto materialInstance = std::make_shared<MaterialInstance>(material);

	// Default texture sampler
	const Sampler::Settings samplerSettings(SamplerFilter::Linear, true);
	const auto sampler = graphics.getSampler(samplerSettings);

	const DefaultPBRData pbrData(materialData);

#define ATEMA_PBR_SET_TEXTURE(atTextureName) \
	if (!pbrData.atTextureName ## Path.empty()) \
	{ \
		const auto image = graphics.getImage(pbrData.atTextureName ## Path); \
		materialInstance->setParameter(#atTextureName, image, sampler); \
	}

	ATEMA_PBR_SET_TEXTURE(BaseColorMap)
	ATEMA_PBR_SET_TEXTURE(NormalMap)
	ATEMA_PBR_SET_TEXTURE(AmbientOcclusionMap)
	ATEMA_PBR_SET_TEXTURE(HeightMap)
	ATEMA_PBR_SET_TEXTURE(EmissiveColorMap)
	ATEMA_PBR_SET_TEXTURE(MetalnessMap)
	ATEMA_PBR_SET_TEXTURE(RoughnessMap)
	ATEMA_PBR_SET_TEXTURE(AlphaMaskMap)

#undef ATEMA_PBR_SET_TEXTURE

	if (pbrData.useUniformBuffer)
		materialInstance->setParameter("MaterialData", pbrData);

	return materialInstance;
}
