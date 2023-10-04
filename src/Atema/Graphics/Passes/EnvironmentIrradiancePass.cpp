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

#include <Atema/Graphics/Passes/EnvironmentIrradiancePass.hpp>
#include <Atema/Graphics/FrameGraphBuilder.hpp>
#include <Atema/Graphics/FrameGraphContext.hpp>
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/RenderMaterial.hpp>
#include <Atema/Graphics/RenderScene.hpp>

using namespace at;

namespace
{
	constexpr uint32_t EnvironmentSetIndex = 1;
	constexpr uint32_t IrradianceDataSetIndex = 2;

	const std::string IrradianceShaderName = "EnvironmentIrradianceShader";
	constexpr char IrradianceShader[] = R"(
include Atema.CubemapPass;

struct IrradianceDataStruct
{
	uint SampleCount;
	float MipLevelOffset;
	uint Resolution;
}

external
{
	[set(1), binding(0)] samplerCubef EnvironmentMap;
	[set(2), binding(0)] IrradianceDataStruct IrradianceData;
}

const float PI = 3.14159265359;

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
	const uint sampleCount = IrradianceData.SampleCount;
	const float mipLevelOffset = IrradianceData.MipLevelOffset;
	const float resolution = float(IrradianceData.Resolution);
	const float resolution2 = resolution * resolution;

	vec3f up = vec3f(0.0, 1.0, 0.0);
	vec3f right = normalize(cross(up, N));
	up = normalize(cross(N, right));
	
	vec3f V = N;
	
	vec3f irradiance = vec3f(0.0);
	float totalWeight = 0.0;

	for(uint i = 0u; i < sampleCount; ++i)
	{
		// generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
		vec2f Xi = Hammersley(i, sampleCount);
		vec3f H = ImportanceSampleGGX(Xi, N, 1.0);

		float D = 1.0 / PI;
		float NdotH = max(dot(N, H), 0.0);
		float pdf = (D * NdotH / (4.0)) + 0.0001;

		float saTexel = 4.0 * PI / (6.0 * resolution2);
		float saSample = 1.0 / (float(sampleCount) * pdf + 0.0001);

		float mipLevel = (0.5 * log2(saSample / saTexel)) + mipLevelOffset;

		irradiance = irradiance + textureLod(EnvironmentMap, H, mipLevel).rgb * NdotH;
		totalWeight = totalWeight + NdotH;
	}
	
	irradiance = (PI * irradiance) / totalWeight;
	
	return vec4f(irradiance, 1.0);
}
)";

class IrradianceData : public ShaderData
{
public:
	class Layout : public ShaderData::Layout
	{
	public:
		Layout() = delete;
		Layout(StructLayout structLayout = StructLayout::Default)
		{
			BufferLayout bufferLayout(structLayout);
			
			sampleCountOffset = bufferLayout.add(BufferElementType::UInt);
			mipLevelOffsetOffset = bufferLayout.add(BufferElementType::Float);
			resolutionOffset = bufferLayout.add(BufferElementType::UInt);

			initialize(bufferLayout);
		}
		
		size_t sampleCountOffset;
		size_t mipLevelOffsetOffset;
		size_t resolutionOffset;
	};

	IrradianceData() = default;
	IrradianceData(const IrradianceData& other) = default;
	IrradianceData(IrradianceData&& other) noexcept = default;
	virtual ~IrradianceData() = default;

	size_t getByteSize(StructLayout structLayout = StructLayout::Default) const noexcept override
	{
		return Layout(structLayout).getByteSize();
	}

	void copyTo(void* dstData, StructLayout structLayout = StructLayout::Default) const override
	{
		const Layout layout(structLayout);
		
		mapMemory<uint32_t>(dstData, layout.sampleCountOffset) = sampleCount;
		mapMemory<float>(dstData, layout.mipLevelOffsetOffset) = mipLevelOffset;
		mapMemory<uint32_t>(dstData, layout.resolutionOffset) = resolution;
	}
	
	uint32_t sampleCount = 1024;
	float mipLevelOffset = 1.0f;
	uint32_t resolution = 1024;

	IrradianceData& operator=(const IrradianceData& other) = default;
	IrradianceData& operator=(IrradianceData&& other) noexcept = default;
};
}

EnvironmentIrradiancePass::EnvironmentIrradiancePass(RenderResourceManager& resourceManager) :
	m_resourceManager(&resourceManager),
	m_updateResources(true),
	m_sampleCount(1024),
	m_mipLevelOffset(1.0f),
	m_resolution(1024),
	m_environmentMap(nullptr)
{
	// Material
	Graphics& graphics = Graphics::instance();

	if (!graphics.uberShaderExists(IrradianceShaderName))
		graphics.setUberShader(IrradianceShaderName, IrradianceShader);

	RenderMaterial::Settings renderMaterialSettings;
	renderMaterialSettings.material = std::make_shared<Material>(graphics.getUberShader(IrradianceShaderName));
	renderMaterialSettings.shaderLibraryManager = &ShaderLibraryManager::instance();
	renderMaterialSettings.pipelineState.depth.test = false;
	renderMaterialSettings.pipelineState.depth.write = false;
	renderMaterialSettings.pipelineState.rasterization.cullMode = CullMode::None;

	auto renderMaterial = std::make_shared<RenderMaterial>(resourceManager, renderMaterialSettings);

	initialize(std::move(renderMaterial));

	// Buffer
	Buffer::Settings bufferSettings;
	bufferSettings.usages = BufferUsage::Uniform | BufferUsage::TransferDst;
	bufferSettings.byteSize = IrradianceData().getByteSize();

	m_irradianceBuffer = resourceManager.createBuffer(bufferSettings);

	m_irradianceDescriptorSet = getRenderMaterial().createSet(IrradianceDataSetIndex);
	m_irradianceDescriptorSet->update(0, m_irradianceBuffer->getBuffer(), m_irradianceBuffer->getOffset(), m_irradianceBuffer->getSize());

	// Sampler
	m_sampler = graphics.getSampler(Sampler::Settings(SamplerFilter::Linear));
}

const char* EnvironmentIrradiancePass::getName() const noexcept
{
	return "EnvironmentIrradiance";
}

void EnvironmentIrradiancePass::setSampleCount(uint32_t sampleCount)
{
	m_sampleCount = sampleCount;

	m_updateResources = true;
}

uint32_t EnvironmentIrradiancePass::getSampleCount() const noexcept
{
	return m_sampleCount;
}

void EnvironmentIrradiancePass::setMipLevelOffset(float mipLevelOffset)
{
	m_mipLevelOffset = mipLevelOffset;

	m_updateResources = true;
}

float EnvironmentIrradiancePass::getMipLevelOffset() const noexcept
{
	return m_mipLevelOffset;
}

void EnvironmentIrradiancePass::setEnvironmentMap(Image& environmentMap)
{
	m_environmentMap = &environmentMap;

	m_resolution = environmentMap.getSize().x;

	m_updateResources = true;
}

void EnvironmentIrradiancePass::updateResources(CommandBuffer& commandBuffer)
{
	if (m_updateResources)
	{
		void* data = m_resourceManager->mapBuffer(*m_irradianceBuffer);

		IrradianceData irradianceData;
		irradianceData.sampleCount = m_sampleCount;
		irradianceData.mipLevelOffset = m_mipLevelOffset;
		irradianceData.resolution = m_resolution;

		irradianceData.copyTo(data);

		m_updateResources = false;
	}
}

void EnvironmentIrradiancePass::initializeFrameGraphPass(FrameGraphPass& frameGraphPass, FrameGraphBuilder& frameGraphBuilder, const FrameGraphSettings& settings)
{
	for (auto& texture : settings.environmentMapFaces)
		frameGraphPass.addSampledTexture(texture, ShaderStage::Fragment);
}

bool EnvironmentIrradiancePass::bindResources(FrameGraphContext& context, const FrameGraphSettings& settings)
{
	if (!m_environmentMap || m_environmentMap->getType() != ImageType::CubeMap)
		return false;

	Ptr<DescriptorSet> environmentSet = getRenderMaterial().createSet(EnvironmentSetIndex);

	environmentSet->update(0, *m_environmentMap->getView(), *m_sampler);

	context.getCommandBuffer().bindDescriptorSet(EnvironmentSetIndex, *environmentSet);
	context.getCommandBuffer().bindDescriptorSet(IrradianceDataSetIndex, *m_irradianceDescriptorSet);

	context.destroyAfterUse(std::move(environmentSet));

	return true;
}
