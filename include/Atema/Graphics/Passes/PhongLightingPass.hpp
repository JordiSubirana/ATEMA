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

#ifndef ATEMA_GRAPHICS_PHONGLIGHTINGPASS_HPP
#define ATEMA_GRAPHICS_PHONGLIGHTINGPASS_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/FrameGraphTexture.hpp>
#include <Atema/Graphics/AbstractRenderPass.hpp>
#include <Atema/Renderer/Color.hpp>
#include <Atema/Renderer/DepthStencil.hpp>
#include <Atema/Math/Matrix.hpp>

#include <vector>
#include <optional>

namespace at
{
	class VertexBuffer;
	class Sampler;
	class DescriptorSetLayout;
	class Camera;
	class Renderable;
	class FrameGraphBuilder;

	class ATEMA_GRAPHICS_API PhongLightingPass : public AbstractRenderPass
	{
	public:
		struct Settings
		{
			// Output texture
			FrameGraphTextureHandle output = FrameGraph::InvalidTextureHandle;
			// Optional clear values
			// If unspecified or null, the attachment won't be cleared
			std::optional<Color> outputClearValue;

			// GBuffer texture handles
			std::vector<FrameGraphTextureHandle> gbuffer;
			std::vector<FrameGraphTextureHandle> shadowMaps;
			Ptr<Image> shadowMap;

			// Depth texture handle for stencil mask operations
			FrameGraphTextureHandle depthStencil = FrameGraph::InvalidTextureHandle;
			// Optional clear values
			// If unspecified or null, the attachment won't be cleared
			std::optional<DepthStencil> depthStencilClearValue;
		};

		struct ShadowCascadeData
		{
			float depth = 0.0f;
			float depthBias = 0.0f;
			Matrix4f viewProjection;
		};

		PhongLightingPass();
		PhongLightingPass(const PhongLightingPass& other) = default;
		PhongLightingPass(PhongLightingPass&& other) noexcept = default;
		~PhongLightingPass() = default;

		const char* getName() const noexcept override;

		void setShadowData(const std::vector<ShadowCascadeData>& shadowData);
		void setLightData(const Vector3f& direction, const Color& color, float ambientStrength);

		FrameGraphPass& addToFrameGraph(FrameGraphBuilder& frameGraphBuilder, const Settings& settings);

		void updateResources(RenderFrame& renderFrame, CommandBuffer& commandBuffer) override;

		void execute(FrameGraphContext& context, const Settings& settings);

		PhongLightingPass& operator=(const PhongLightingPass& other) = default;
		PhongLightingPass& operator=(PhongLightingPass&& other) noexcept = default;

	protected:
		void doBeginFrame() override;
		void doEndFrame() override;

	private:
		Ptr<DescriptorSetLayout> m_gbufferLayout;
		Ptr<DescriptorSetLayout> m_phongLayout;
		Ptr<GraphicsPipeline> m_pipeline;
		Ptr<Sampler> m_gbufferSampler;
		Ptr<Sampler> m_shadowMapSampler;
		Ptr<VertexBuffer> m_quadMesh;
		Ptr<DescriptorSet> m_phongSet;
		Ptr<Buffer> m_frameBuffer;
		Ptr<Buffer> m_shadowBuffer;
		Ptr<Buffer> m_lightBuffer;

		std::vector<ShadowCascadeData> m_shadowCascadeDatas;
		Vector3f m_lightDirection;
		Color m_lightColor;
		float m_lightAmbientStrength;
	};
}

#endif