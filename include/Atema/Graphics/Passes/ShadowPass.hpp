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

#ifndef ATEMA_GRAPHICS_SHADOWPASS_HPP
#define ATEMA_GRAPHICS_SHADOWPASS_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/FrameGraphTexture.hpp>
#include <Atema/Graphics/AbstractRenderPass.hpp>
#include <Atema/Graphics/Renderable.hpp>
#include <Atema/Renderer/Renderer.hpp>
#include <Atema/Renderer/Color.hpp>
#include <Atema/Renderer/DepthStencil.hpp>
#include <Atema/Math/Frustum.hpp>

#include <vector>
#include <optional>

namespace at
{
	class Camera;
	class Renderable;
	class FrameGraphBuilder;

	class ATEMA_GRAPHICS_API ShadowPass : public AbstractRenderPass
	{
	public:
		struct Settings
		{
			uint32_t shadowMapSize = 0;

			FrameGraphTextureHandle shadowMap = FrameGraph::InvalidTextureHandle;
			// Optional clear values
			// If unspecified or null, the attachment won't be cleared
			std::optional<DepthStencil> shadowMapClearValue;
		};

		ShadowPass() = delete;
		// threadCount : Number of threads this pass is allowed to use
		// 0 means as much as possible
		ShadowPass(size_t threadCount);
		ShadowPass(const ShadowPass& other) = default;
		ShadowPass(ShadowPass&& other) noexcept = default;
		~ShadowPass() = default;

		const char* getName() const noexcept override;

		void setViewProjection(const Matrix4f& viewProjection);
		void setFrustum(const Frustumf& frustum);

		FrameGraphPass& addToFrameGraph(FrameGraphBuilder& frameGraphBuilder, const Settings& settings);

		void execute(FrameGraphContext& context, const Settings& settings);

		ShadowPass& operator=(const ShadowPass& other) = default;
		ShadowPass& operator=(ShadowPass&& other) noexcept = default;

	protected:
		void doBeginFrame() override;
		void doEndFrame() override;

	private:
		struct FrameData
		{
			Ptr<Buffer> buffer;
			Ptr<DescriptorSet> descriptorSet;
		};

		void frustumCull();
		void frustumCullElements(std::vector<RenderElement>& renderElements, size_t index, size_t count) const;
		void updateFrameData(FrameData& frameData);
		void drawElements(CommandBuffer& commandBuffer, FrameData& frameData, size_t index, size_t count, uint32_t shadowMapSize);

		size_t m_threadCount;

		Ptr<DescriptorSetLayout> m_setLayout;
		Ptr<GraphicsPipeline> m_pipeline;
		std::array<FrameData, Renderer::FramesInFlight> m_frameDatas;

		Matrix4f m_viewProjection;
		Frustumf m_frustum;
		std::vector<RenderElement> m_renderElements;
	};
}

#endif