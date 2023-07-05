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
	constexpr uint32_t GBufferSetIndex = 0;
	constexpr uint32_t FrameSetIndex = 1;
	constexpr uint32_t LightSetIndex = 2;
	constexpr uint32_t ShadowSetIndex = 3;

	constexpr char* ShaderName = "AtemaPhongLightingPass";

	constexpr char ShaderCode[] = R"(
option
{
	bool UseBlinnPhong = true;
	bool EnableShadows = true;
	bool ShowDebugCascades = false;
	uint MaxShadowMapCascadeCount = 16;
}

include Atema.GBufferRead;

struct FrameData
{
	vec3f cameraPosition;
	mat4f view;
}

struct LightData
{
	vec3f direction;
	vec3f color;
	float ambientStrength;
	float diffuseStrength;
}

struct CascadedShadowData
{
	uint count;
	mat4f viewProjection[MaxShadowMapCascadeCount];
	float depth[MaxShadowMapCascadeCount];
	float depthBias[MaxShadowMapCascadeCount];
}

external
{
	[set(1), binding(0)] FrameData frameData;

	[set(2), binding(0)] LightData lightData;
	
	[optional(EnableShadows)]
	[set(3), binding(0)] CascadedShadowData shadowData;

	[optional(EnableShadows)]
	[set(3), binding(1)] sampler2DArray shadowMap;
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

uint getCascadeIndex(vec3f worldPos)
{
	vec4f posViewSpace = (frameData.view * vec4f(worldPos, 1.0));
	
	uint cascadeIndex = uint(shadowData.count) - uint(1);
	for(uint i = uint(0); i < shadowData.count; i++)
	{
		if(-posViewSpace.z < shadowData.depth[i])
		{	
			cascadeIndex = i;
			break;
		}
	}
	
	return cascadeIndex;
}

float sampleVisibility(vec2f uv, uint cascadeIndex, float shadowZ)
{
	float shadowMapZ = sample(shadowMap, vec3f(uv, cascadeIndex)).r;
	
	if (shadowMapZ < shadowZ)
		return 0.0;
	
	return 1.0;
}

float getVisibility(vec3f worldPos, float angle)
{
	optional (!EnableShadows)
		return 1.0;
	
	optional (EnableShadows)
	{
		uint cascadeIndex = getCascadeIndex(worldPos);
		
		float cascadeBias = clamp(shadowData.depthBias[cascadeIndex] * tan(angle), 0.0, 0.5);
		
		vec4f shadowCoord = (shadowData.viewProjection[cascadeIndex] * vec4f(worldPos, 1.0));
		shadowCoord.z = clamp(shadowCoord.z, -1.0, 1.0);
		
		vec2f shadowMapUV = shadowCoord.xy * 0.5 + 0.5;
		float shadowZ = shadowCoord.z - cascadeBias;
		
		return sampleVisibility(shadowMapUV, cascadeIndex, shadowZ);
	}
}

[entry(fragment)]
void main()
{
	vec3f worldPos = atGBufferReadPosition(inTexCoords);
	vec3f normal = atGBufferReadNormal(inTexCoords);
	vec3f color = atGBufferReadAlbedo(inTexCoords);
	float ao = atGBufferReadAO(inTexCoords);
	float metalness = atGBufferReadMetalness(inTexCoords);
	int roughness = int(atGBufferReadRoughness(inTexCoords) * 255.0);
	vec3f emissiveColor = atGBufferReadEmissive(inTexCoords);
	
	vec3f inverseLightDir = -normalize(lightData.direction);
	
	float cosTheta = dot(normal, inverseLightDir);
	
	//TODO: Add shininess/specular to material data
	const float Pi = 3.14159265;
	const float shininess = 16.0;
	const float specular = metalness;

	// Ambient
	vec3f ambientColor = lightData.ambientStrength * lightData.color * ao;
	
	// Diffuse
	float diffuseFactor = max(cosTheta, 0.0);
	
	vec3f diffuseColor = diffuseFactor * lightData.diffuseStrength * lightData.color;
	
	// Specular
	vec3f viewDir = normalize(frameData.cameraPosition - worldPos);
	
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
	
	vec3f specularColor = specularFactor * specular * lightData.color;
	
	// Visibility
	float visibility = getVisibility(worldPos, acos(cosTheta));
	
	// Result
	vec3f finalColor = (ambientColor + visibility * (diffuseColor + specularColor)) * color.rgb;
	finalColor = finalColor + emissiveColor;
	
	// Debug
	optional (ShowDebugCascades)
	{
		vec3f colors[4] = vec3f[](vec3f(0, 1, 0), vec3f(0, 0, 1), vec3f(1, 1, 0), vec3f(1, 0, 0));
		
		uint cascadeIndex = getCascadeIndex(worldPos);
		
		finalColor = finalColor + colors[int(cascadeIndex % 4)] * 0.15;
	}
	
	// Output
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
}

PhongLightingPass::PhongLightingPass()
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
			{ DescriptorType::UniformBuffer, 0, 1, ShaderStage::Fragment }
		};
		descriptorSetLayoutSettings.pageSize = Renderer::FramesInFlight;

		m_phongLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);
	}

	// Buffers and descriptor sets (one per frame in flight)
	{
		FrameLayout frameLayout(StructLayout::Default);

		Buffer::Settings frameBufferSettings;
		frameBufferSettings.usages = BufferUsage::Uniform | BufferUsage::Map;
		frameBufferSettings.byteSize = frameLayout.layout.getSize();

		for (auto& frameData : m_frameDatas)
		{
			frameData.frameBuffer = Buffer::create(frameBufferSettings);

			frameData.descriptorSet = m_phongLayout->createSet();
			frameData.descriptorSet->update(0, *frameData.frameBuffer);
		}
	}

	// Graphics pipeline
	GraphicsPipeline::Settings pipelineSettings;
	pipelineSettings.state.vertexInput.inputs = Vertex_XYZ_UV::getVertexInput();
	pipelineSettings.state.depth.test = false;
	pipelineSettings.state.depth.write = false;
	pipelineSettings.state.stencil = true;
	pipelineSettings.state.stencilFront.compareOperation = CompareOperation::Equal;
	pipelineSettings.state.stencilFront.compareMask = 1;
	pipelineSettings.state.stencilFront.reference = 1;
	pipelineSettings.state.colorBlend.enabled = true;
	pipelineSettings.state.colorBlend.colorSrcFactor = BlendFactor::One;
	pipelineSettings.state.colorBlend.colorDstFactor = BlendFactor::One;

	pipelineSettings.descriptorSetLayouts =
	{
		m_gbufferLayout,
		m_phongLayout,
		Graphics::instance().getLightLayout()
	};

	// Light pipeline
	{
		std::vector<UberShader::Option> uberOptions =
		{
			{ "UseBlinnPhong", ConstantValue(false) },
			{ "EnableShadows", ConstantValue(false) }
		};

		pipelineSettings.vertexShader = graphics.getShader(*graphics.getUberShaderFromString(std::string(ShaderName), AstShaderStage::Vertex, uberOptions));
		pipelineSettings.fragmentShader = graphics.getShader(*graphics.getUberShaderFromString(std::string(ShaderName), AstShaderStage::Fragment, uberOptions));
		m_lightPipeline = GraphicsPipeline::create(pipelineSettings);
	}

	// Light & Shadow pipeline
	{
		std::vector<UberShader::Option> uberOptions =
		{
			{ "UseBlinnPhong", ConstantValue(false) },
			{ "EnableShadows", ConstantValue(true) },
			{ "ShowDebugCascades", ConstantValue(false) },
			{ "MaxShadowMapCascadeCount", ConstantValue(static_cast<uint32_t>(CascadedShadowData::MaxCascadeCount)) }
		};

		pipelineSettings.vertexShader = graphics.getShader(*graphics.getUberShaderFromString(std::string(ShaderName), AstShaderStage::Vertex, uberOptions));
		pipelineSettings.fragmentShader = graphics.getShader(*graphics.getUberShaderFromString(std::string(ShaderName), AstShaderStage::Fragment, uberOptions));
		pipelineSettings.descriptorSetLayouts.emplace_back(Graphics::instance().getLightShadowLayout());

		m_lightShadowPipeline = GraphicsPipeline::create(pipelineSettings);
	}

	m_gbufferSampler = graphics.getSampler(Sampler::Settings(SamplerFilter::Nearest));

	m_quadMesh = graphics.getQuadMesh();
}

const char* PhongLightingPass::getName() const noexcept
{
	return "Phong Lighting";
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
	auto& frameData = m_frameDatas[renderFrame.getFrameIndex()];

	{
		const FrameLayout layout(StructLayout::Default);
		
		auto data = frameData.frameBuffer->map();

		mapMemory<Vector3f>(data, layout.cameraPositionOffset) = getRenderData().getCamera().getPosition();
		mapMemory<Matrix4f>(data, layout.viewOffset) = getRenderData().getCamera().getViewMatrix();

		frameData.frameBuffer->unmap();
	}
}

void PhongLightingPass::execute(FrameGraphContext& context, const Settings& settings)
{
	const auto& renderData = getRenderData();

	if (!renderData.isValid())
		return;

	auto& renderLights = renderData.getRenderLights();

	const auto& frameData = m_frameDatas[context.getFrameIndex()];

	const auto& viewport = getRenderData().getCamera().getViewport();
	const auto& scissor = getRenderData().getCamera().getScissor();

	auto& commandBuffer = context.getCommandBuffer();

	commandBuffer.setViewport(viewport);

	commandBuffer.setScissor(scissor.pos, scissor.size);

	auto gbufferSet = m_gbufferLayout->createSet();

	for (uint32_t i = 0; i < settings.gbuffer.size(); i++)
		gbufferSet->update(i, *context.getImageView(settings.gbuffer[i]), *m_gbufferSampler);

	commandBuffer.bindPipeline(*m_lightPipeline);

	commandBuffer.bindDescriptorSet(GBufferSetIndex, *gbufferSet);
	commandBuffer.bindDescriptorSet(FrameSetIndex, *frameData.descriptorSet);

	commandBuffer.bindVertexBuffer(*m_quadMesh->getBuffer(), 0);

	for (auto& renderLight : renderLights)
	{
		if (renderLight->getLight().castShadows())
			continue;

		commandBuffer.bindDescriptorSet(LightSetIndex, renderLight->getLightDescriptorSet());

		commandBuffer.draw(static_cast<uint32_t>(m_quadMesh->getSize()));
	}

	commandBuffer.bindPipeline(*m_lightShadowPipeline);

	for (auto& renderLight : renderLights)
	{
		if (!renderLight->getLight().castShadows())
			continue;

		commandBuffer.bindDescriptorSet(LightSetIndex, renderLight->getLightDescriptorSet());
		commandBuffer.bindDescriptorSet(ShadowSetIndex, renderLight->getShadowDescriptorSet());

		commandBuffer.draw(static_cast<uint32_t>(m_quadMesh->getSize()));
	}

	context.destroyAfterUse(std::move(gbufferSet));
}
