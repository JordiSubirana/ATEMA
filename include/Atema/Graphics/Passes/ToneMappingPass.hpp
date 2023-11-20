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

#ifndef ATEMA_GRAPHICS_TONEMAPPINGPASS_HPP
#define ATEMA_GRAPHICS_TONEMAPPINGPASS_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/AbstractRenderPass.hpp>
#include <Atema/Graphics/FrameGraph.hpp>

#include <optional>

namespace at
{
	class RenderMaterial;
	class VertexBuffer;
	class Sampler;
	class DescriptorSetLayout;

	class ATEMA_GRAPHICS_API ToneMappingPass : public AbstractRenderPass
	{
	public:
		struct Settings
		{
			FrameGraphTextureHandle input = FrameGraph::InvalidTextureHandle;
			FrameGraphTextureHandle output = FrameGraph::InvalidTextureHandle;
			std::optional<Color> outputClearColor;
		};

		ToneMappingPass() = delete;
		ToneMappingPass(RenderResourceManager& resourceManager);
		ToneMappingPass(const ToneMappingPass& other) = default;
		ToneMappingPass(ToneMappingPass&& other) noexcept = default;
		~ToneMappingPass() = default;
		
		const char* getName() const noexcept override;

		void setExposure(float exposure) noexcept;
		float getExposure() const noexcept;

		// Use 1.0f when rendering to a sRGB color format (gamma correction will be automatically applied)
		// Use another value if rendering to a linear color space texture (typically 2.2f)
		// Default value: 2.2f
		void setGamma(float gamma) noexcept;
		float getGamma() const noexcept;

		void updateResources(CommandBuffer& commandBuffer) override;

		FrameGraphPass& addToFrameGraph(FrameGraphBuilder& frameGraphBuilder, const Settings& settings);

		void execute(FrameGraphContext& context, const Settings& settings);

		ToneMappingPass& operator=(const ToneMappingPass& other) = default;
		ToneMappingPass& operator=(ToneMappingPass&& other) noexcept = default;

	private:
		RenderResourceManager* m_resourceManager;

		bool m_updateResources;

		float m_exposure;
		float m_gamma;

		Ptr<Material> m_material;
		Ptr<RenderMaterial> m_renderMaterial;
		Ptr<BufferAllocation> m_toneMappingBuffer;
		Ptr<DescriptorSet> m_toneMappingSet;

		Ptr<Sampler> m_sampler;

		Ptr<VertexBuffer> m_quadMesh;
	};
}

#endif
