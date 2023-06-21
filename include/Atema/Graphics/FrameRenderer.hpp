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

#ifndef ATEMA_GRAPHICS_FRAMERENDERER_HPP
#define ATEMA_GRAPHICS_FRAMERENDERER_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/AbstractFrameRenderer.hpp>
#include <Atema/Graphics/Passes/DebugFrameGraphPass.hpp>
#include <Atema/Graphics/Passes/DebugRendererPass.hpp>
#include <Atema/Graphics/Passes/GBufferPass.hpp>
#include <Atema/Graphics/Passes/PhongLightingPass.hpp>
#include <Atema/Graphics/Passes/ScreenPass.hpp>

namespace at
{
	class ShadowPass;
	
	class ATEMA_GRAPHICS_API FrameRenderer : public AbstractFrameRenderer
	{
	public:
		FrameRenderer();
		FrameRenderer(const FrameRenderer& other) = delete;
		FrameRenderer(FrameRenderer&& other) noexcept = default;
		~FrameRenderer() = default;

		void enableDebugRenderer(bool enable);
		void enableDebugFrameGraph(bool enable);

		FrameRenderer& operator=(const FrameRenderer& other) = delete;
		FrameRenderer& operator=(FrameRenderer&& other) noexcept = default;

	protected:
		void createFrameGraph() override;
		FrameGraph& getFrameGraph() override;
		std::vector<AbstractRenderPass*>& getRenderPasses() override;
		void destroyResources(RenderFrame& renderFrame) override;
		void doBeginFrame() override;

	private:
		void createPasses();
		void updateShadowData();

		Ptr<FrameGraph> m_frameGraph;
		
		std::vector<AbstractRenderPass*> m_activePasses;

		std::vector<UPtr<AbstractRenderPass>> m_oldRenderPasses;
		std::vector<Ptr<FrameGraph>> m_oldFrameGraphs;

		UPtr<GBufferPass> m_gbufferPass;
		std::vector<UPtr<ShadowPass>> m_shadowPasses;
		UPtr<PhongLightingPass> m_phongLightingPass;
		UPtr<DebugRendererPass> m_debugRendererPass;
		UPtr<DebugFrameGraphPass> m_debugFrameGraphPass;
		UPtr<ScreenPass> m_screenPass;

		std::vector<PhongLightingPass::ShadowCascadeData> m_shadowCascadeData;

		bool m_enableDebugRenderer;
		bool m_enableDebugFrameGraph;

		// Shadows
		uint32_t m_currentShadowMapSize;
		uint32_t m_shadowMapSize;
		Ptr<Image> m_shadowMap;
	};
}

#endif