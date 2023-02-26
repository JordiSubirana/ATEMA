/*
	Copyright 2022 Jordi SUBIRANA

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

#ifndef ATEMA_SANDBOX_GRAPHICSSYSTEM_HPP
#define ATEMA_SANDBOX_GRAPHICSSYSTEM_HPP

#include <Atema/Graphics/DebugRenderer.hpp>
#include <Atema/Math/AABB.hpp>

#include "System.hpp"
#include "../Settings.hpp"

struct MaterialData;

class GraphicsSystem : public System
{
public:
	GraphicsSystem() = delete;
	GraphicsSystem(const at::Ptr<at::RenderWindow>& renderWindow);
	virtual ~GraphicsSystem();

	void update(at::TimeStep timeStep) override;
	void onEvent(at::Event& event) override;

private:
	struct FrameData
	{
		size_t objectCount;
		at::Ptr<at::Buffer> objectsUniformBuffer;
		at::Ptr<at::DescriptorSet> objectDescriptorSet;

		at::Ptr<at::Buffer> frameUniformBuffer;
		at::Ptr<at::DescriptorSet> frameDescriptorSet;

		at::Ptr<at::Buffer> shadowBuffer;
		at::Ptr<at::DescriptorSet> shadowSet;

		at::Ptr<at::Buffer> shadowCascadesBuffer;

		at::Ptr<at::Buffer> phongBuffer;
	};

	struct Material
	{
		at::Ptr<at::GraphicsPipeline> pipeline;
		at::Ptr<at::DescriptorSet> descriptorSet;
	};

	void translateShaders();

	void checkSettings();

	void createFrameGraph();
	void onResize(const at::Vector2u& size);
	void updateFrame();

	void updateBoundingBoxes();
	void updateUniformBuffers(FrameData& frameData);

	at::WPtr<at::RenderWindow> m_renderWindow;
	
	float m_totalTime;

	at::ImageFormat m_depthFormat;

	at::Viewport m_viewport;
	at::Vector2u m_windowSize;

	// FrameGraph
	bool m_updateFrameGraph;
	at::Ptr<at::FrameGraph> m_frameGraph;

	// Rendering resources (post process)
	at::Ptr<at::Buffer> m_ppQuad;
	at::Ptr<at::Sampler> m_ppSampler;
	at::Ptr<at::DescriptorSetLayout> m_ppLayout;

	// Object resources
	std::unordered_map<MaterialData*, Material> m_materials;

	// Frame resources
	at::Ptr<at::DescriptorSetLayout> m_frameLayout;
	at::Ptr<at::DescriptorSetLayout> m_objectLayout;
	uint32_t m_elementByteSize;
	uint32_t m_dynamicObjectBufferOffset;
	std::vector<FrameData> m_frameDatas;

	// Shadow mapping
	at::Ptr<at::DescriptorSetLayout> m_shadowLayout;
	at::Ptr<at::DescriptorSetLayout> m_gbufferShadowMapLayout;
	at::Ptr<at::GraphicsPipeline> m_shadowPipeline;
	uint32_t m_currentShadowMapSize;
	uint32_t m_shadowMapSize;
	uint32_t m_dynamicShadowBufferOffset;
	uint32_t m_shadowElementByteSize;
	at::Viewport m_shadowViewport;
	at::Ptr<at::Image> m_shadowMap;
	at::Ptr<at::Sampler> m_shadowMapSampler;
	std::array<float, SHADOW_CASCADE_COUNT> m_shadowCascadeDepths;
	std::array<float, SHADOW_CASCADE_COUNT> m_shadowCascadeRadii;

	// Phong lighting
	at::Ptr<at::DescriptorSetLayout> m_phongLayout;
	at::Ptr<at::GraphicsPipeline> m_phongPipeline;

	// Screen
	at::Ptr<at::DescriptorSetLayout> m_screenLayout;
	at::Ptr<at::GraphicsPipeline> m_screenPipeline;

	// Settings
	bool m_enableDebugRenderer;
	Settings::DebugViewMode m_debugViewMode;
	std::vector<Settings::DebugView> m_debugViews;

	// Misc
	at::Ptr<at::DebugRenderer> m_debugRenderer;

	float m_frustumRotation;
	at::Frustumf m_customfrustum;
	std::function<bool(const at::AABBf&)> m_cullFunction;

	std::array<at::Frustumf, SHADOW_CASCADE_COUNT> m_lightFrustums;

	at::Matrix4f m_viewProjection;
};

#endif
