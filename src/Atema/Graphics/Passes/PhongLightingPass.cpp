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

#include <Atema/Graphics/Passes/PhongLightingPass.hpp>
#include <Atema/Graphics/FrameGraphBuilder.hpp>
#include <Atema/Graphics/FrameGraphContext.hpp>
#include <Atema/Graphics/RenderData.hpp>
#include <Atema/Graphics/VertexBuffer.hpp>
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/VertexTypes.hpp>
#include <Atema/Renderer/DescriptorSetLayout.hpp>
#include <Atema/Renderer/Renderer.hpp>

using namespace at;

namespace
{
	constexpr size_t MaxShadowCascadeCount = 16;

	constexpr uint32_t GBufferSetIndex = 0;
	constexpr uint32_t PhongSetIndex = 1;
	constexpr uint32_t FrameBindingIndex = 0;
	constexpr uint32_t ShadowBindingIndex = 1;
	constexpr uint32_t LightBindingIndex = 2;

	constexpr char* ShaderName = "AtemaPhongLightingPass";

	constexpr char ShaderCode[] = R"(
option
{
	uint MaxShadowMapCascadeCount = 16;
}

include Atema.GBufferRead;

struct FrameData
{
	vec3f cameraPosition;
	mat4f view;
}

struct ShadowData
{
	uint count;
	mat4f viewProjection[MaxShadowMapCascadeCount];
	float depth[MaxShadowMapCascadeCount];
	float depthBias[MaxShadowMapCascadeCount];
}

struct LightData
{
	vec3f direction;
	vec3f color;
	float ambientStrength;
}

external
{
	[set(0), binding(4)] sampler2DArray shadowMap;
	
	[set(1), binding(0)] FrameData frameData;

	[set(1), binding(1)] ShadowData shadowData;
	
	[set(1), binding(2)] LightData lightData;
}

[stage(vertex)]
input
{
	[location(0)] vec3f inPosition;
	[location(1)] vec2f inTexCoords;
}

[stage(vertex)]
output
{
	[location(0)] vec2f outTexCoords;
}

[entry(vertex)]
void main()
{
	vec4f position = vec4f(inPosition, 1.0);
	
	outTexCoords = inTexCoords;
	
	setVertexPosition(position);
}

[stage(fragment)]
input
{
	[location(0)] vec2f inTexCoords;
}

[stage(fragment)]
output
{
	[location(0)] vec4f outColor;
}

vec2f poissonDisk[16] = vec2f[]( 
   vec2f( -0.94201624, -0.39906216 ), 
   vec2f( 0.94558609, -0.76890725 ), 
   vec2f( -0.094184101, -0.92938870 ), 
   vec2f( 0.34495938, 0.29387760 ), 
   vec2f( -0.91588581, 0.45771432 ), 
   vec2f( -0.81544232, -0.87912464 ), 
   vec2f( -0.38277543, 0.27676845 ), 
   vec2f( 0.97484398, 0.75648379 ), 
   vec2f( 0.44323325, -0.97511554 ), 
   vec2f( 0.53742981, -0.47373420 ), 
   vec2f( -0.26496911, -0.41893023 ), 
   vec2f( 0.79197514, 0.19090188 ), 
   vec2f( -0.24188840, 0.99706507 ), 
   vec2f( -0.81409955, 0.91437590 ), 
   vec2f( 0.19984126, 0.78641367 ), 
   vec2f( 0.14383161, -0.14100790 ) 
);

float random(vec3f seed, int i)
{
	vec4f seed4 = vec4f(seed,i);
	
	float dotProduct = dot(seed4, vec4f(12.9898,78.233,45.164,94.673));
	
	return fract(sin(dotProduct) * 43758.5453);
}

float sampleVisibility(vec2f uv, uint cascadeIndex, float shadowZ)
{
	float shadowMapZ = sample(shadowMap, vec3f(uv, cascadeIndex)).r;
	
	if (shadowMapZ < shadowZ)
		return 0.0;
	
	return 1.0;
}

float sampleVisibilityPCF(vec2f uv, uint cascadeIndex, float shadowZ)
{
	float scale = 0.1;
	
	vec2i texDim = textureSize(shadowMap, 0).xy;
	float dx = scale / float(texDim.x);
	float dy = scale / float(texDim.y);

	float result = 0.0;
	int count = 0;
	int range = 8;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			result = result + sampleVisibility(uv + vec2(dx*x, dy*y), cascadeIndex, shadowZ);
			count++;
		}
	}
	
	return result / count;
}

float sampleVisibilityPoissonPCF(vec2f uv, uint cascadeIndex, float shadowZ)
{
	float scale = 0.4;
	
	vec2i texDim = textureSize(shadowMap, 0).xy;
	float dx = scale / float(texDim.x);
	float dy = scale / float(texDim.y);

	float result = 0.0;
	int count = 0;
	int range = 4;
	
	vec3f pos = atGBufferReadPosition(inTexCoords);
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			int index = int(int(16.0 * random(floor(pos * 1000.0), x + y * 100)) % 16);
		
			vec2f offset = poissonDisk[index] / 200;
			offset = offset * scale * 0.0;
			
			result = result + sampleVisibility(uv + vec2(dx*x, dy*y) + offset, cascadeIndex, shadowZ);
			count++;
		}
	}
	
	return result / count;
}

float getVisibility(float angle)
{
	vec3f pos = atGBufferReadPosition(inTexCoords);
	
	vec4f posViewSpace = (frameData.view * vec4f(pos, 1.0));
	
	uint cascadeIndex = uint(shadowData.count) - uint(1);
	for(uint i = uint(0); i < shadowData.count; i++)
	{
		if(-posViewSpace.z < shadowData.depth[i])
		{	
			cascadeIndex = i;
			break;
		}
	}
	
	float cascadePoissonStrengths[8] = float[](2000.0, 2000.0, 2000.0, 2000.0, 2000.0, 2000.0, 2000.0, 2000.0);
	
	float cascadePoissonStrength = cascadePoissonStrengths[cascadeIndex];
	
	float cascadeBias = clamp(shadowData.depthBias[cascadeIndex] * tan(angle), 0.0, 0.5);
	
	vec4f shadowCoord = (shadowData.viewProjection[cascadeIndex] * vec4f(pos, 1.0));
	shadowCoord.z = clamp(shadowCoord.z, -1.0, 1.0);
	
	vec2f shadowMapUV = shadowCoord.xy * 0.5 + 0.5;
	float shadowZ = shadowCoord.z - cascadeBias;
	
	return sampleVisibility(shadowMapUV, cascadeIndex, shadowZ);
}

[entry(fragment)]
void main()
{
	vec3f pos = atGBufferReadPosition(inTexCoords);
	vec3f normal = atGBufferReadNormal(inTexCoords);
	vec3f color = atGBufferReadAlbedo(inTexCoords);
	float ao = atGBufferReadAO(inTexCoords);
	float metalness = atGBufferReadMetalness(inTexCoords);
	int roughness = int(atGBufferReadRoughness(inTexCoords) * 255.0);
	vec3f emissiveColor = atGBufferReadEmissive(inTexCoords);
	
	vec3f inverseLightDir = -normalize(lightData.direction);
	
	vec3f ambientColor = lightData.ambientStrength * lightData.color * ao;
	
	float cosTheta = dot(normal, inverseLightDir);
	
	float diffuseFactor = max(cosTheta, 0.0);
	
	vec3f diffuseColor = diffuseFactor * lightData.color;
	
	vec3f viewDir = normalize(frameData.cameraPosition - pos);
	
	vec3f reflectDir = reflect(-inverseLightDir, normal);
	
	float specularFactor = clamp(pow(max(dot(viewDir, reflectDir), 0.0), roughness), 0.0, 1.0);
	
	vec3f specularColor = specularFactor * metalness * lightData.color;
	
	float visibility = getVisibility(acos(cosTheta));
	
	vec3f finalColor = (ambientColor + visibility * (diffuseColor + specularColor)) * color.rgb;
	finalColor = finalColor + emissiveColor;
	
	outColor = vec4f(finalColor, 1.0);
	
	
	
	vec3f colors[4] = vec3f[](vec3f(0, 1, 0), vec3f(0, 0, 1), vec3f(1, 1, 0), vec3f(1, 0, 0));
	
	vec4f posViewSpace = (frameData.view * vec4f(pos, 1.0));
	
	uint cascadeIndex = uint(shadowData.count) - uint(1);
	for(uint i = uint(0); i < shadowData.count; i++)
	{
		if(-posViewSpace.z < shadowData.depth[i])
		{	
			cascadeIndex = i;
			break;
		}
	}
	
	outColor = vec4f(finalColor + colors[int(cascadeIndex % 4)] * 0.15, 1.0);
	outColor = vec4f(finalColor, 1.0);
}
)";

	struct FrameLayout
	{
		FrameLayout(StructLayout structLayout) : layout(structLayout)
		{
			/*struct FrameData
			{
				vec3f cameraPosition;
				mat4f view;
			}*/

			cameraPositionOffset = layout.add(BufferElementType::Float3);
			viewOffset = layout.addMatrix(BufferElementType::Float, 4, 4);
		}

		BufferLayout layout;

		size_t cameraPositionOffset;
		size_t viewOffset;
	};

	struct ShadowLayout
	{
		ShadowLayout(StructLayout structLayout) : layout(structLayout)
		{
			/*struct ShadowData
			{
				uint count;
				mat4f viewProjection[MaxShadowCascadeCount];
				float depth[MaxShadowCascadeCount];
				float depthBias[MaxShadowCascadeCount];
			}*/

			countOffset = layout.add(BufferElementType::UInt);
			viewProjectionOffset = layout.addMatrixArray(BufferElementType::Float, 4, 4, true, MaxShadowCascadeCount);
			depthOffset = layout.addArray(BufferElementType::Float, MaxShadowCascadeCount);
			depthBiasOffset = layout.addArray(BufferElementType::Float, MaxShadowCascadeCount);
		}

		BufferLayout layout;

		size_t countOffset;
		size_t viewProjectionOffset;
		size_t depthOffset;
		size_t depthBiasOffset;
	};

	struct LightLayout
	{
		LightLayout(StructLayout structLayout) : layout(structLayout)
		{
			/*struct LightData
			{
				vec3f direction;
				vec3f color;
				float ambientStrength;
			}*/
			
			directionOffset = layout.add(BufferElementType::Float3);
			colorOffset = layout.add(BufferElementType::Float3);
			ambientStrengthOffset = layout.add(BufferElementType::Float);
		}

		BufferLayout layout;

		size_t directionOffset;
		size_t colorOffset;
		size_t ambientStrengthOffset;
	};
}

PhongLightingPass::PhongLightingPass() :
	AbstractRenderPass()
{
	auto& graphics = Graphics::instance();

	if (!graphics.uberShaderExists(ShaderName))
		graphics.setUberShader(ShaderName, ShaderCode);

	// GBuffer layout
	{
		DescriptorSetLayout::Settings descriptorSetLayoutSettings;
		descriptorSetLayoutSettings.bindings =
		{
			//TODO: Support different GBuffer formats
			// GBuffer
			{ DescriptorType::CombinedImageSampler, 0, 1, ShaderStage::Fragment },
			{ DescriptorType::CombinedImageSampler, 1, 1, ShaderStage::Fragment },
			{ DescriptorType::CombinedImageSampler, 2, 1, ShaderStage::Fragment },
			{ DescriptorType::CombinedImageSampler, 3, 1, ShaderStage::Fragment },
			// Shadow map
			{ DescriptorType::CombinedImageSampler, 4, 1, ShaderStage::Fragment }
		};
		descriptorSetLayoutSettings.pageSize = Renderer::FramesInFlight;

		m_gbufferLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);
	}

	// Phong layout
	{
		DescriptorSetLayout::Settings descriptorSetLayoutSettings;
		descriptorSetLayoutSettings.bindings =
		{
			{ DescriptorType::UniformBuffer, FrameBindingIndex, 1, ShaderStage::Fragment },
			{ DescriptorType::UniformBuffer, ShadowBindingIndex, 1, ShaderStage::Fragment },
			{ DescriptorType::UniformBuffer, LightBindingIndex, 1, ShaderStage::Fragment }
		};
		descriptorSetLayoutSettings.pageSize = Renderer::FramesInFlight;

		m_phongLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);
	}

	// Frame buffer
	{
		FrameLayout layout(StructLayout::Default);

		m_frameBuffer = Buffer::create({ BufferUsage::Uniform | BufferUsage::TransferDst, layout.layout.getSize() });
	}

	// Shadow buffer
	{
		ShadowLayout layout(StructLayout::Default);

		m_shadowBuffer = Buffer::create({ BufferUsage::Uniform | BufferUsage::TransferDst, layout.layout.getSize() });
	}

	// Light buffer
	{
		LightLayout layout(StructLayout::Default);

		m_lightBuffer = Buffer::create({ BufferUsage::Uniform | BufferUsage::TransferDst, layout.layout.getSize() });
	}

	// Phong set
	m_phongSet = m_phongLayout->createSet();

	m_phongSet->update(FrameBindingIndex, *m_frameBuffer);
	m_phongSet->update(ShadowBindingIndex, *m_shadowBuffer);
	m_phongSet->update(LightBindingIndex, *m_lightBuffer);

	// Graphics pipeline
	std::vector<UberShader::Option> uberOptions =
	{
		{ "MaxShadowMapCascadeCount", ConstantValue(static_cast<uint32_t>(MaxShadowCascadeCount)) }
	};

	GraphicsPipeline::Settings pipelineSettings;
	pipelineSettings.vertexShader = graphics.getShader(*graphics.getUberShaderFromString(std::string(ShaderName), AstShaderStage::Vertex, uberOptions));
	pipelineSettings.fragmentShader = graphics.getShader(*graphics.getUberShaderFromString(std::string(ShaderName), AstShaderStage::Fragment, uberOptions));
	pipelineSettings.descriptorSetLayouts.resize(2);
	pipelineSettings.descriptorSetLayouts[GBufferSetIndex] = m_gbufferLayout;
	pipelineSettings.descriptorSetLayouts[PhongSetIndex] = m_phongLayout;
	pipelineSettings.state.vertexInput.inputs = Vertex_XYZ_UV::getVertexInput();
	pipelineSettings.state.depth.test = false;
	pipelineSettings.state.depth.write = false;
	pipelineSettings.state.stencil = true;
	pipelineSettings.state.stencilFront.compareOperation = CompareOperation::Equal;
	pipelineSettings.state.stencilFront.compareMask = 1;
	pipelineSettings.state.stencilFront.reference = 1;

	m_pipeline = GraphicsPipeline::create(pipelineSettings);

	m_gbufferSampler = graphics.getSampler(Sampler::Settings(SamplerFilter::Nearest));

	Sampler::Settings samplerSettings(SamplerFilter::Linear);
	samplerSettings.addressModeU = SamplerAddressMode::ClampToBorder;
	samplerSettings.addressModeV = SamplerAddressMode::ClampToBorder;
	samplerSettings.addressModeW = SamplerAddressMode::ClampToBorder;
	samplerSettings.borderColor = SamplerBorderColor::WhiteFloat;
	samplerSettings.enableCompare = true;
	samplerSettings.compareOperation = CompareOperation::Less;

	m_shadowMapSampler = graphics.getSampler(samplerSettings);

	m_quadMesh = graphics.getQuadMesh();
}

const char* PhongLightingPass::getName() const noexcept
{
	return "Phong Lighting";
}

void PhongLightingPass::setShadowData(const std::vector<ShadowCascadeData>& shadowData)
{
	m_shadowCascadeDatas = shadowData;
}

void PhongLightingPass::setLightData(const Vector3f& direction, const Color& color, float ambientStrength)
{
	m_lightDirection = direction;
	m_lightColor = color;
	m_lightAmbientStrength = ambientStrength;
}

FrameGraphPass& PhongLightingPass::addToFrameGraph(FrameGraphBuilder& frameGraphBuilder, const Settings& settings)
{
	auto& pass = frameGraphBuilder.createPass(getName());

	if (settings.outputClearValue.has_value())
		pass.addOutputTexture(settings.output, 0, settings.outputClearValue.value());
	else
		pass.addOutputTexture(settings.output, 0);

	for (const auto& texture : settings.gbuffer)
		pass.addSampledTexture(texture, ShaderStage::Fragment);

	for (const auto& texture : settings.shadowMaps)
		pass.addSampledTexture(texture, ShaderStage::Fragment);

	if (settings.depthStencilClearValue.has_value())
		pass.setDepthTexture(settings.depthStencil, settings.depthStencilClearValue.value());
	else
		pass.setDepthTexture(settings.depthStencil);

	Settings settingsCopy = settings;

	pass.setExecutionCallback([this, settingsCopy](FrameGraphContext& context)
		{
			execute(context, settingsCopy);
		});

	return pass;
}

void PhongLightingPass::updateResources(RenderFrame& renderFrame, CommandBuffer& commandBuffer)
{
	const size_t shadowMapCount = std::min(MaxShadowCascadeCount, m_shadowCascadeDatas.size());

	commandBuffer.memoryBarrier(MemoryBarrier::TransferBegin);

	{
		const FrameLayout layout(StructLayout::Default);

		auto stagingBuffer = renderFrame.createStagingBuffer(layout.layout.getSize());

		auto data = stagingBuffer.map();

		mapMemory<Vector3f>(data, layout.cameraPositionOffset) = getRenderData().getCamera().getPosition();
		mapMemory<Matrix4f>(data, layout.viewOffset) = getRenderData().getCamera().getViewMatrix();

		stagingBuffer.unmap();

		commandBuffer.copyBuffer(*stagingBuffer.buffer, *m_frameBuffer, stagingBuffer.size, stagingBuffer.offset);
	}

	{
		const ShadowLayout layout(StructLayout::Default);

		auto stagingBuffer = renderFrame.createStagingBuffer(layout.layout.getSize());

		auto data = stagingBuffer.map();

		mapMemory<uint32_t>(data, layout.countOffset) = static_cast<uint32_t>(shadowMapCount);
		MemoryMapper viewProjections(data, layout.viewProjectionOffset, sizeof(Matrix4f));
		MemoryMapper depths(data, layout.depthOffset, layout.layout.getArrayAlignment(BufferElementType::Float));
		MemoryMapper depthBiases(data, layout.depthBiasOffset, layout.layout.getArrayAlignment(BufferElementType::Float));

		for (size_t i = 0; i < shadowMapCount; i++)
		{
			viewProjections.map<Matrix4f>(i) = m_shadowCascadeDatas[i].viewProjection;
			depths.map<float>(i) = m_shadowCascadeDatas[i].depth;
			depthBiases.map<float>(i) = m_shadowCascadeDatas[i].depthBias;
		}

		stagingBuffer.unmap();

		commandBuffer.copyBuffer(*stagingBuffer.buffer, *m_shadowBuffer, stagingBuffer.size, stagingBuffer.offset);
	}

	{
		const LightLayout layout(StructLayout::Default);

		auto stagingBuffer = renderFrame.createStagingBuffer(layout.layout.getSize());

		auto data = stagingBuffer.map();

		mapMemory<Vector3f>(data, layout.directionOffset) = m_lightDirection;
		mapMemory<Vector3f>(data, layout.colorOffset) = m_lightColor.toVector3f();
		mapMemory<float>(data, layout.ambientStrengthOffset) = m_lightAmbientStrength;

		stagingBuffer.unmap();

		commandBuffer.copyBuffer(*stagingBuffer.buffer, *m_lightBuffer, stagingBuffer.size, stagingBuffer.offset);
	}

	commandBuffer.memoryBarrier(MemoryBarrier::TransferEnd);
}

void PhongLightingPass::execute(FrameGraphContext& context, const Settings& settings)
{
	const auto& renderData = getRenderData();

	if (!renderData.isValid())
		return;

	const auto& viewport = getRenderData().getCamera().getViewport();
	const auto& scissor = getRenderData().getCamera().getScissor();

	auto gbufferSet = m_gbufferLayout->createSet();

	for (uint32_t i = 0; i < settings.gbuffer.size(); i++)
		gbufferSet->update(i, *context.getImageView(settings.gbuffer[i]), *m_gbufferSampler);
	gbufferSet->update(static_cast<uint32_t>(settings.gbuffer.size()), *settings.shadowMap->getView(), *m_shadowMapSampler);

	auto& commandBuffer = context.getCommandBuffer();

	commandBuffer.bindPipeline(*m_pipeline);

	commandBuffer.setViewport(viewport);

	commandBuffer.setScissor(scissor.pos, scissor.size);

	commandBuffer.bindDescriptorSet(GBufferSetIndex, *gbufferSet);
	commandBuffer.bindDescriptorSet(PhongSetIndex, *m_phongSet);

	commandBuffer.bindVertexBuffer(*m_quadMesh->getBuffer(), 0);

	commandBuffer.draw(static_cast<uint32_t>(m_quadMesh->getSize()));

	context.destroyAfterUse(std::move(gbufferSet));
}

void PhongLightingPass::doBeginFrame()
{
}

void PhongLightingPass::doEndFrame()
{
}
