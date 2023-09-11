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

#include <Atema/Core/TaskManager.hpp>
#include <Atema/Graphics/Passes/LightPass.hpp>
#include <Atema/Graphics/FrameGraphBuilder.hpp>
#include <Atema/Graphics/FrameGraphContext.hpp>
#include <Atema/Graphics/GBuffer.hpp>
#include <Atema/Graphics/RenderScene.hpp>
#include <Atema/Graphics/VertexBuffer.hpp>
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/Material.hpp>
#include <Atema/Graphics/PointLight.hpp>
#include <Atema/Graphics/Primitive.hpp>
#include <Atema/Graphics/SpotLight.hpp>
#include <Atema/Graphics/VertexTypes.hpp>
#include <Atema/Graphics/Loaders/ModelLoader.hpp>
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

	constexpr uint32_t StencilFrameSetIndex = 0;
	constexpr uint32_t StencilLightSetIndex = 1;

	constexpr size_t LightSphereSubdivisions = 4;
	constexpr size_t LightConeVerticalSubdivisions = 8;
	constexpr size_t LightConeHorizontalSubdivisions = 1;

	const std::string StencilShaderName = "LightPassStencil";
	constexpr char StencilShader[] = R"(
const uint DirectionalLightType = uint(0);
const uint PointLightType = uint(1);
const uint SpotLightType = uint(2);

struct FrameDataStruct
{
	mat4f Projection;
	mat4f View;
	vec3f CameraPosition;
	vec2u ScreenSize;
}

struct LightDataStruct
{
	uint Type;
	mat4f Transform;
	vec3f Color;
	float Intensity;
	float IndirectIntensity;
	vec4f Parameter0;
	vec4f Parameter1;
}

external
{
	[set(0), binding(0)] FrameDataStruct FrameData;

	[set(1), binding(0)] LightDataStruct LightData;
}

[stage(vertex)]
input
{
	[location(0)] vec3f inPosition;
	[location(1)] vec2f inTexCoords;
}

[entry(vertex)]
void main()
{
	vec4f worldPosition = LightData.Transform * vec4f(inPosition, 1.0);
	vec4f screenPosition = worldPosition;
	if (LightData.Type != DirectionalLightType)
		screenPosition = FrameData.Projection * FrameData.View * worldPosition;

	setVertexPosition(screenPosition);
}

[stage(fragment)]
output
{
	[location(0)] vec4f outColor;
}

[entry(fragment)]
void main()
{
	//TODO: Remove this trick to make the material work
	outColor = vec4f(0, 0, 0, 0) * LightData.IndirectIntensity * FrameData.CameraPosition.x;
}
)";

	constexpr char LightShaderDefinitions[] = R"(
option
{
	bool EnableShadows = true;
	bool ShowDebugCascades = false;
	uint MaxShadowMapCascadeCount = 16;
}

const uint DirectionalLightType = uint(0);
const uint PointLightType = uint(1);
const uint SpotLightType = uint(2);

include Atema.GBufferRead;

struct FrameDataStruct
{
	mat4f Projection;
	mat4f View;
	vec3f CameraPosition;
	vec2u ScreenSize;
}

struct LightDataStruct
{
	uint Type;
	mat4f Transform;
	vec3f Color;
	float Intensity;
	float IndirectIntensity;
	vec4f Parameter0;
	vec4f Parameter1;
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

[entry(vertex)]
void main()
{
	vec4f worldPosition = LightData.Transform * vec4f(inPosition, 1.0);
	vec4f screenPosition = worldPosition;
	if (LightData.Type != DirectionalLightType)
		screenPosition = FrameData.Projection * FrameData.View * worldPosition;

	setVertexPosition(screenPosition);
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
	vec2f uv = getFragmentCoordinates().xy / FrameData.ScreenSize;
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
	outColor = vec4f(finalColor * 0.01 + vec3f(uv, 0), 1.0);
	outColor = vec4f(finalColor, 1.0);
}
)";

const std::string EmissiveShaderName = "LightPassEmissive";
constexpr char EmissiveShader[] = R"(
include Atema.GBufferRead.EmissiveColor;
include Atema.PostProcess;

vec4f getPostProcessColor(vec2f uv)
{
	return vec4f(GBufferReadEmissiveColor(uv), 1.0);
}
)";
}

LightPass::LightPass(RenderResourceManager& resourceManager, const GBuffer& gbuffer, const ShaderLibraryManager& shaderLibraryManager, size_t threadCount) :
	m_resourceManager(&resourceManager),
	m_gbuffer(&gbuffer),
	m_shaderLibraryManager(&shaderLibraryManager),
	m_updateShader(false),
	m_useFrameSet(false),
	m_useLightSet(false),
	m_useLightShadowSet(false),
	m_gbufferSet(nullptr),
	m_emissiveSet(nullptr),
	m_iblFrameSet(nullptr),
	m_iblEnvironmentSet(nullptr)
{
	const auto& taskManager = TaskManager::instance();
	const auto maxThreadCount = taskManager.getSize();
	m_threadCount = threadCount;
	if (threadCount == 0 || threadCount > maxThreadCount)
		m_threadCount = maxThreadCount;

	auto& graphics = Graphics::instance();

	m_gbufferSampler = graphics.getSampler(Sampler::Settings(SamplerFilter::Nearest));
	m_iblSampler = graphics.getSampler(Sampler::Settings(SamplerFilter::Linear));

	m_quadMesh = graphics.getQuadMesh();

	ModelLoader::Settings settings(VertexFormat::create(DefaultVertexFormat::XYZ_UV));

	m_sphereMesh = Primitive::createUVSphere(settings, 1.5f, LightSphereSubdivisions, LightSphereSubdivisions);
	m_coneMesh = Primitive::createConeFromRadius(settings, Vector3f(0.0f, 0.0f, -1.0f), 1.0f, 1.0f,
												LightConeVerticalSubdivisions, LightConeHorizontalSubdivisions);

	// Stencil pass
	{
		if (!graphics.uberShaderExists(StencilShaderName))
			graphics.setUberShader(StencilShaderName, StencilShader);

		RenderMaterial::Settings renderMaterialSettings;
		renderMaterialSettings.material = std::make_shared<Material>(graphics.getUberShader(StencilShaderName));
		renderMaterialSettings.shaderLibraryManager = m_shaderLibraryManager;
		renderMaterialSettings.pipelineState.rasterization.cullMode = CullMode::None;
		renderMaterialSettings.pipelineState.depth.test = true;
		renderMaterialSettings.pipelineState.depth.write = false;
		renderMaterialSettings.pipelineState.stencil = true;
		renderMaterialSettings.pipelineState.stencilFront.depthFailOperation = StencilOperation::DecrementAndClamp;
		renderMaterialSettings.pipelineState.stencilFront.writeMask = 0xFF;
		renderMaterialSettings.pipelineState.stencilBack.depthFailOperation = StencilOperation::IncrementAndClamp;
		renderMaterialSettings.pipelineState.stencilBack.writeMask = 0xFF;

		m_meshStencilMaterial = std::make_shared<RenderMaterial>(*m_resourceManager, renderMaterialSettings);
	}

	// Emissive pass
	{
		if (!graphics.uberShaderExists(EmissiveShaderName))
			graphics.setUberShader(EmissiveShaderName, EmissiveShader);

		const auto bindings = gbuffer.getTextureBindings({"EmissiveColor"});
		m_gbufferEmissiveIndex = bindings[0].index;

		RenderMaterial::Settings renderMaterialSettings;
		renderMaterialSettings.material = std::make_shared<Material>(graphics.getUberShader(EmissiveShaderName));
		renderMaterialSettings.shaderLibraryManager = m_shaderLibraryManager;
		renderMaterialSettings.pipelineState.depth.test = true;
		renderMaterialSettings.pipelineState.depth.write = false;
		renderMaterialSettings.pipelineState.colorBlend.enabled = true;
		renderMaterialSettings.pipelineState.colorBlend.colorSrcFactor = BlendFactor::One;
		renderMaterialSettings.pipelineState.colorBlend.colorDstFactor = BlendFactor::One;
		renderMaterialSettings.uberShaderOptions =
		{
			{bindings[0].bindingOptionName, static_cast<int32_t>(0)}
		};

		m_lightEmissiveMaterial = std::make_shared<RenderMaterial>(*m_resourceManager, renderMaterialSettings);
	}
}

const char* LightPass::getName() const noexcept
{
	return "Light";
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

	pass.setDepthTexture(settings.gbufferDepthStencil);

	for (const auto& texture : settings.shadowMaps)
		pass.addSampledTexture(texture, ShaderStage::Fragment);

	if (settings.depthStencilClearValue.has_value())
		pass.setDepthTexture(settings.depthStencil, settings.depthStencilClearValue.value());
	else
		pass.setDepthTexture(settings.depthStencil);

	for (auto& texture : settings.environmentTextures)
		pass.addSampledTexture(texture, ShaderStage::Fragment);

	pass.enableSecondaryCommandBuffers(m_threadCount != 1);

	Settings settingsCopy = settings;

	pass.setExecutionCallback([this, settingsCopy](FrameGraphContext& context)
		{
			execute(context, settingsCopy);
		});

	return pass;
}

void LightPass::updateResources(CommandBuffer& commandBuffer)
{
	if (!m_useFrameSet)
		return;
	
	const auto& camera = getRenderScene().getCamera();
	
	FrameData frameData;
	frameData.cameraPosition = camera.getPosition();
	frameData.view = camera.getViewMatrix();
	frameData.projection = camera.getProjectionMatrix();
	frameData.screenSize = camera.getScissor().size;

	void* data = m_resourceManager->mapBuffer(*m_frameDataBuffer);
	
	frameData.copyTo(data);

	for (auto& iblMaterial : m_iblMaterials)
		iblMaterial->update();
}

void LightPass::setLightingModels(const std::vector<std::string>& lightingModelNames)
{
	for (const auto& lightingModelName : lightingModelNames)
		createLightingModel(lightingModelName);

	createShaders();
}

void LightPass::execute(FrameGraphContext& context, const Settings& settings)
{
	const auto& renderScene = getRenderScene();

	if (!renderScene.isValid() || !m_meshMaterial)
		return;

	auto& commandBuffer = context.getCommandBuffer();

	// Update GBuffer set
	auto gbufferSet = m_meshMaterial->createSet(GBufferSetIndex);

	for (uint32_t i = 0; i < settings.gbuffer.size(); i++)
	{
		const auto bindingIndex = m_gbufferBindings[i];

		if (bindingIndex != InvalidGBufferBinding)
			gbufferSet->update(bindingIndex, *context.getImageView(settings.gbuffer[i]), *m_gbufferSampler);
	}

	m_gbufferSet = gbufferSet.get();

	// Update emissive set
	auto emissiveSet = m_lightEmissiveMaterial->createSet(0);
	emissiveSet->update(0, *context.getImageView(settings.gbuffer[m_gbufferEmissiveIndex]), *m_gbufferSampler);

	m_emissiveSet = emissiveSet.get();

	// Update IBL sets
	if (!m_iblMaterials.empty() && renderScene.getSkyBox())
	{
		const auto& skyBox = renderScene.getSkyBox();

		auto iblFrameSet = m_iblMaterials[0]->createSet(1);
		iblFrameSet->update(0, m_frameDataBuffer->getBuffer(), m_frameDataBuffer->getOffset(), m_frameDataBuffer->getSize());

		m_iblFrameSet = iblFrameSet.get();

		auto iblEnvironmentSet = m_iblMaterials[0]->createSet(2);
		iblEnvironmentSet->update(0, *skyBox->irradianceMap->getView(), *m_iblSampler);
		iblEnvironmentSet->update(1, *skyBox->prefilteredMap->getView(), *m_iblSampler);

		m_iblEnvironmentSet = iblEnvironmentSet.get();

		context.destroyAfterUse(std::move(iblFrameSet));
		context.destroyAfterUse(std::move(iblEnvironmentSet));
	}

	if (m_threadCount == 1)
	{
		drawElements(commandBuffer, true, 0, m_directionalLights.size(), 0, m_pointLights.size(), 0, m_spotLights.size());
	}
	else
	{
		const size_t directionalSize = m_directionalLights.size();
		const size_t pointSize = m_pointLights.size();
		const size_t spotSize = m_spotLights.size();
		const size_t elementSize = directionalSize + pointSize + spotSize;
		const size_t pointBegin = directionalSize;
		const size_t spotBegin = pointBegin + pointSize;

		// At least one thread to apply emissive lighting
		const size_t threadCount = std::max(std::min(elementSize, m_threadCount), static_cast<size_t>(1));

		auto& taskManager = TaskManager::instance();
		std::vector<Ptr<Task>> tasks;
		tasks.reserve(threadCount);

		std::vector<Ptr<CommandBuffer>> commandBuffers;
		commandBuffers.resize(threadCount);

		size_t firstIndex = 0;
		size_t size = elementSize / threadCount;

		for (size_t taskIndex = 0; taskIndex < threadCount; taskIndex++)
		{
			bool applyPostProcess = false;

			if (taskIndex == threadCount - 1)
			{
				const auto remainingSize = elementSize - threadCount * size;

				size += remainingSize;

				applyPostProcess = true;
			}

			size_t directionalIndex = 0;
			size_t directionalCount = 0;

			size_t pointIndex = 0;
			size_t pointCount = 0;

			size_t spotIndex = 0;
			size_t spotCount = 0;

			if (firstIndex < pointBegin)
			{
				directionalIndex = firstIndex;
				directionalCount = std::min(size, directionalSize - directionalIndex);

				size_t remainingSize = size - directionalCount;

				if (remainingSize > 0)
				{
					pointIndex = 0;
					pointCount = std::min(remainingSize, pointSize - pointIndex);

					remainingSize -= pointCount;

					if (remainingSize > 0)
					{
						spotIndex = 0;
						spotCount = remainingSize;
					}
				}
			}
			else if (firstIndex < spotBegin)
			{
				pointIndex = firstIndex - pointBegin;
				pointCount = std::min(size, pointSize - pointIndex);

				const size_t remainingSize = size - pointCount;

				if (remainingSize > 0)
				{
					spotIndex = 0;
					spotCount = remainingSize;
				}
			}
			else
			{
				spotIndex = firstIndex - spotBegin;
				spotCount = size;
			}

			auto task = taskManager.createTask([this, &context, &commandBuffers, taskIndex, applyPostProcess, directionalIndex, directionalCount, pointIndex, pointCount, spotIndex, spotCount](size_t threadIndex)
				{
					auto commandBuffer = context.createSecondaryCommandBuffer(threadIndex);

					drawElements(*commandBuffer, applyPostProcess, directionalIndex, directionalCount, pointIndex, pointCount, spotIndex, spotCount);

					commandBuffer->end();

					commandBuffers[taskIndex] = commandBuffer;
				});

			tasks.push_back(task);

			firstIndex += size;
		}

		for (auto& task : tasks)
			task->wait();

		commandBuffer.executeSecondaryCommands(commandBuffers);
	}

	context.destroyAfterUse(std::move(gbufferSet));
	context.destroyAfterUse(std::move(emissiveSet));

	m_gbufferSet = nullptr;
	m_emissiveSet = nullptr;
	m_iblFrameSet = nullptr;
	m_iblEnvironmentSet = nullptr;
}

void LightPass::beginFrame()
{
	const auto& renderScene = getRenderScene();

	if (!renderScene.isValid())
		return;

	frustumCull();
}

void LightPass::endFrame()
{
	m_directionalLights.clear();
	m_pointLights.clear();
	m_spotLights.clear();
}

void LightPass::createLightingModel(const std::string& name)
{
	if (m_lightingModelNames.find(name) != m_lightingModelNames.end())
		return;

	m_lightingModelNames.emplace(name);

	const auto& lightingModel = Graphics::instance().getLightingModel(name);

	const auto gbufferBindings = m_gbuffer->getTextureBindings(lightingModel);
	for (const auto& binding : gbufferBindings)
		m_gbufferOptions[binding.index] = binding.bindingOptionName;

	m_updateShader = true;
}

void LightPass::createShaders()
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

	for (const auto& lightingModelName : m_lightingModelNames)
	{
		shader += "include Atema.LightingModel." + lightingModelName + "LightMaterial;\n";
	}

	shader += LightShaderMainBegin;

	bool useElse = false;

	for (const auto& lightingModelName : m_lightingModelNames)
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
	renderMaterialSettings.pipelineState.stencilBack.compareOperation = CompareOperation::Less;
	renderMaterialSettings.pipelineState.stencilBack.compareMask = 0xFF;
	renderMaterialSettings.pipelineState.colorBlend.enabled = true;
	renderMaterialSettings.pipelineState.colorBlend.colorSrcFactor = BlendFactor::One;
	renderMaterialSettings.pipelineState.colorBlend.colorDstFactor = BlendFactor::One;

	// Light material
	renderMaterialSettings.uberShaderOptions = gbufferOptions;
	renderMaterialSettings.uberShaderOptions.emplace_back("EnableShadows", ConstantValue(false));

	renderMaterialSettings.pipelineState.rasterization.cullMode = CullMode::Front;
	renderMaterialSettings.pipelineState.stencil = true;
	m_meshMaterial = std::make_shared<RenderMaterial>(*m_resourceManager, renderMaterialSettings);

	renderMaterialSettings.pipelineState.rasterization.cullMode = CullMode::Back;
	renderMaterialSettings.pipelineState.stencil = false;
	m_directionalMaterial = std::make_shared<RenderMaterial>(*m_resourceManager, renderMaterialSettings);

	// Image Based Lighting materials
	m_iblMaterials.clear();
	for (const auto& lightingModelName : m_lightingModelNames)
	{
		const LightingModel& lightingModel = graphics.getLightingModel(lightingModelName);

		if (lightingModel.environmentLightMaterial)
		{
			RenderMaterial::Settings materialSettings = renderMaterialSettings;
			materialSettings.material = lightingModel.environmentLightMaterial;

			m_iblMaterials.emplace_back(std::make_shared<RenderMaterial>(*m_resourceManager, materialSettings));
		}
	}

	// Light + shadow material
	renderMaterialSettings.uberShaderOptions = gbufferOptions;
	renderMaterialSettings.uberShaderOptions.emplace_back("EnableShadows", ConstantValue(true));
	renderMaterialSettings.uberShaderOptions.emplace_back("ShowDebugCascades", ConstantValue(false));
	renderMaterialSettings.uberShaderOptions.emplace_back("MaxShadowMapCascadeCount", ConstantValue(static_cast<uint32_t>(CascadedShadowData::MaxCascadeCount)));

	renderMaterialSettings.pipelineState.rasterization.cullMode = CullMode::Front;
	renderMaterialSettings.pipelineState.stencil = true;
	m_meshShadowMaterial = std::make_shared<RenderMaterial>(*m_resourceManager, renderMaterialSettings);

	renderMaterialSettings.pipelineState.rasterization.cullMode = CullMode::Back;
	renderMaterialSettings.pipelineState.stencil = false;
	m_directionalShadowMaterial = std::make_shared<RenderMaterial>(*m_resourceManager, renderMaterialSettings);

	m_useFrameSet = m_meshMaterial->hasBinding("FrameData") && m_meshMaterial->getBinding("FrameData").set != RenderMaterial::InvalidBindingIndex;
	m_useLightSet = m_meshMaterial->hasBinding("LightData") && m_meshMaterial->getBinding("LightData").set != RenderMaterial::InvalidBindingIndex;
	m_useLightShadowSet = m_meshShadowMaterial->hasBinding("CascadedShadowData") && m_meshShadowMaterial->getBinding("CascadedShadowData").set != RenderMaterial::InvalidBindingIndex;
	
	// Buffers and descriptor sets (one per frame in flight)
	Buffer::Settings frameBufferSettings;
	frameBufferSettings.usages = BufferUsage::Uniform | BufferUsage::TransferDst;
	frameBufferSettings.byteSize = FrameData::getLayout().getByteSize();

	m_oldResources.emplace_back(std::move(m_frameDataBuffer));
	m_oldResources.emplace_back(std::move(m_frameDataDescriptorSet));

	if (m_useFrameSet)
	{
		m_frameDataBuffer = m_resourceManager->createBuffer(frameBufferSettings);

		m_frameDataDescriptorSet = m_meshMaterial->createSet(FrameSetIndex);
		m_frameDataDescriptorSet->update(0, m_frameDataBuffer->getBuffer(), m_frameDataBuffer->getOffset(), m_frameDataBuffer->getSize());
	}

	gbufferBindingIndex = 0;
	const auto& textures = m_gbuffer->getTextures();
	for (const auto& texture : textures)
	{
		if (!m_meshMaterial->hasBinding(texture.name))
			m_gbufferBindings[gbufferBindingIndex] = InvalidGBufferBinding;

		gbufferBindingIndex++;
	}

	m_updateShader = false;
}

void LightPass::frustumCull()
{
	const auto& renderLights = getRenderScene().getRenderLights();

	std::vector<std::vector<const RenderLight*>> visibleLights;

	if (m_threadCount == 1)
	{
		visibleLights.resize(1);

		frustumCullElements(0, renderLights.size(), visibleLights[0]);
	}
	else
	{
		auto& taskManager = TaskManager::instance();

		visibleLights.resize(m_threadCount);

		std::vector<Ptr<Task>> tasks;
		tasks.reserve(m_threadCount);

		size_t firstIndex = 0;
		size_t size = renderLights.size() / m_threadCount;

		for (size_t taskIndex = 0; taskIndex < m_threadCount; taskIndex++)
		{
			if (taskIndex == m_threadCount - 1)
			{
				const auto remainingSize = renderLights.size() - m_threadCount * size;

				size += remainingSize;
			}

			auto task = taskManager.createTask([this, taskIndex, firstIndex, size, &visibleLights](size_t threadIndex)
				{
					frustumCullElements(firstIndex, size, visibleLights[taskIndex]);
				});

			tasks.push_back(task);

			firstIndex += size;
		}

		for (auto& task : tasks)
			task->wait();

		for (auto& lights : visibleLights)
		{
			for (auto& light : lights)
			{
				switch (light->getLight().getType())
				{
					case LightType::Directional:
					{
						m_directionalLights.emplace_back(light);
						break;
					}
					case LightType::Point:
					{
						m_pointLights.emplace_back(light);
						break;
					}
					case LightType::Spot:
					{
						m_spotLights.emplace_back(light);
						break;
					}
					default:
					{
						ATEMA_ERROR("Unhandled LightType");
					}
				};
			}
		}
	}
}

void LightPass::frustumCullElements(size_t index, size_t count, std::vector<const RenderLight*>& visibleLights) const
{
	if (!count)
		return;

	const auto& renderLights = getRenderScene().getRenderLights();

	const auto& frustum = getRenderScene().getCamera().getFrustum();

	visibleLights.reserve(count);

	for (size_t i = index; i < index + count; i++)
	{
		const auto& renderLight = *renderLights[i];

		switch (renderLight.getLight().getType())
		{
			case LightType::Directional:
			{
				visibleLights.emplace_back(&renderLight);
				break;
			}
			case LightType::Point:
			{
				const auto& pointLight = static_cast<const PointLight&>(renderLight.getLight());
				Sphere<float> boundingSphere(pointLight.getPosition(), pointLight.getRadius());

				if (frustum.getIntersectionType(boundingSphere) != IntersectionType::Outside)
					visibleLights.emplace_back(&renderLight);

				break;
			}
			case LightType::Spot:
			{
				const auto& spotLight = static_cast<const SpotLight&>(renderLight.getLight());
				Sphere<float> boundingSphere(spotLight.getPosition(), spotLight.getRange());

				if (frustum.getIntersectionType(boundingSphere) != IntersectionType::Outside)
					visibleLights.emplace_back(&renderLight);

				break;
			}
			default:
			{
				ATEMA_ERROR("Unhandled LightType");
			}
		}
	}
}

void LightPass::drawElements(CommandBuffer& commandBuffer, bool applyPostProcess,
	size_t directionalIndex, size_t directionalCount,
	size_t pointIndex, size_t pointCount,
	size_t spotIndex, size_t spotCount)
{
	const auto& viewport = getRenderScene().getCamera().getViewport();
	const auto& scissor = getRenderScene().getCamera().getScissor();

	commandBuffer.setViewport(viewport);

	commandBuffer.setScissor(scissor.pos, scissor.size);

	// Mesh lights
	if (pointCount > 0 || spotCount > 0)
	{
		// Step #1 : Fill stencil buffer for mesh lights
		m_meshStencilMaterial->bindTo(commandBuffer);

		commandBuffer.bindDescriptorSet(StencilFrameSetIndex, *m_frameDataDescriptorSet);

		// PointLights
		if (pointCount > 0)
		{
			commandBuffer.bindVertexBuffer(*m_sphereMesh->getVertexBuffer()->getBuffer(), 0);
			commandBuffer.bindIndexBuffer(*m_sphereMesh->getIndexBuffer()->getBuffer(), m_sphereMesh->getIndexBuffer()->getIndexType());

			for (size_t i = pointIndex; i < pointIndex + pointCount; i++)
			{
				const auto& renderLight = m_pointLights[i];

				commandBuffer.bindDescriptorSet(StencilLightSetIndex, renderLight->getLightDescriptorSet());

				commandBuffer.drawIndexed(static_cast<uint32_t>(m_sphereMesh->getIndexBuffer()->getSize()));
			}
		}

		// SpotLights
		if (spotCount > 0)
		{
			commandBuffer.bindVertexBuffer(*m_coneMesh->getVertexBuffer()->getBuffer(), 0);
			commandBuffer.bindIndexBuffer(*m_coneMesh->getIndexBuffer()->getBuffer(), m_coneMesh->getIndexBuffer()->getIndexType());

			for (size_t i = spotIndex; i < spotIndex + spotCount; i++)
			{
				const auto& renderLight = m_spotLights[i];

				commandBuffer.bindDescriptorSet(StencilLightSetIndex, renderLight->getLightDescriptorSet());

				commandBuffer.drawIndexed(static_cast<uint32_t>(m_coneMesh->getIndexBuffer()->getSize()));
			}
		}

		// Step #2 : draw lights that don't cast any shadows
		m_meshMaterial->bindTo(commandBuffer);

		commandBuffer.bindDescriptorSet(GBufferSetIndex, *m_gbufferSet);

		if (m_useFrameSet)
			commandBuffer.bindDescriptorSet(FrameSetIndex, *m_frameDataDescriptorSet);

		// PointLights
		if (pointCount > 0)
		{
			commandBuffer.bindVertexBuffer(*m_sphereMesh->getVertexBuffer()->getBuffer(), 0);
			commandBuffer.bindIndexBuffer(*m_sphereMesh->getIndexBuffer()->getBuffer(), m_sphereMesh->getIndexBuffer()->getIndexType());

			for (size_t i = pointIndex; i < pointIndex + pointCount; i++)
			{
				const auto& renderLight = m_pointLights[i];

				if (renderLight->getLight().castShadows())
					continue;

				if (m_useLightSet)
					commandBuffer.bindDescriptorSet(LightSetIndex, renderLight->getLightDescriptorSet());

				commandBuffer.drawIndexed(static_cast<uint32_t>(m_sphereMesh->getIndexBuffer()->getSize()));
			}
		}

		// SpotLights
		if (spotCount > 0)
		{
			commandBuffer.bindVertexBuffer(*m_coneMesh->getVertexBuffer()->getBuffer(), 0);
			commandBuffer.bindIndexBuffer(*m_coneMesh->getIndexBuffer()->getBuffer(), m_coneMesh->getIndexBuffer()->getIndexType());

			for (size_t i = spotIndex; i < spotIndex + spotCount; i++)
			{
				const auto& renderLight = m_spotLights[i];

				if (renderLight->getLight().castShadows())
					continue;

				if (m_useLightSet)
					commandBuffer.bindDescriptorSet(LightSetIndex, renderLight->getLightDescriptorSet());

				commandBuffer.drawIndexed(static_cast<uint32_t>(m_coneMesh->getIndexBuffer()->getSize()));
			}
		}

		// Step #3 : draw lights that cast shadows
		m_meshShadowMaterial->bindTo(commandBuffer);

		// PointLights
		if (pointCount > 0)
		{
			commandBuffer.bindVertexBuffer(*m_sphereMesh->getVertexBuffer()->getBuffer(), 0);
			commandBuffer.bindIndexBuffer(*m_sphereMesh->getIndexBuffer()->getBuffer(), m_sphereMesh->getIndexBuffer()->getIndexType());

			for (size_t i = pointIndex; i < pointIndex + pointCount; i++)
			{
				const auto& renderLight = m_pointLights[i];

				if (!renderLight->getLight().castShadows())
					continue;

				if (m_useLightSet)
					commandBuffer.bindDescriptorSet(LightSetIndex, renderLight->getLightDescriptorSet());

				if (m_useLightShadowSet)
					commandBuffer.bindDescriptorSet(ShadowSetIndex, renderLight->getShadowDescriptorSet());

				commandBuffer.drawIndexed(static_cast<uint32_t>(m_sphereMesh->getIndexBuffer()->getSize()));
			}
		}

		// SpotLights
		if (spotCount > 0)
		{
			commandBuffer.bindVertexBuffer(*m_coneMesh->getVertexBuffer()->getBuffer(), 0);
			commandBuffer.bindIndexBuffer(*m_coneMesh->getIndexBuffer()->getBuffer(), m_coneMesh->getIndexBuffer()->getIndexType());

			for (size_t i = spotIndex; i < spotIndex + spotCount; i++)
			{
				const auto& renderLight = m_spotLights[i];

				if (!renderLight->getLight().castShadows())
					continue;

				if (m_useLightSet)
					commandBuffer.bindDescriptorSet(LightSetIndex, renderLight->getLightDescriptorSet());

				if (m_useLightShadowSet)
					commandBuffer.bindDescriptorSet(ShadowSetIndex, renderLight->getShadowDescriptorSet());

				commandBuffer.drawIndexed(static_cast<uint32_t>(m_coneMesh->getIndexBuffer()->getSize()));
			}
		}
	}

	// Directional lights
	if (directionalCount > 0)
	{
		// Step #1 : draw lights that don't cast any shadows
		m_directionalMaterial->bindTo(commandBuffer);

		commandBuffer.bindVertexBuffer(*m_quadMesh->getBuffer(), 0);

		commandBuffer.bindDescriptorSet(GBufferSetIndex, *m_gbufferSet);

		if (m_useFrameSet)
			commandBuffer.bindDescriptorSet(FrameSetIndex, *m_frameDataDescriptorSet);

		for (size_t i = directionalIndex; i < directionalIndex + directionalCount; i++)
		{
			const auto& renderLight = m_directionalLights[i];

			if (renderLight->getLight().castShadows())
				continue;

			if (m_useLightSet)
				commandBuffer.bindDescriptorSet(LightSetIndex, renderLight->getLightDescriptorSet());

			commandBuffer.draw(static_cast<uint32_t>(m_quadMesh->getSize()));
		}

		// Step #2 : draw lights that cast shadows
		m_directionalShadowMaterial->bindTo(commandBuffer);

		for (size_t i = directionalIndex; i < directionalIndex + directionalCount; i++)
		{
			const auto& renderLight = m_directionalLights[i];

			if (!renderLight->getLight().castShadows())
				continue;

			if (m_useLightSet)
				commandBuffer.bindDescriptorSet(LightSetIndex, renderLight->getLightDescriptorSet());

			if (m_useLightShadowSet)
				commandBuffer.bindDescriptorSet(ShadowSetIndex, renderLight->getShadowDescriptorSet());

			commandBuffer.draw(static_cast<uint32_t>(m_quadMesh->getSize()));
		}
	}

	// Add post process (emissive lighting + ibl)
	if (applyPostProcess)
	{
		commandBuffer.bindVertexBuffer(*m_quadMesh->getBuffer(), 0);

		// Image Based Lighting
		if (m_iblFrameSet && m_iblEnvironmentSet)
		{
			for (auto& iblMaterial : m_iblMaterials)
			{
				iblMaterial->bindTo(commandBuffer);

				commandBuffer.bindDescriptorSet(GBufferSetIndex, *m_gbufferSet);
				commandBuffer.bindDescriptorSet(FrameSetIndex, *m_iblFrameSet);
				commandBuffer.bindDescriptorSet(2, *m_iblEnvironmentSet);

				commandBuffer.draw(static_cast<uint32_t>(m_quadMesh->getSize()));
			}
		}

		// Emissive lighting
		m_lightEmissiveMaterial->bindTo(commandBuffer);

		commandBuffer.bindDescriptorSet(0, *m_emissiveSet);

		commandBuffer.draw(static_cast<uint32_t>(m_quadMesh->getSize()));
	}
}
