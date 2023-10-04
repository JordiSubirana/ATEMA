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

#include <sstream>
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/RenderMaterial.hpp>
#include <Atema/Graphics/RenderScene.hpp>
#include <Atema/Graphics/Passes/EnvironmentPrefilterPass.hpp>
#include <Atema/Shader/Glsl/GlslShaderWriter.hpp>

using namespace at;

namespace
{
	constexpr uint32_t EnvironmentSetIndex = 1;
	constexpr uint32_t PrefilterDataSetIndex = 2;

	const std::string EnvironmentPrefilterShaderName = "EnvironmentPrefilterShader";
	constexpr char EnvironmentPrefilterShader[] = R"(
include Atema.CubemapPass;

struct PrefilterDataStruct
{
	float Roughness;
	uint SampleCount;
	float MipLevelOffset;
	uint Resolution;
}

external
{
	[set(1), binding(0)] samplerCubef EnvironmentMap;
	[set(2), binding(0)] PrefilterDataStruct PrefilterData;
}

const float PI = 3.14159265359;

float DistributionGGX(vec3f N, vec3f H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom   = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}

float RadicalInverse_VdC(uint bits) 
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2f Hammersley(uint i, uint N)
{
	return vec2f(float(i)/float(N), RadicalInverse_VdC(i));
}

vec3f ImportanceSampleGGX(vec2f Xi, vec3f N, float roughness)
{
	float a = roughness * roughness;

	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	// from spherical coordinates to cartesian coordinates - halfway vector
	vec3f H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// from tangent-space H vector to world-space sample vector
	vec3f up = (abs(N.z) < 0.999) ? vec3f(0.0, 0.0, 1.0) : vec3f(1.0, 0.0, 0.0);
	vec3f tangent = normalize(cross(up, N));
	vec3f bitangent = cross(N, tangent);

	vec3f sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

vec4f getCubemapFaceColor(int faceIndex, vec3f uvw)
{		
	vec3f N = uvw;
	const uint sampleCount = PrefilterData.SampleCount;
	const float mipLevelOffset = PrefilterData.MipLevelOffset;
	const float roughness = PrefilterData.Roughness;
	const float resolution = float(PrefilterData.Resolution);
	const float resolution2 = resolution * resolution;

	// make the simplifying assumption that V equals R equals the normal 
	vec3f R = N;
	vec3f V = R;

	vec3f prefilteredColor = vec3f(0.0);
	float totalWeight = 0.0;

	for(uint i = 0u; i < sampleCount; ++i)
	{
		// generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
		vec2f Xi = Hammersley(i, sampleCount);
		vec3f H = ImportanceSampleGGX(Xi, N, roughness);
		vec3f L = normalize(2.0 * dot(V, H) * H - V);

		float NdotL = max(dot(N, L), 0.0);
		if(NdotL > 0.0)
		{
			// sample from the environment's mip level based on roughness/pdf
			float D = DistributionGGX(N, H, roughness);
			float NdotH = max(dot(N, H), 0.0);
			float HdotV = max(dot(H, V), 0.0);
			float pdf = (D * NdotH / (4.0 * HdotV)) + 0.0001;

			float saTexel = (4.0 * PI) / (6.0 * resolution2);
			float saSample = 1.0 / (float(sampleCount) * pdf + 0.0001);
			
			float mipLevel = (roughness == 0.0) ? 0.0 : (0.5 * log2(saSample / saTexel) + mipLevelOffset);
			prefilteredColor = prefilteredColor + textureLod(EnvironmentMap, L, mipLevel).rgb * NdotL;
			totalWeight = totalWeight + NdotL;
		}
	}

	prefilteredColor = prefilteredColor / totalWeight;

	return vec4f(prefilteredColor, 1.0);
}
)";

class PrefilterData : public ShaderData
{
public:
	class Layout : public ShaderData::Layout
	{
	public:
		Layout() = delete;
		Layout(StructLayout structLayout = StructLayout::Default)
		{
			BufferLayout bufferLayout(structLayout);

			roughnessOffset = bufferLayout.add(BufferElementType::Float);
			sampleCountOffset = bufferLayout.add(BufferElementType::UInt);
			mipLevelOffsetOffset = bufferLayout.add(BufferElementType::Float);
			resolutionOffset = bufferLayout.add(BufferElementType::UInt);

			initialize(bufferLayout);
		}

		size_t roughnessOffset;
		size_t sampleCountOffset;
		size_t mipLevelOffsetOffset;
		size_t resolutionOffset;
	};

	PrefilterData() = default;
	PrefilterData(const PrefilterData& other) = default;
	PrefilterData(PrefilterData&& other) noexcept = default;
	virtual ~PrefilterData() = default;

	size_t getByteSize(StructLayout structLayout = StructLayout::Default) const noexcept override
	{
		return Layout(structLayout).getByteSize();
	}

	void copyTo(void* dstData, StructLayout structLayout = StructLayout::Default) const override
	{
		const Layout layout(structLayout);

		mapMemory<float>(dstData, layout.roughnessOffset) = roughness;
		mapMemory<uint32_t>(dstData, layout.sampleCountOffset) = sampleCount;
		mapMemory<float>(dstData, layout.mipLevelOffsetOffset) = mipLevelOffset;
		mapMemory<uint32_t>(dstData, layout.resolutionOffset) = resolution;
	}

	float roughness = 0.0f;
	uint32_t sampleCount = 1024;
	float mipLevelOffset = 1.0f;
	uint32_t resolution = 1024;

	PrefilterData& operator=(const PrefilterData& other) = default;
	PrefilterData& operator=(PrefilterData&& other) noexcept = default;
};
}

EnvironmentPrefilterPass::EnvironmentPrefilterPass(RenderResourceManager& resourceManager) :
	m_resourceManager(&resourceManager),
	m_updateResources(true),
	m_roughness(0.0f),
	m_sampleCount(1024),
	m_mipLevelOffset(1.0f),
	m_resolution(1024),
	m_environmentMap(nullptr)
{
	// Material
	Graphics& graphics = Graphics::instance();

	if (!graphics.uberShaderExists(EnvironmentPrefilterShaderName))
		graphics.setUberShader(EnvironmentPrefilterShaderName, EnvironmentPrefilterShader);
	
	std::ostringstream stream;
	auto uber = graphics.getUberShader(EnvironmentPrefilterShaderName);
	uber = graphics.getUberShader(*uber, {}, &ShaderLibraryManager::instance());
	GlslShaderWriter glsl(stream);
	glsl.visit(*uber->getAst());
	auto code = stream.str();

	RenderMaterial::Settings renderMaterialSettings;
	renderMaterialSettings.material = std::make_shared<Material>(graphics.getUberShader(EnvironmentPrefilterShaderName));
	renderMaterialSettings.shaderLibraryManager = &ShaderLibraryManager::instance();
	renderMaterialSettings.pipelineState.depth.test = false;
	renderMaterialSettings.pipelineState.depth.write = false;
	renderMaterialSettings.pipelineState.rasterization.cullMode = CullMode::None;

	auto renderMaterial = std::make_shared<RenderMaterial>(resourceManager, renderMaterialSettings);

	initialize(std::move(renderMaterial));

	// Buffer
	Buffer::Settings bufferSettings;
	bufferSettings.usages = BufferUsage::Uniform | BufferUsage::TransferDst;
	bufferSettings.byteSize = PrefilterData().getByteSize();

	m_prefilterBuffer = resourceManager.createBuffer(bufferSettings);

	m_prefilterDescriptorSet = getRenderMaterial().createSet(PrefilterDataSetIndex);
	m_prefilterDescriptorSet->update(0, m_prefilterBuffer->getBuffer(), m_prefilterBuffer->getOffset(), m_prefilterBuffer->getSize());

	// Sampler
	Sampler::Settings samplerSettings(SamplerFilter::Linear, true);

	m_sampler = graphics.getSampler(samplerSettings);
}

const char* EnvironmentPrefilterPass::getName() const noexcept
{
	return "EnvironmentPrefilter";
}

void EnvironmentPrefilterPass::setRoughness(float roughness)
{
	m_roughness = roughness;

	m_updateResources = true;
}

float EnvironmentPrefilterPass::getRoughness() const noexcept
{
	return m_roughness;
}

void EnvironmentPrefilterPass::setSampleCount(uint32_t sampleCount)
{
	m_sampleCount = sampleCount;

	m_updateResources = true;
}

uint32_t EnvironmentPrefilterPass::getSampleCount() const noexcept
{
	return m_sampleCount;
}

void EnvironmentPrefilterPass::setMipLevelOffset(float mipLevelOffset)
{
	m_mipLevelOffset = mipLevelOffset;

	m_updateResources = true;
}

float EnvironmentPrefilterPass::getMipLevelOffset() const noexcept
{
	return m_mipLevelOffset;
}

void EnvironmentPrefilterPass::setEnvironmentMap(Image& environmentMap)
{
	m_environmentMap = &environmentMap;

	m_resolution = environmentMap.getSize().x;

	m_updateResources = true;
}

void EnvironmentPrefilterPass::updateResources(CommandBuffer& commandBuffer)
{
	if (m_updateResources)
	{
		void* data = m_resourceManager->mapBuffer(*m_prefilterBuffer);

		PrefilterData prefilterData;
		prefilterData.roughness = m_roughness;
		prefilterData.sampleCount = m_sampleCount;
		prefilterData.mipLevelOffset = m_mipLevelOffset;
		prefilterData.resolution = m_resolution;

		prefilterData.copyTo(data);

		m_updateResources = false;
	}
}

void EnvironmentPrefilterPass::initializeFrameGraphPass(FrameGraphPass& frameGraphPass, FrameGraphBuilder& frameGraphBuilder, const FrameGraphSettings& settings)
{
	for (auto& texture : settings.environmentMapFaces)
		frameGraphPass.addSampledTexture(texture, ShaderStage::Fragment);
}

bool EnvironmentPrefilterPass::bindResources(FrameGraphContext& context, const FrameGraphSettings& settings)
{
	if (!m_environmentMap || m_environmentMap->getType() != ImageType::CubeMap)
		return false;

	Ptr<DescriptorSet> textureSet = getRenderMaterial().createSet(EnvironmentSetIndex);

	textureSet->update(0, *m_environmentMap->getView(), *m_sampler);

	context.getCommandBuffer().bindDescriptorSet(EnvironmentSetIndex, *textureSet);
	context.getCommandBuffer().bindDescriptorSet(PrefilterDataSetIndex, *m_prefilterDescriptorSet);

	context.destroyAfterUse(std::move(textureSet));

	return true;
}
