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

#include <Atema/Graphics/FrameRenderer.hpp>
#include <Atema/Graphics/FrameGraphBuilder.hpp>
#include <Atema/Graphics/Passes/GBufferPass.hpp>
#include <Atema/Graphics/Passes/ShadowPass.hpp>
#include <Atema/Graphics/Passes/PhongLightingPass.hpp>
#include <Atema/Graphics/Passes/DebugRendererPass.hpp>
#include <Atema/Graphics/Passes/DebugFrameGraphPass.hpp>
#include <Atema/Graphics/Passes/ScreenPass.hpp>

using namespace at;

namespace
{
	const std::vector<ImageFormat> GBufferFormat =
	{
		// Position (RGB) + Metalness (A)
		ImageFormat::RGBA32_SFLOAT,
		// Normal (RGB) + Roughness (A)
		ImageFormat::RGBA32_SFLOAT,
		// Albedo (RGB) + AO (A)
		ImageFormat::RGBA8_UNORM,
		// Emissive
		ImageFormat::RGBA8_UNORM,
	};

	const std::vector<std::optional<Color>> GBufferClearValues =
	{
		Color::Black,
		Color::Black,
		Color::Black,
		Color::Black
	};

	constexpr ImageFormat ColorFormat = ImageFormat::RGBA32_SFLOAT;
	constexpr ImageFormat DepthFormat = ImageFormat::D32_SFLOAT_S8_UINT;

	const DepthStencil DepthClearValue = DepthStencil(1.0f, 0);

	const Vector3f LightDirection = Vector3f(1.0f, 1.0f, -1.0f).normalize();

	const Color LightColor = Color::White;

	constexpr float LightAmbientStrength = 0.35f;

	constexpr float LightSpecularStrength = 0.5f;

	constexpr size_t ShadowCascadeCount = 8;

	constexpr float BaseDepthBias = 0.07f;

	constexpr float MaxFarDepth = 1000.0f;

	const float DepthStep = MaxFarDepth / static_cast<float>(std::pow(2, ShadowCascadeCount));

	constexpr uint32_t ShadowMapSize = 4096;

	const Color SkyColor = Color::Blue;

	constexpr size_t ThreadCount = 0;
}

FrameRenderer::FrameRenderer() :
	m_enableDebugRenderer(false),
	m_enableDebugFrameGraph(false),
	m_currentShadowMapSize(0),
	m_shadowMapSize(ShadowMapSize)
{
	createPasses();
}

void FrameRenderer::enableDebugRenderer(bool enable)
{
	if (m_enableDebugRenderer != enable)
	{
		m_enableDebugRenderer = enable;

		updateFrameGraph();
	}
}

void FrameRenderer::enableDebugFrameGraph(bool enable)
{
	if (m_enableDebugFrameGraph != enable)
	{
		m_enableDebugFrameGraph = enable;

		updateFrameGraph();
	}
}

void FrameRenderer::createFrameGraph()
{
	FrameGraphBuilder frameGraphBuilder;
	
	//-----
	// Shadow Map (we are creating it here because the size can change & it causes FrameGraph to rebuild)
	if (m_currentShadowMapSize != m_shadowMapSize)
	{
		Image::Settings imageSettings;
		imageSettings.format = ImageFormat::D16_UNORM;
		imageSettings.width = m_shadowMapSize;
		imageSettings.height = m_shadowMapSize;
		imageSettings.layers = ShadowCascadeCount;

		m_shadowMap = Image::create(imageSettings);

		m_currentShadowMapSize = m_shadowMapSize;
	}
	
	//-----
	// Texture setup

	Vector2u targetSize = getSize();

	FrameGraphTextureSettings textureSettings;
	textureSettings.width = targetSize.x;
	textureSettings.height = targetSize.y;

	std::vector<FrameGraphTextureHandle> gbufferTextures;
	gbufferTextures.reserve(GBufferFormat.size());

	for (auto& format : GBufferFormat)
	{
		textureSettings.format = format;

		auto texture = frameGraphBuilder.createTexture(textureSettings);

		gbufferTextures.emplace_back(texture);
	}

	textureSettings.format = DepthFormat;
	auto gbufferDepthTexture = frameGraphBuilder.createTexture(textureSettings);

	textureSettings.format = ColorFormat;
	auto compositionTexture = frameGraphBuilder.createTexture(textureSettings);

	std::vector<FrameGraphTextureHandle> shadowCascades;
	shadowCascades.reserve(ShadowCascadeCount);
	for (size_t i = 0; i < ShadowCascadeCount; i++)
		shadowCascades.emplace_back(frameGraphBuilder.importTexture(m_shadowMap, i));

	//-----
	// Pass setup

	m_activePasses.clear();

	// GBuffer
	{
		GBufferPass::Settings passSettings;

		passSettings.gbuffer = gbufferTextures;
		passSettings.gbufferClearValue = GBufferClearValues;
		passSettings.depthStencil = gbufferDepthTexture;
		passSettings.depthStencilClearValue = DepthClearValue;
		
		m_gbufferPass->addToFrameGraph(frameGraphBuilder, passSettings);
		m_activePasses.emplace_back(m_gbufferPass.get());
	}

	// Shadow
	for (size_t i = 0; i < ShadowCascadeCount; i++)
	{
		ShadowPass::Settings passSettings;
		
		passSettings.shadowMapSize = ShadowMapSize;
		passSettings.shadowMap = shadowCascades[i];
		passSettings.shadowMapClearValue = DepthStencil(1.0f, 0);
		
		m_shadowPasses[i]->addToFrameGraph(frameGraphBuilder, passSettings);
		m_activePasses.emplace_back(m_shadowPasses[i].get());
	}

	// Phong Lighting
	{
		PhongLightingPass::Settings passSettings;

		passSettings.output = compositionTexture;
		passSettings.outputClearValue = SkyColor;
		passSettings.gbuffer = gbufferTextures;
		passSettings.shadowMaps = shadowCascades;
		passSettings.shadowMap = m_shadowMap;
		passSettings.depthStencil = gbufferDepthTexture;
		
		m_phongLightingPass->addToFrameGraph(frameGraphBuilder, passSettings);
		m_activePasses.emplace_back(m_phongLightingPass.get());
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
	if (m_enableDebugFrameGraph)
	{
		DebugFrameGraphPass::Settings passSettings;

		passSettings.output = compositionTexture;
		passSettings.textures = gbufferTextures;
		passSettings.textures.insert(passSettings.textures.end(), shadowCascades.begin(), shadowCascades.end());
		passSettings.columnCount = 0;
		
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

	//-----
	// Build frame graph
	m_oldFrameGraphs.emplace_back(std::move(m_frameGraph));

	m_frameGraph = frameGraphBuilder.build();
}

FrameGraph& FrameRenderer::getFrameGraph()
{
	return *m_frameGraph;
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

void FrameRenderer::doBeginFrame()
{
	updateShadowData();
}

void FrameRenderer::createPasses()
{
	m_activePasses.clear();

	// Destroy old passes
	m_oldRenderPasses.emplace_back(std::move(m_gbufferPass));

	for (auto& pass : m_shadowPasses)
		m_shadowPasses.emplace_back(std::move(pass));
	m_shadowPasses.clear();

	m_oldRenderPasses.emplace_back(std::move(m_phongLightingPass));

	m_oldRenderPasses.emplace_back(std::move(m_debugRendererPass));

	m_oldRenderPasses.emplace_back(std::move(m_debugFrameGraphPass));

	m_oldRenderPasses.emplace_back(std::move(m_screenPass));

	// Create new ones
	m_gbufferPass = std::make_unique<GBufferPass>(ThreadCount);

	m_shadowPasses.reserve(ShadowCascadeCount);
	m_shadowPasses.clear();
	for (size_t i = 0; i < ShadowCascadeCount; i++)
		m_shadowPasses.emplace_back(std::make_unique<ShadowPass>(ThreadCount));

	m_phongLightingPass = std::make_unique<PhongLightingPass>();

	m_debugRendererPass = std::make_unique<DebugRendererPass>();

	m_debugFrameGraphPass = std::make_unique<DebugFrameGraphPass>();

	m_screenPass = std::make_unique<ScreenPass>();
}

void FrameRenderer::updateShadowData()
{
	const auto& camera = getRenderData().getCamera();
	const auto& cameraPos = camera.getPosition();

	float previousDepth = 0.01f;

	const auto lightView = Matrix4f::createLookAt(-LightDirection * MaxFarDepth, Vector3f(0, 0, 0), Vector3f(0, 0, 1));

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

	float currentFar = 0.0f;
	m_shadowCascadeData.resize(ShadowCascadeCount);
	for (size_t i = 0; i < ShadowCascadeCount; i++)
	{
		auto& shadowPass = *m_shadowPasses[i];
		auto& shadowCascadeData = m_shadowCascadeData[i];

		auto& depth = shadowCascadeData.depth;
		auto& depthBias = shadowCascadeData.depthBias;
		auto& viewProjection = shadowCascadeData.viewProjection;

		currentFar += DepthStep * static_cast<float>(std::pow(2, i));
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

		const float texelSizeWorldSpace = sphereRadius / static_cast<float>(m_shadowMapSize / 2);

		const auto factor = static_cast<float>(m_shadowMapSize) / static_cast<float>(m_shadowMapSize - 1);
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
		depthBias = BaseDepthBias / diffZ;

		shadowPass.setFrustum(Frustumf(cullingProj * lightView));

		previousDepth = depth;
	}

	m_phongLightingPass->setShadowData(m_shadowCascadeData);
	m_phongLightingPass->setLightData(LightDirection, LightColor, LightAmbientStrength, LightSpecularStrength);
}
