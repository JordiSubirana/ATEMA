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

#include <Atema/Graphics/DefaultLightingModels.hpp>
#include <Atema/Graphics/Graphics.hpp>

#define ATEMA_DEFAULT_MODEL(atName, atObjectLib, atLightLib) \
	const std::string atName ## ObjectLibName("Default" #atName "ObjectLibrary"); \
	constexpr const char atName ## ObjectLib[] = atObjectLib; \
	const std::string atName ## LightLibName("Default" #atName "LightLibrary"); \
	constexpr const char atName ## LightLib[] = atLightLib;

using namespace at;

namespace
{
	ATEMA_DEFAULT_MODEL(Emissive, R"(
option
{
	uint LightingModel = 0;
}

include Atema.GBufferWrite;

struct FrameDataStruct
{
	mat4f proj;
	mat4f view;
	vec3f cameraPosition;
	vec2u ScreenSize;
}

struct TransformDataStruct
{
	mat4f model;
}

external
{
	[set(0), binding(0)] FrameDataStruct FrameData;

	[set(1), binding(0)] TransformDataStruct TransformData;
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
	
	vec4f screenPosition = FrameData.proj * FrameData.view * worldPos;
	
	setVertexPosition(screenPosition);
}

[stage(fragment)]
input
{
	[location(0)] vec3f inPosition;
	[location(1)] vec2f inTexCoords;
	[location(2)] mat3f inTBN;
}

struct MaterialFragmentParameters
{
	vec3f EmissiveColor;
}

MaterialFragmentParameters getDefaultMaterialFragmentParameters()
{
	MaterialFragmentParameters parameters;
	
	parameters.EmissiveColor = vec3f(0.0, 0.0, 0.0);
	
	return parameters;
}

MaterialFragmentParameters getMaterialFragmentParameters();

[entry(fragment)]
void main()
{
	MaterialFragmentParameters parameters = getMaterialFragmentParameters();
	
	vec3f normal = normalize(inTBN * vec3f(0.0, 0.0, 1.0));
	
	GBufferWriteLightingModel(LightingModel);
	GBufferWritePosition(inPosition);
	GBufferWriteNormal(normal);
	GBufferWriteEmissiveColor(parameters.EmissiveColor);
}
)",
R"(
vec3f getEmissiveFinalColor(vec2f uv)
{
	return vec3f(0, 0, 0);
}
)");

	ATEMA_DEFAULT_MODEL(Phong, R"(
option
{
	uint LightingModel = 0;
	bool LightModelUseOpacityMask = true;
}

include Atema.GBufferWrite;

struct FrameDataStruct
{
	mat4f proj;
	mat4f view;
	vec3f cameraPosition;
	vec2u ScreenSize;
}

struct TransformDataStruct
{
	mat4f model;
}

external
{
	[set(0), binding(0)] FrameDataStruct FrameData;

	[set(1), binding(0)] TransformDataStruct TransformData;
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

struct MaterialFragmentParameters
{
	vec3f BaseColor;
	vec3f EmissiveColor;
	float Metalness;
	float Roughness;
	float AmbientOcclusion;
	float AlphaMask;
	float AlphaMaskThreshold;
	vec3f Normal;
}

MaterialFragmentParameters getDefaultMaterialFragmentParameters()
{
	MaterialFragmentParameters parameters;
	
	parameters.BaseColor = vec3f(1.0, 1.0, 1.0);
	parameters.EmissiveColor = vec3f(0.0, 0.0, 0.0);
	parameters.Metalness = 0.0;
	parameters.Roughness = 0.0;
	parameters.AmbientOcclusion = 1.0;
	parameters.AlphaMask = 1.0;
	parameters.AlphaMaskThreshold = 0.5;
	parameters.Normal = vec3f(0.5, 0.5, 1.0);
	
	return parameters;
}

MaterialFragmentParameters getMaterialFragmentParameters();

[entry(fragment)]
void main()
{
	MaterialFragmentParameters parameters = getMaterialFragmentParameters();
	
	optional (UseAlphaMask)
	{
		if (parameters.AlphaMask < parameters.AlphaMaskThreshold)
			discard;
	}
	
	vec3f normal = (parameters.Normal * 2.0) - 1.0;
	normal = normalize(inTBN * normal);
	
	GBufferWriteLightingModel(LightingModel);
	GBufferWritePosition(inPosition);
	GBufferWriteNormal(normal);
	GBufferWriteBaseColor(parameters.BaseColor);
	GBufferWriteAmbientOcclusion(parameters.AmbientOcclusion);
	GBufferWriteEmissiveColor(parameters.EmissiveColor);
	GBufferWriteMetalness(parameters.Metalness);
	GBufferWriteRoughness(parameters.Roughness);
}
)",
R"(
vec3f getPhongFinalColor(vec2f uv)
{
	vec3f worldPos = GBufferReadPosition(uv);
	vec3f normal = GBufferReadNormal(uv);
	vec3f baseColor = GBufferReadBaseColor(uv);
	float ambientOcclusion = GBufferReadAmbientOcclusion(uv);
	float metalness = GBufferReadMetalness(uv);
	int roughness = int(GBufferReadRoughness(uv) * 255.0);
	
	//TODO: Add shininess/specular to material data
	const float Pi = 3.14159265;
	const float shininess = 16.0;
	const float specular = metalness;
	
	vec3f lightDirection;
	float lightAttenuation = 1.0;
	
	if (LightData.Type == DirectionalLightType)
	{
		lightDirection = LightData.Parameter0.xyz;
	}
	else if (LightData.Type == PointLightType)
	{
		lightDirection =  worldPos - LightData.Parameter0.xyz;
		
		float distance = length(lightDirection);
		
		lightAttenuation = max(1.0 - distance / LightData.Parameter0.w, 0.0);
	}
	else if (LightData.Type == SpotLightType)
	{
		lightDirection =  worldPos - LightData.Parameter0.xyz;
		
		vec3f spotDirection = LightData.Parameter1.xyz;
		float spotAngle = LightData.Parameter1.a;
		
		float distance = length(lightDirection);
		
		float fragmentAngle = max(dot(spotDirection, lightDirection) / max(distance, 0.001), 0.0);
		
		lightAttenuation = max(1.0 - distance / LightData.Parameter0.w, 0.0);
		lightAttenuation = lightAttenuation * max((fragmentAngle - spotAngle) / (1.0 - spotAngle), 0.0);
	}
	
	vec3f inverseLightDir = -normalize(lightDirection);
		
	float cosTheta = dot(normal, inverseLightDir);
	
	// Ambient
	vec3f ambientColor = LightData.AmbientStrength * LightData.Color * ambientOcclusion;
	
	// Diffuse
	float diffuseFactor = max(cosTheta, 0.0);
	
	vec3f diffuseColor = diffuseFactor * LightData.DiffuseStrength * LightData.Color;
	
	// Specular
	vec3f viewDir = normalize(FrameData.CameraPosition - worldPos);
	
	float specularFactor = 0.0;
	optional (UseBlinnPhong)
	{
		const float energyConservation = ( 8.0 + shininess ) / ( 8.0 * Pi );
		vec3f halfwayDir = normalize(inverseLightDir + viewDir);
		specularFactor = energyConservation * pow(max(dot(normal, halfwayDir), 0.0), shininess);
	}
	
	optional (!UseBlinnPhong)
	{
		const float energyConservation = ( 2.0 + shininess ) / ( 2.0 * Pi );
		vec3f reflectDir = reflect(-inverseLightDir, normal);
		specularFactor = energyConservation * pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	}
	
	vec3f specularColor = specularFactor * specular * LightData.Color;
	
	// Visibility
	float visibility = getVisibility(worldPos, acos(cosTheta));
	
	// Result
	vec3f finalColor = (ambientColor + visibility * (diffuseColor + specularColor)) * baseColor.rgb * lightAttenuation;
	
	return finalColor;
}
)");
}

LightingModel DefaultLightingModels::getEmissive()
{
	static size_t id = LightingModel::generateID();

	auto& graphics = Graphics::instance();

	// Ensure the default shader is loaded
	if (!graphics.uberShaderExists(EmissiveObjectLibName))
		graphics.setUberShader(EmissiveObjectLibName, EmissiveObjectLib);

	if (!graphics.uberShaderExists(EmissiveLightLibName))
		graphics.setUberShader(EmissiveLightLibName, EmissiveLightLib);

	LightingModel model;
	model.id = id;
	model.name = "Emissive";
	model.parameters =
	{
		{ "LightingModel", ImageFormat::R16_SFLOAT },
		{ "Position", ImageFormat::RGB32_SFLOAT },
		{ "Normal", ImageFormat::RGB16_SFLOAT },
		{ "EmissiveColor", ImageFormat::RGB8_UNORM }
	};

	const std::vector<UberShader::Option> uberShaderOptions =
	{
		{ "LightingModel", static_cast<uint32_t>(id) }
	};
	const auto baseUberShader = graphics.getUberShader(EmissiveObjectLibName);
	model.materialLibrary = graphics.getUberShader(*baseUberShader, uberShaderOptions);

	model.lightLibrary = graphics.getUberShader(EmissiveLightLibName);

	return model;
}

LightingModel DefaultLightingModels::getPhong()
{
	static size_t id = LightingModel::generateID();

	auto& graphics = Graphics::instance();

	// Ensure the default shader is loaded
	if (!graphics.uberShaderExists(PhongObjectLibName))
		graphics.setUberShader(PhongObjectLibName, PhongObjectLib);

	if (!graphics.uberShaderExists(PhongLightLibName))
		graphics.setUberShader(PhongLightLibName, PhongLightLib);

	LightingModel model;
	model.id = id;
	model.name = "Phong";
	model.parameters =
	{
		{ "LightingModel", ImageFormat::R16_SFLOAT },
		{ "Position", ImageFormat::RGB32_SFLOAT },
		{ "Normal", ImageFormat::RGB16_SFLOAT },
		{ "BaseColor", ImageFormat::RGB8_UNORM },
		{ "EmissiveColor", ImageFormat::RGB8_UNORM },
		{ "Metalness", ImageFormat::R8_UNORM },
		{ "Roughness", ImageFormat::R8_UNORM },
		{ "AmbientOcclusion", ImageFormat::R8_UNORM },
	};

	const std::vector<UberShader::Option> uberShaderOptions =
	{
		{ "LightingModel", static_cast<uint32_t>(id) }
	};
	const auto baseUberShader = graphics.getUberShader(PhongObjectLibName);
	model.materialLibrary = graphics.getUberShader(*baseUberShader, uberShaderOptions);

	model.lightLibrary = graphics.getUberShader(PhongLightLibName);

	return model;
}
