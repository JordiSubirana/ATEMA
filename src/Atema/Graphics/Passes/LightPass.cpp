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

#include <Atema/Graphics/Passes/LightPass.hpp>
#include <Atema/Graphics/FrameGraphBuilder.hpp>
#include <Atema/Graphics/FrameGraphContext.hpp>
#include <Atema/Graphics/GBuffer.hpp>
#include <Atema/Graphics/RenderScene.hpp>
#include <Atema/Graphics/VertexBuffer.hpp>
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/Material.hpp>
#include <Atema/Graphics/VertexTypes.hpp>
#include <Atema/Renderer/DescriptorSetLayout.hpp>
#include <Atema/Renderer/Renderer.hpp>
#include <Atema/Shader/Atsl/AtslParser.hpp>
#include <Atema/Shader/Atsl/AtslToAstConverter.hpp>

using namespace at;

namespace
{
	constexpr uint32_t InvalidGBufferBinding = std::numeric_limits<uint32_t>::max();
	constexpr uint32_t GBufferSetIndex = 0;
	constexpr uint32_t FrameSetIndex = 1;
	constexpr uint32_t LightSetIndex = 2;
	constexpr uint32_t ShadowSetIndex = 3;

	constexpr char LightShaderDefinitions[] = R"(
option
{
	bool EnableShadows = true;
	bool ShowDebugCascades = false;
	uint MaxShadowMapCascadeCount = 16;
}

include Atema.GBufferRead;

struct FrameDataStruct
{
	vec3f CameraPosition;
	mat4f View;
}

struct LightDataStruct
{
	vec3f Direction;
	vec3f Color;
	float AmbientStrength;
	float DiffuseStrength;
}

struct CascadedShadowDataStruct
{
	uint Count;
	mat4f ViewProjection[MaxShadowMapCascadeCount];
	float Depth[MaxShadowMapCascadeCount];
	float DepthBias[MaxShadowMapCascadeCount];
}

external
{
	[set(1), binding(0)] FrameDataStruct FrameData;

	[set(2), binding(0)] LightDataStruct LightData;
	
	[optional(EnableShadows)]
	[set(3), binding(0)] CascadedShadowDataStruct CascadedShadowData;

	[optional(EnableShadows)]
	[set(3), binding(1)] sampler2DArray ShadowMap;
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
	vec4f posViewSpace = (FrameData.View * vec4f(worldPos, 1.0));
	
	uint cascadeIndex = uint(CascadedShadowData.Count) - uint(1);
	for(uint i = uint(0); i < CascadedShadowData.Count; i++)
	{
		if(-posViewSpace.z < CascadedShadowData.Depth[i])
		{	
			cascadeIndex = i;
			break;
		}
	}
	
	return cascadeIndex;
}

float sampleVisibility(vec2f uv, uint cascadeIndex, float shadowZ)
{
	float shadowMapZ = sample(ShadowMap, vec3f(uv, cascadeIndex)).r;
	
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
		
		float cascadeBias = clamp(CascadedShadowData.DepthBias[cascadeIndex] * tan(angle), 0.0, 0.5);
		
		vec4f shadowCoord = (CascadedShadowData.ViewProjection[cascadeIndex] * vec4f(worldPos, 1.0));
		shadowCoord.z = clamp(shadowCoord.z, -1.0, 1.0);
		
		vec2f shadowMapUV = shadowCoord.xy * 0.5 + 0.5;
		float shadowZ = shadowCoord.z - cascadeBias;
		
		return sampleVisibility(shadowMapUV, cascadeIndex, shadowZ);
	}
}
)";

	constexpr char LightShaderMainBegin[] = R"(
[entry(fragment)]
void main()
{
	vec2f uv = inTexCoords;
	vec3f finalColor;
	uint lightingModel = uint(GBufferReadLightingModel(uv));

)";

constexpr char LightShaderMainEnd[] = R"(
	else
		discard;

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

const std::string EmissiveShaderName = "LightPassEmissive";
constexpr char EmissiveShader[] = R"(
include Atema.GBufferRead.EmissiveColor;
include Atema.PostProcess;

[entry(fragment)]
void main()
{
	vec3f emissiveColor = GBufferReadEmissiveColor(atPostProcessGetTexCoords());
	
	atPostProcessWriteOutColor(vec4f(emissiveColor, 1.0));
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

LightPass::LightPass(const GBuffer& gbuffer, const ShaderLibraryManager& shaderLibraryManager) :
	m_gbuffer(&gbuffer),
	m_shaderLibraryManager(&shaderLibraryManager),
	m_updateShader(false),
	m_useFrameSet(false),
	m_useLightSet(false),
	m_useLightShadowSet(false)
{
	auto& graphics = Graphics::instance();

	m_gbufferSampler = graphics.getSampler(Sampler::Settings(SamplerFilter::Nearest));

	m_quadMesh = graphics.getQuadMesh();

	AtslParser parser;

	AtslToAstConverter converter;

	auto ast = converter.createAst(parser.createTokens(EmissiveShader));

	if (!graphics.uberShaderExists(EmissiveShaderName))
		graphics.setUberShader(EmissiveShaderName, EmissiveShader);

	const auto bindings = gbuffer.getTextureBindings({ "EmissiveColor" });
	m_gbufferEmissiveIndex = bindings[0].index;

	RenderMaterial::Settings renderMaterialSettings;
	renderMaterialSettings.material = std::make_shared<Material>(graphics.getUberShader(EmissiveShaderName));
	renderMaterialSettings.shaderLibraryManager = m_shaderLibraryManager;
	renderMaterialSettings.pipelineState.depth.test = false;
	renderMaterialSettings.pipelineState.depth.write = false;
	renderMaterialSettings.pipelineState.colorBlend.enabled = true;
	renderMaterialSettings.pipelineState.colorBlend.colorSrcFactor = BlendFactor::One;
	renderMaterialSettings.pipelineState.colorBlend.colorDstFactor = BlendFactor::One;
	renderMaterialSettings.uberShaderOptions =
	{
		{ bindings[0].bindingOptionName, static_cast<int32_t>(0) }
	};

	m_lightEmissiveMaterial = std::make_shared<RenderMaterial>(renderMaterialSettings);
}

const char* LightPass::getName() const noexcept
{
	return "Phong Lighting";
}

FrameGraphPass& LightPass::addToFrameGraph(FrameGraphBuilder& frameGraphBuilder, const Settings& settings)
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

void LightPass::updateResources(RenderFrame& renderFrame, CommandBuffer& commandBuffer)
{
	if (!m_useFrameSet)
		return;
	
	auto& frameResources = m_frameResources[renderFrame.getFrameIndex()];

	const FrameLayout layout(StructLayout::Default);

	auto data = frameResources.buffer->map();

	mapMemory<Vector3f>(data, layout.cameraPositionOffset) = getRenderScene().getCamera().getPosition();
	mapMemory<Matrix4f>(data, layout.viewOffset) = getRenderScene().getCamera().getViewMatrix();

	frameResources.buffer->unmap();
}

void LightPass::setLightingModels(const std::vector<std::string>& lightingModelNames)
{
	for (const auto& lightingModelName : lightingModelNames)
		createLightingModel(lightingModelName);

	createShader();
}

void LightPass::execute(FrameGraphContext& context, const Settings& settings)
{
	const auto& renderScene = getRenderScene();

	if (!renderScene.isValid() || !m_lightMaterial)
		return;

	auto& renderLights = renderScene.getRenderLights();

	const auto& frameResources = m_frameResources[context.getFrameIndex()];

	const auto& viewport = getRenderScene().getCamera().getViewport();
	const auto& scissor = getRenderScene().getCamera().getScissor();

	auto& commandBuffer = context.getCommandBuffer();

	commandBuffer.setViewport(viewport);

	commandBuffer.setScissor(scissor.pos, scissor.size);

	m_lightMaterial->bindTo(commandBuffer);

	auto gbufferSet = m_lightMaterial->createSet(GBufferSetIndex);

	for (uint32_t i = 0; i < settings.gbuffer.size(); i++)
	{
		const auto bindingIndex = m_gbufferBindings[i];

		if (bindingIndex != InvalidGBufferBinding)
			gbufferSet->update(bindingIndex, *context.getImageView(settings.gbuffer[i]), *m_gbufferSampler);
	}

	commandBuffer.bindDescriptorSet(GBufferSetIndex, *gbufferSet);

	if (m_useFrameSet)
		commandBuffer.bindDescriptorSet(FrameSetIndex, *frameResources.descriptorSet);

	commandBuffer.bindVertexBuffer(*m_quadMesh->getBuffer(), 0);

	for (auto& renderLight : renderLights)
	{
		if (renderLight->getLight().castShadows())
			continue;

		if (m_useLightSet)
			commandBuffer.bindDescriptorSet(LightSetIndex, renderLight->getLightDescriptorSet());

		commandBuffer.draw(static_cast<uint32_t>(m_quadMesh->getSize()));
	}

	m_lightShadowMaterial->bindTo(commandBuffer);

	for (auto& renderLight : renderLights)
	{
		if (!renderLight->getLight().castShadows())
			continue;

		if (m_useLightSet)
			commandBuffer.bindDescriptorSet(LightSetIndex, renderLight->getLightDescriptorSet());

		if (m_useLightShadowSet)
			commandBuffer.bindDescriptorSet(ShadowSetIndex, renderLight->getShadowDescriptorSet());

		commandBuffer.draw(static_cast<uint32_t>(m_quadMesh->getSize()));
	}

	auto emissiveSet = m_lightEmissiveMaterial->createSet(0);
	emissiveSet->update(0, *context.getImageView(settings.gbuffer[m_gbufferEmissiveIndex]), *m_gbufferSampler);

	m_lightEmissiveMaterial->bindTo(commandBuffer);

	commandBuffer.bindDescriptorSet(0, *emissiveSet);

	commandBuffer.draw(static_cast<uint32_t>(m_quadMesh->getSize()));

	context.destroyAfterUse(std::move(gbufferSet));
	context.destroyAfterUse(std::move(emissiveSet));
}

void LightPass::createLightingModel(const std::string& name)
{
	if (m_lightingModels.find(name) != m_lightingModels.end())
		return;

	m_lightingModels.emplace(name);

	const auto& lightingModel = Graphics::instance().getLightingModel(name);

	const auto gbufferBindings = m_gbuffer->getTextureBindings(lightingModel);
	for (const auto& binding : gbufferBindings)
		m_gbufferOptions[binding.index] = binding.bindingOptionName;

	m_updateShader = true;
}

void LightPass::createShader()
{
	if (!m_updateShader)
		return;

	auto& graphics = Graphics::instance();

	// GBuffer bindings
	std::vector<UberShader::Option> gbufferOptions;
	gbufferOptions.reserve(m_gbufferOptions.size());

	m_gbufferBindings.clear();
	m_gbufferBindings.resize(m_gbuffer->getTextures().size(), InvalidGBufferBinding);
	uint32_t gbufferBindingIndex = 0;
	for (const auto& [gbufferTextureIndex, optionName] : m_gbufferOptions)
	{
		gbufferOptions.emplace_back(optionName, static_cast<int32_t>(gbufferBindingIndex));
		m_gbufferBindings[gbufferTextureIndex] = gbufferBindingIndex++;
	}

	std::string shader = LightShaderDefinitions;

	for (const auto& lightingModelName : m_lightingModels)
	{
		shader += "include Atema.LightingModel." + lightingModelName + "LightMaterial;\n";
	}

	shader += LightShaderMainBegin;

	bool useElse = false;

	for (const auto& lightingModelName : m_lightingModels)
	{
		const size_t lightingModelId = graphics.getLightingModelID(lightingModelName);

		shader += "\n\t";
		if (useElse)
			shader += "else ";
		shader += "if (lightingModel == uint(" + std::to_string(lightingModelId) + "))";
		shader += "\n\t\tfinalColor = get" + lightingModelName + "FinalColor(uv);";

		useElse = true;
	}

	shader += LightShaderMainEnd;

	graphics.setUberShader("GlobalLightShader", shader);
	auto uberShader = graphics.getUberShader(std::string("GlobalLightShader"));

	RenderMaterial::Settings renderMaterialSettings;
	renderMaterialSettings.material = std::make_shared<Material>(uberShader);
	renderMaterialSettings.shaderLibraryManager = m_shaderLibraryManager;
	renderMaterialSettings.pipelineState.depth.test = false;
	renderMaterialSettings.pipelineState.depth.write = false;
	renderMaterialSettings.pipelineState.colorBlend.enabled = true;
	renderMaterialSettings.pipelineState.colorBlend.colorSrcFactor = BlendFactor::One;
	renderMaterialSettings.pipelineState.colorBlend.colorDstFactor = BlendFactor::One;

	// Light material
	renderMaterialSettings.uberShaderOptions = gbufferOptions;
	renderMaterialSettings.uberShaderOptions.emplace_back("EnableShadows", ConstantValue(false));

	m_lightMaterial = std::make_shared<RenderMaterial>(renderMaterialSettings);

	// Light + shadow material
	renderMaterialSettings.uberShaderOptions = gbufferOptions;
	renderMaterialSettings.uberShaderOptions.emplace_back("EnableShadows", ConstantValue(true));
	renderMaterialSettings.uberShaderOptions.emplace_back("ShowDebugCascades", ConstantValue(false));
	renderMaterialSettings.uberShaderOptions.emplace_back("MaxShadowMapCascadeCount", ConstantValue(static_cast<uint32_t>(CascadedShadowData::MaxCascadeCount)));

	m_lightShadowMaterial = std::make_shared<RenderMaterial>(renderMaterialSettings);

	m_useFrameSet = m_lightMaterial->hasBinding("FrameData") && m_lightMaterial->getBinding("FrameData").set != RenderMaterial::InvalidBindingIndex;
	m_useLightSet = m_lightMaterial->hasBinding("LightData") && m_lightMaterial->getBinding("LightData").set != RenderMaterial::InvalidBindingIndex;
	m_useLightShadowSet = m_lightShadowMaterial->hasBinding("CascadedShadowData") && m_lightShadowMaterial->getBinding("CascadedShadowData").set != RenderMaterial::InvalidBindingIndex;
	
	// Buffers and descriptor sets (one per frame in flight)
	FrameLayout frameLayout(StructLayout::Default);

	Buffer::Settings frameBufferSettings;
	frameBufferSettings.usages = BufferUsage::Uniform | BufferUsage::Map;
	frameBufferSettings.byteSize = frameLayout.layout.getSize();

	for (auto& frameResources : m_frameResources)
	{
		m_oldResources.emplace_back(std::move(frameResources.buffer));
		m_oldResources.emplace_back(std::move(frameResources.descriptorSet));

		if (m_useFrameSet)
		{
			frameResources.buffer = Buffer::create(frameBufferSettings);
			frameResources.descriptorSet = m_lightMaterial->createSet(LightSetIndex);
			frameResources.descriptorSet->update(0, *frameResources.buffer);
		}
	}

	gbufferBindingIndex = 0;
	const auto& textures = m_gbuffer->getTextures();
	for (const auto& texture : textures)
	{
		if (!m_lightMaterial->hasBinding(texture.name))
			m_gbufferBindings[gbufferBindingIndex] = InvalidGBufferBinding;

		gbufferBindingIndex++;
	}

	m_updateShader = false;
}
