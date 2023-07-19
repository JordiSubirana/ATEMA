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

#define ATEMA_DEFAULT_MODEL(atName, atShader) const std::string atName ## Model("Default" #atName "Model"); \
	constexpr const char atName ## ModelShader[] = atShader;

using namespace at;

namespace
{
	ATEMA_DEFAULT_MODEL(Phong, R"(
option
{
	bool LightModelUseOpacityMask = true;
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
	
	GBufferWritePosition(inPosition);
	GBufferWriteNormal(normal);
	GBufferWriteBaseColor(parameters.BaseColor);
	GBufferWriteAmbientOcclusion(parameters.AmbientOcclusion);
	GBufferWriteEmissiveColor(parameters.EmissiveColor);
	GBufferWriteMetalness(parameters.Metalness);
	GBufferWriteRoughness(parameters.Roughness);
}
)");
}

LightingModel DefaultLightingModels::getPhong()
{
	auto& graphics = Graphics::instance();

	// Ensure the default shader is loaded
	if (!graphics.uberShaderExists(PhongModel))
		graphics.setUberShader(PhongModel, PhongModelShader);

	LightingModel model;
	model.name = "Phong";
	model.parameters =
	{
		{ "Position", ImageFormat::RGB32_SFLOAT },
		{ "Normal", ImageFormat::RGB16_SFLOAT },
		{ "BaseColor", ImageFormat::RGB8_UNORM },
		{ "EmissiveColor", ImageFormat::RGB8_UNORM },
		{ "Metalness", ImageFormat::R8_UNORM },
		{ "Roughness", ImageFormat::R8_UNORM },
		{ "AmbientOcclusion", ImageFormat::R8_UNORM },
	};
	model.materialLibrary = graphics.getUberShader(PhongModel);

	return model;
}
