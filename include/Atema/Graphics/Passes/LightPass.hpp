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

#ifndef ATEMA_GRAPHICS_LIGHTPASS_HPP
#define ATEMA_GRAPHICS_LIGHTPASS_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/FrameGraphTexture.hpp>
#include <Atema/Graphics/AbstractRenderPass.hpp>
#include <Atema/Renderer/Color.hpp>
#include <Atema/Renderer/DepthStencil.hpp>
#include <Atema/Renderer/Renderer.hpp>
#include <Atema/Shader/ShaderLibraryManager.hpp>
#include <Atema/Math/Matrix.hpp>

#include <vector>
#include <optional>

namespace at
{
	class RenderResourceManager;
	class RenderLight;
	class Mesh;
	class GBuffer;
	class RenderMaterial;
	class Light;
	class VertexBuffer;
	class Sampler;
	class DescriptorSetLayout;
	class Camera;
	class Renderable;
	class FrameGraphBuilder;

	class ATEMA_GRAPHICS_API LightPass : public AbstractRenderPass
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
			FrameGraphTextureHandle gbufferDepthStencil = FrameGraph::InvalidTextureHandle;
			std::vector<FrameGraphTextureHandle> shadowMaps;

			// Depth texture handle for stencil mask operations
			FrameGraphTextureHandle depthStencil = FrameGraph::InvalidTextureHandle;
			// Optional clear values
			// If unspecified or null, the attachment won't be cleared
			std::optional<DepthStencil> depthStencilClearValue;
		};

		LightPass() = delete;
		LightPass(RenderResourceManager& resourceManager, const GBuffer& gbuffer, const ShaderLibraryManager& shaderLibraryManager, size_t threadCount);
		LightPass(const LightPass& other) = default;
		LightPass(LightPass&& other) noexcept = default;
		~LightPass() = default;

		const char* getName() const noexcept override;

		FrameGraphPass& addToFrameGraph(FrameGraphBuilder& frameGraphBuilder, const Settings& settings);

		void updateResources(RenderFrame& renderFrame, CommandBuffer& commandBuffer) override;

		void setLightingModels(const std::vector<std::string>& lightingModelNames);
		void execute(FrameGraphContext& context, const Settings& settings);

		LightPass& operator=(const LightPass& other) = default;
		LightPass& operator=(LightPass&& other) noexcept = default;

	protected:
		void beginFrame() override;
		void endFrame() override;

	private:
		struct FrameResources
		{
			Ptr<DescriptorSet> descriptorSet;
			Ptr<Buffer> buffer;
		};

		void createLightingModel(const std::string& name);
		void createShaders();
		void frustumCull();
		void frustumCullElements(size_t index, size_t count, std::vector<const RenderLight*>& visibleLights) const;
		void drawElements(CommandBuffer& commandBuffer, const FrameResources& frameResources, bool applyEmissive, size_t directionalIndex, size_t directionalCount, size_t pointIndex, size_t pointCount, size_t spotIndex, size_t spotCount);

		RenderResourceManager* m_resourceManager;
		const GBuffer* m_gbuffer;
		const ShaderLibraryManager* m_shaderLibraryManager;
		size_t m_threadCount;

		Ptr<Sampler> m_gbufferSampler;
		Ptr<VertexBuffer> m_quadMesh;

		std::array<FrameResources, Renderer::FramesInFlight> m_frameResources;

		std::unordered_set<std::string> m_lightingModels;

		bool m_updateShader;
		bool m_useFrameSet;
		bool m_useLightSet;
		bool m_useLightShadowSet;
		Ptr<RenderMaterial> m_meshStencilMaterial;
		Ptr<RenderMaterial> m_meshMaterial;
		Ptr<RenderMaterial> m_meshShadowMaterial;
		Ptr<RenderMaterial> m_directionalMaterial;
		Ptr<RenderMaterial> m_directionalShadowMaterial;
		size_t m_gbufferEmissiveIndex;
		Ptr<RenderMaterial> m_lightEmissiveMaterial;

		std::map<size_t, std::string> m_gbufferOptions;
		std::vector<uint32_t> m_gbufferBindings;

		Ptr<Mesh> m_sphereMesh;
		Ptr<Mesh> m_coneMesh;

		std::vector<const RenderLight*> m_directionalLights;
		std::vector<const RenderLight*> m_pointLights;
		std::vector<const RenderLight*> m_spotLights;

		const DescriptorSet* m_gbufferSet;
		const DescriptorSet* m_emissiveSet;

		std::vector<Ptr<void>> m_oldResources;
	};
}

#endif