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

#ifndef ATEMA_GRAPHICS_SKYPASS_HPP
#define ATEMA_GRAPHICS_SKYPASS_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/AbstractRenderPass.hpp>
#include <Atema/Graphics/RenderResourceManager.hpp>

#include <optional>
#include <Atema/Renderer/Renderer.hpp>

namespace at
{
	class Material;
	class RenderMaterial;
	class Mesh;

	class ATEMA_GRAPHICS_API SkyPass : public AbstractRenderPass
	{
	public:
		struct Settings
		{
			// Output texture
			FrameGraphTextureHandle output = FrameGraph::InvalidTextureHandle;
			// Optional clear values
			// If unspecified or null, the attachment won't be cleared
			std::optional<Color> outputClearValue;
			
			FrameGraphTextureHandle gbufferDepthStencil = FrameGraph::InvalidTextureHandle;
		};

		SkyPass() = delete;
		SkyPass(RenderResourceManager& resourceManager);
		SkyPass(const SkyPass& other) = default;
		SkyPass(SkyPass&& other) noexcept = default;
		~SkyPass() = default;

		const char* getName() const noexcept override;

		FrameGraphPass& addToFrameGraph(FrameGraphBuilder& frameGraphBuilder, const Settings& settings);

		void updateResources(CommandBuffer& commandBuffer) override;

		void execute(FrameGraphContext& context, const Settings& settings);

		SkyPass& operator=(const SkyPass& other) = default;
		SkyPass& operator=(SkyPass&& other) noexcept = default;

	private:
		RenderResourceManager* m_resourceManager;

		Ptr<Material> m_skyBoxMaterial;
		Ptr<Material> m_skySphereMaterial;
		Ptr<Material> m_skySphereToBoxMaterial;
		Ptr<RenderMaterial> m_skyBoxRenderMaterial;
		Ptr<RenderMaterial> m_skySphereRenderMaterial;
		Ptr<RenderMaterial> m_skySphereToBoxRenderMaterial;
		Ptr<Mesh> m_boxMesh;
		Ptr<Mesh> m_sphereMesh;
		Ptr<Sampler> m_sampler;

		Ptr<BufferAllocation> m_frameDataBuffer;
		Ptr<DescriptorSet> m_frameDataDescriptorSet;
	};
}

#endif
