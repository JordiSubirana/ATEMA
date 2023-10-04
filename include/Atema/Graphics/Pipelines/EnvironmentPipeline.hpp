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

#ifndef ATEMA_GRAPHICS_ENVIRONMENTPIPELINE_HPP
#define ATEMA_GRAPHICS_ENVIRONMENTPIPELINE_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/Passes/EquirectangularToCubemapPass.hpp>
#include <Atema/Graphics/Passes/EnvironmentIrradiancePass.hpp>
#include <Atema/Graphics/Passes/EnvironmentPrefilterPass.hpp>

namespace at
{
	class ATEMA_GRAPHICS_API EnvironmentPipeline
	{
	public:
		EnvironmentPipeline();
		EnvironmentPipeline(const EnvironmentPipeline& other) = delete;
		EnvironmentPipeline(EnvironmentPipeline&& other) noexcept = delete;
		virtual ~EnvironmentPipeline() = default;

		// Sets the environment map that will be the pipeline's input
		// environmentMap can be an equirectangular map or a cubemap
		void setInput(const Ptr<Image>& environmentMap);
		// Sets the optional outputs of the pipeline
		// The pipeline will adapt according to the output properties (size, mip levels, ...)
		// Every output must be a cubemap and have the flag BufferUsage::RenderTarget
		// environmentMap is generated if the input is equirectangular (mip level 0)
		// irradianceMap (mip level 0)
		// prefilteredMap's mip levels define how much roughness levels are desired
		void setOutput(const Ptr<Image>& environmentMap, const Ptr<Image>& irradianceMap, const Ptr<Image>& prefilteredMap);

		void beginRender();
		void updateResources(CommandBuffer& commandBuffer, RenderContext& renderContext);
		void render(CommandBuffer& commandBuffer, RenderContext& renderContext);

		EnvironmentPipeline& operator=(const EnvironmentPipeline& other) = delete;
		EnvironmentPipeline& operator=(EnvironmentPipeline&& other) noexcept = delete;

	private:
		void createFrameGraph();

		RenderResourceManager m_resourceManager;

		bool m_updateFrameGraph;
		Ptr<FrameGraph> m_environmentFrameGraph;
		Ptr<FrameGraph> m_processFrameGraph;

		UPtr<EquirectangularToCubemapPass> m_equirectangularToCubemapPass;
		UPtr<EnvironmentIrradiancePass> m_irradiancePass;
		std::vector<UPtr<EnvironmentPrefilterPass>> m_prefilterPasses;

		std::vector<AbstractRenderPass*> m_activePasses;

		Ptr<Image> m_inEnvironmentMap;
		Ptr<Image> m_outEnvironmentMap;
		Ptr<Image> m_outIrradianceMap;
		Ptr<Image> m_outPrefilteredMap;

		std::vector<Ptr<void>> m_oldResources;
	};
}

#endif
