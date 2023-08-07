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

#include <Atema/Graphics/DirectionalLight.hpp>
#include <Atema/Graphics/FrameRenderer.hpp>
#include <Atema/Graphics/FrameGraphBuilder.hpp>
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/Material.hpp>
#include <Atema/Graphics/Passes/GBufferPass.hpp>
#include <Atema/Graphics/Passes/ShadowPass.hpp>
#include <Atema/Graphics/Passes/DebugRendererPass.hpp>
#include <Atema/Graphics/Passes/DebugFrameGraphPass.hpp>
#include <Atema/Graphics/Passes/ScreenPass.hpp>

using namespace at;

namespace
{
	constexpr ImageFormat ColorFormat = ImageFormat::RGBA8_SRGB;
	constexpr ImageFormat DepthFormat = ImageFormat::D32_SFLOAT_S8_UINT;

	const DepthStencil DepthClearValue = DepthStencil(1.0f, 0);

	const Color SkyColor = Color::Black;

	constexpr size_t ThreadCount = 0;
}

FrameRenderer::FrameRenderer() :
	m_shaderLibraryManager(ShaderLibraryManager::instance()),
	m_updateFrameGraph(false),
	m_enableDebugRenderer(false),
	m_enableDebugGBuffer(false),
	m_enableDebugShadowMaps(false)
{
	createGBuffer();
}

Ptr<RenderMaterial> FrameRenderer::createRenderMaterial(Ptr<Material> material)
{
	const auto materialID = m_materialIdManager.get();

	ATEMA_ASSERT(material->getMetaData().exists(MaterialData::LightingModel), "Material must define the lighting model in its metadata");

	const auto& lightingModelName = material->getMetaData().getValue(MaterialData::LightingModel).get<std::string>();

	const auto lightingModelIndex = Graphics::instance().getLightingModelID(lightingModelName);

	addLightingModel(lightingModelName);

	RenderMaterial::Settings settings;
	settings.material = std::move(material);
	settings.id = materialID;
	settings.shaderLibraryManager = &m_shaderLibraryManager;

	settings.pipelineState.stencil = true;
	settings.pipelineState.stencilFront.compareOperation = CompareOperation::Always;
	settings.pipelineState.stencilFront.passOperation = StencilOperation::Replace;
	settings.pipelineState.stencilFront.writeMask = 0xFF;
	settings.pipelineState.stencilFront.reference = static_cast<uint32_t>(lightingModelIndex);

	auto renderMaterial = std::make_shared<RenderMaterial>(getRenderScene().getResourceManager(), settings);

	m_connectionGuard.connect(renderMaterial->onDestroy, [this, materialID]()
		{
			m_materialIdManager.release(materialID);
		});

	return renderMaterial;
}

void FrameRenderer::enableDebugRenderer(bool enable)
{
	if (m_enableDebugRenderer != enable)
	{
		m_enableDebugRenderer = enable;

		updateFrameGraph();
	}
}

void FrameRenderer::enableDebugGBuffer(bool enable)
{
	if (m_enableDebugGBuffer != enable)
	{
		m_enableDebugGBuffer = enable;

		updateFrameGraph();
	}
}

void FrameRenderer::enableDebugShadowMaps(bool enable)
{
	if (m_enableDebugShadowMaps != enable)
	{
		m_enableDebugShadowMaps = enable;

		updateFrameGraph();
	}
}

void FrameRenderer::createFrameGraph()
{
	m_oldFrameGraphs.emplace_back(std::move(m_frameGraph));

	FrameGraphBuilder frameGraphBuilder;

	Vector2u targetSize = getSize();

	if (targetSize.x == 0 || targetSize.y == 0)
		return;

	FrameGraphTextureSettings textureSettings;
	textureSettings.width = targetSize.x;
	textureSettings.height = targetSize.y;

	// Basic FrameGraph : only shows UI if GBuffer is not valid
	if (!m_gbuffer)
	{
		textureSettings.format = ColorFormat;
		auto compositionTexture = frameGraphBuilder.createTexture(textureSettings);

		// Empty pass only here to clear attachments
		{
			auto& pass = frameGraphBuilder.createPass("Empty");

			pass.addOutputTexture(compositionTexture, 0, Color::Black);

			pass.setExecutionCallback([](FrameGraphContext& context)
				{
					// Does nothing
				});
		}

		// Screen
		{
			ScreenPass::Settings passSettings;

			passSettings.input = compositionTexture;

			m_screenPass->addToFrameGraph(frameGraphBuilder, passSettings);
			m_activePasses.emplace_back(m_screenPass.get());
		}
	}
	// GBuffer is valid : build full FrameGraph
	else
	{
		// Texture setup

		std::vector<FrameGraphTextureHandle> gbufferTextures;
		std::vector<std::optional<Color>> gbufferClearValues;
		for (auto& texture : m_gbuffer->getTextures())
		{
			textureSettings.format = texture.format;

			const auto gbufferTexture = frameGraphBuilder.createTexture(textureSettings);

			gbufferTextures.emplace_back(gbufferTexture);
			gbufferClearValues.emplace_back(Color::Black);
		}

		textureSettings.format = DepthFormat;
		auto gbufferDepthTexture = frameGraphBuilder.createTexture(textureSettings);

		textureSettings.format = ColorFormat;
		auto compositionTexture = frameGraphBuilder.createTexture(textureSettings);

		// Pass setup

		m_activePasses.clear();

		// GBuffer
		{
			GBufferPass::Settings passSettings;

			passSettings.gbuffer = gbufferTextures;
			passSettings.gbufferClearValue = gbufferClearValues;
			passSettings.depthStencil = gbufferDepthTexture;
			passSettings.depthStencilClearValue = DepthClearValue;

			m_gbufferPass->addToFrameGraph(frameGraphBuilder, passSettings);
			m_activePasses.emplace_back(m_gbufferPass.get());
		}

		// Shadow
		std::vector<FrameGraphTextureHandle> shadowMaps;
		for (auto& [renderLight, shadowData] : m_shadowData)
		{
			const auto light = &renderLight->getLight();
			const auto& shadowMap = renderLight->getShadowMap();

			for (size_t i = 0; i < light->getShadowCascadeCount(); i++)
			{
				auto& shadowPass = shadowData->passes[i];

				auto shadowTextureHandle = frameGraphBuilder.importTexture(shadowMap, static_cast<uint32_t>(i));

				shadowMaps.emplace_back(shadowTextureHandle);

				ShadowPass::Settings passSettings;

				passSettings.shadowMapSize = shadowMap->getSize().x;
				passSettings.shadowMap = shadowTextureHandle;
				passSettings.shadowMapClearValue = DepthStencil(1.0f, 0);

				shadowPass->addToFrameGraph(frameGraphBuilder, passSettings);

				m_activePasses.emplace_back(shadowPass.get());
			}
		}

		// Light
		{
			LightPass::Settings passSettings;

			passSettings.output = compositionTexture;
			passSettings.outputClearValue = SkyColor;
			passSettings.gbuffer = gbufferTextures;
			passSettings.gbufferDepthStencil = gbufferDepthTexture;
			passSettings.shadowMaps = shadowMaps;
			passSettings.depthStencil = gbufferDepthTexture;

			m_lightPass->addToFrameGraph(frameGraphBuilder, passSettings);
			m_activePasses.emplace_back(m_lightPass.get());
		}

		// Debug Renderer
		if (m_enableDebugRenderer)
		{
			DebugRendererPass::Settings passSettings;

			passSettings.output = compositionTexture;
			passSettings.depthStencil = gbufferDepthTexture;

			m_debugRendererPass->addToFrameGraph(frameGraphBuilder, passSettings);
			m_activePasses.emplace_back(m_debugRendererPass.get());
		}

		// Debug FrameGraph
		if (m_enableDebugGBuffer || m_enableDebugShadowMaps)
		{
			DebugFrameGraphPass::Settings passSettings;

			passSettings.output = compositionTexture;
			passSettings.columnCount = 0;

			if (m_enableDebugGBuffer)
				passSettings.textures.insert(passSettings.textures.end(), gbufferTextures.begin(), gbufferTextures.end());

			if (m_enableDebugShadowMaps)
				passSettings.textures.insert(passSettings.textures.end(), shadowMaps.begin(), shadowMaps.end());

			m_debugFrameGraphPass->addToFrameGraph(frameGraphBuilder, passSettings);
			m_activePasses.emplace_back(m_debugFrameGraphPass.get());
		}

		// Screen
		{
			ScreenPass::Settings passSettings;

			passSettings.input = compositionTexture;

			m_screenPass->addToFrameGraph(frameGraphBuilder, passSettings);
			m_activePasses.emplace_back(m_screenPass.get());
		}
	}

	//-----
	// Build frame graph

	m_frameGraph = frameGraphBuilder.build();
}

FrameGraph* FrameRenderer::getFrameGraph()
{
	return m_frameGraph.get();
}

std::vector<AbstractRenderPass*>& FrameRenderer::getRenderPasses()
{
	return m_activePasses;
}

void FrameRenderer::destroyResources(RenderFrame& renderFrame)
{
	for (auto& renderPass : m_oldRenderPasses)
		renderFrame.destroyAfterUse(std::move(renderPass));

	m_oldRenderPasses.clear();

	for (auto& frameGraph : m_oldFrameGraphs)
		renderFrame.destroyAfterUse(std::move(frameGraph));

	m_oldFrameGraphs.clear();
}

void FrameRenderer::beginFrame()
{
	updateLightResources();

	if (m_updateFrameGraph)
	{
		createFrameGraph();

		m_updateFrameGraph = false;
	}

	if (m_lightPass)
		m_lightPass->setLightingModels(m_lightingModelNames);
}

void FrameRenderer::addLightingModel(const std::string& name)
{
	// Check if the lighting model already exists
	if (m_lightingModels.find(name) != m_lightingModels.end())
		return;

	const auto& lightingModel = Graphics::instance().getLightingModel(name);

	m_lightingModels[name] = lightingModel;
	m_lightingModelNames.emplace_back(name);

	if (!m_gbuffer || !m_gbuffer->isCompatible(lightingModel))
		createGBuffer();
}

void FrameRenderer::createGBuffer()
{
	if (m_lightingModels.empty())
	{
		m_gbuffer.reset();
	}
	else
	{
		std::vector<LightingModel> lightingModels;
		for (const auto& [lightingModelName, lightingModel] : m_lightingModels)
			lightingModels.emplace_back(lightingModel);

		m_gbuffer = std::make_unique<GBuffer>(lightingModels);

		m_gbuffer->generateShaderLibraries(m_shaderLibraryManager);

		getRenderScene().recompileMaterials();
	}

	createPasses();

	updateFrameGraph();
}

void FrameRenderer::createPasses()
{
	m_activePasses.clear();

	// Destroy old passes
	m_oldRenderPasses.emplace_back(std::move(m_gbufferPass));

	m_oldRenderPasses.emplace_back(std::move(m_lightPass));

	m_oldRenderPasses.emplace_back(std::move(m_debugRendererPass));

	m_oldRenderPasses.emplace_back(std::move(m_debugFrameGraphPass));

	m_oldRenderPasses.emplace_back(std::move(m_screenPass));

	// Create new ones

	// Some passes require a valid GBuffer
	if (m_gbuffer)
	{
		m_gbufferPass = std::make_unique<GBufferPass>(ThreadCount);

		m_lightPass = std::make_unique<LightPass>(getRenderScene().getResourceManager(), *m_gbuffer, m_shaderLibraryManager, ThreadCount);
		m_lightPass->setLightingModels(m_lightingModelNames);

		m_debugRendererPass = std::make_unique<DebugRendererPass>();

		m_debugFrameGraphPass = std::make_unique<DebugFrameGraphPass>();
	}

	// Some passes are always valid
	m_screenPass = std::make_unique<ScreenPass>();
}

void FrameRenderer::updateLightResources()
{
	std::unordered_map<const RenderLight*, Ptr<ShadowPassData>> oldShadowData;

	std::swap(m_shadowData, oldShadowData);

	const auto& renderLights = getRenderScene().getRenderLights();

	for (const auto& renderLight : renderLights)
	{
		const auto& light = renderLight->getLight();

		if (!light.castShadows())
			continue;

		// Does the shadow map already exist?
		auto it = oldShadowData.find(renderLight.get());

		if (it != oldShadowData.end())
		{
			// Update shadow data
			updateShadowData(*renderLight, *it->second);

			m_shadowData[renderLight.get()] = std::move(it->second);
		}
		// If not, create it
		else
		{
			createShadowData(*renderLight);
		}
	}

	// If some shadows were removed, update the FrameGraph
	if (m_shadowData.size() != oldShadowData.size())
		m_updateFrameGraph = true;

	// Remove old shadow passes
	for (auto& [light, shadowData] : oldShadowData)
	{
		if (!shadowData)
			continue;

		for (auto& pass : shadowData->passes)
			m_oldRenderPasses.emplace_back(std::move(pass));
	}
}

void FrameRenderer::createShadowData(RenderLight& renderLight)
{
	auto shadowData = std::make_shared<ShadowPassData>();
	shadowData->passes.resize(renderLight.getLight().getShadowCascadeCount());

	for (auto& pass : shadowData->passes)
		pass = std::make_unique<ShadowPass>(ThreadCount);

	updateShadowData(renderLight, *shadowData);

	// We created a new shadow map : we need to update the FrameGraph
	m_updateFrameGraph = true;

	m_shadowData[&renderLight] = std::move(shadowData);

	// Rebuild the FrameGraph everytime the shadow map changes
	m_connectionGuard.connect(renderLight.onShadowMapUpdated, [this]()
		{
			m_updateFrameGraph = true;
		});
}

void FrameRenderer::updateShadowData(RenderLight& renderLight, ShadowPassData& shadowPassData)
{
	const auto& camera = getRenderScene().getCamera();
	const auto& cameraPos = camera.getPosition();

	const auto& light = renderLight.getLight();

	if (shadowPassData.passes.size() < light.getShadowCascadeCount())
	{
		for (size_t i = shadowPassData.passes.size(); i < light.getShadowCascadeCount(); i++)
			shadowPassData.passes.emplace_back(std::make_unique<ShadowPass>(ThreadCount));

		m_updateFrameGraph = true;
	}
	else if (shadowPassData.passes.size() > light.getShadowCascadeCount())
	{
		for (size_t i = light.getShadowCascadeCount(); i < shadowPassData.passes.size(); i++)
			m_oldRenderPasses.emplace_back(std::move(shadowPassData.passes[i]));

		shadowPassData.passes.resize(light.getShadowCascadeCount());

		m_updateFrameGraph = true;
	}

	switch (light.getType())
	{
		case LightType::Directional :
		{
			const auto& directionalLight = static_cast<const DirectionalLight&>(light);

			const auto baseDepthBias = directionalLight.getShadowDepthBias();
			const auto maxDepth = directionalLight.getShadowMaxDepth();
			const float depthStep = maxDepth / static_cast<float>(std::pow(2, light.getShadowCascadeCount()));
			const auto shadowMapSize = static_cast<float>(light.getShadowMapSize());

			const auto lightView = Matrix4f::createLookAt(-directionalLight.getDirection() * maxDepth, Vector3f(0, 0, 0), Vector3f(0, 0, 1));

			const auto view = Matrix4f::createLookAt(Vector3f(0, 0, 0), Vector3f(1, 0, 0), Vector3f(0, 0, 1));

			Frustumf frustumRef(camera.getProjectionMatrix() * view);

			auto frustumPlanesRef = frustumRef.getPlanes();
			auto& nearPlaneRef = frustumPlanesRef[static_cast<size_t>(FrustumPlane::Near)];
			auto& farPlaneRef = frustumPlanesRef[static_cast<size_t>(FrustumPlane::Far)];
			auto& viewDirRef = nearPlaneRef.getNormal();

			auto frustumPlanes = camera.getFrustum().getPlanes();
			auto& nearPlane = frustumPlanes[static_cast<size_t>(FrustumPlane::Near)];
			auto& farPlane = frustumPlanes[static_cast<size_t>(FrustumPlane::Far)];
			auto& viewDir = nearPlane.getNormal();

			float previousDepth = 0.01f;

			float currentFar = 0.0f;

			std::vector<ShadowData> cascades;
			cascades.resize(shadowPassData.passes.size());
			for (size_t i = 0; i < shadowPassData.passes.size(); i++)
			{
				auto& cascade = cascades[i];
				auto& shadowPass = *shadowPassData.passes[i];
				auto& depth = cascade.depth;
				auto& depthBias = cascade.depthBias;
				auto& viewProjection = cascade.viewProjection;

				currentFar += depthStep * static_cast<float>(std::pow(2, i));
				depth = currentFar;

				nearPlane.set(nearPlane.getNormal(), cameraPos + viewDir * previousDepth);
				farPlane.set(farPlane.getNormal(), cameraPos + viewDir * depth);

				nearPlaneRef.set(nearPlaneRef.getNormal(), viewDirRef * previousDepth);
				farPlaneRef.set(farPlaneRef.getNormal(), viewDirRef * depth);

				Frustumf frustum(frustumPlanes);

				frustumRef.set(frustumPlanesRef);

				// Find the tightest box possible around the frustum
				Vector3f boundingBoxSize;
				boundingBoxSize.x = (frustumRef.getCorner(FrustumCorner::FarTopLeft) - frustumRef.getCorner(FrustumCorner::FarTopRight)).getNorm();
				boundingBoxSize.y = (frustumRef.getCorner(FrustumCorner::FarTopLeft) - frustumRef.getCorner(FrustumCorner::FarBottomLeft)).getNorm();
				boundingBoxSize.z = depth - previousDepth;

				// Find the bounding sphere of this box (and of the frustum)
				// This way the shadowmap remains coherent in size no matter the orientation
				const auto sphereCenter = cameraPos + viewDir * (previousDepth + depth) / 2.0f;

				auto sphereRadius = boundingBoxSize.getNorm() / 2.0f;

				const float texelSizeWorldSpace = sphereRadius / (shadowMapSize / 2.0f);

				const auto factor = shadowMapSize / (shadowMapSize - 1.0f);
				sphereRadius = sphereRadius * factor;

				// We move the center of the shadowmap according to its resolution (avoid shadow shimmering)
				auto centerOffsetLightSpace = lightView.transformVector(sphereCenter);
				centerOffsetLightSpace.x = texelSizeWorldSpace * std::round(centerOffsetLightSpace.x / texelSizeWorldSpace);
				centerOffsetLightSpace.y = texelSizeWorldSpace * std::round(centerOffsetLightSpace.y / texelSizeWorldSpace);
				centerOffsetLightSpace.z = texelSizeWorldSpace * std::round(centerOffsetLightSpace.z / texelSizeWorldSpace);

				// Lightspace frustum bounding box
				AABBf aabb;
				for (const auto& corner : frustum.getCorners())
					aabb.extend(lightView.transformPosition(corner));

				auto c = centerOffsetLightSpace;
				const auto r = sphereRadius;

				// Keep the Z range as tight as possible
				auto proj = Matrix4f::createOrtho(c.x - r, c.x + r, -c.y - r, -c.y + r, -aabb.max.z, -aabb.min.z);
				proj[1][1] *= -1;

				viewProjection = proj * lightView;

				shadowPass.setViewProjection(viewProjection);

				// For the culling we use another projection :
				// - reduced X/Y size to draw less objects
				// - increased Z size in case a caster is between the view frustum & the light
				c = aabb.getCenter();
				const auto s = aabb.getSize() / 2.0f;
				auto cullingProj = Matrix4f::createOrtho(c.x - s.x, c.x + s.x, -c.y - s.y, -c.y + s.y, 0.0f, -aabb.min.z);
				cullingProj[1][1] *= -1;

				// Depth bias depends on the base depth bias and the z difference in light space
				const auto diffZ = aabb.getSize().z;
				depthBias = baseDepthBias / diffZ;

				shadowPass.setFrustum(Frustumf(cullingProj * lightView));

				previousDepth = depth;
			}

			renderLight.setShadowData(cascades);

			break;
		}
		default :
		{
			ATEMA_ERROR("Unhandled light type");
		}
	}
}
