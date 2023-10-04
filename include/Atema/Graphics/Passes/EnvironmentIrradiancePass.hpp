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

#ifndef ATEMA_GRAPHICS_ENVIRONMENTPROCESSPASS_HPP
#define ATEMA_GRAPHICS_ENVIRONMENTPROCESSPASS_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/Passes/AbstractCubemapPass.hpp>
#include <Atema/Graphics/RenderResourceManager.hpp>

namespace at
{
	class Sampler;

	struct EnvironmentIrradianceFrameGraphSettings : public AbstractCubemapPass::BaseFrameGraphSettings
	{
		// Optional environment cubemap faces used for synchronization
		std::vector<FrameGraphTextureHandle> environmentMapFaces;
	};

	class ATEMA_GRAPHICS_API EnvironmentIrradiancePass : public CubemapPass<EnvironmentIrradianceFrameGraphSettings>
	{
	public:
		EnvironmentIrradiancePass() = delete;
		EnvironmentIrradiancePass(RenderResourceManager& resourceManager);
		EnvironmentIrradiancePass(const EnvironmentIrradiancePass& other) = default;
		EnvironmentIrradiancePass(EnvironmentIrradiancePass&& other) noexcept = default;
		~EnvironmentIrradiancePass() = default;
		
		const char* getName() const noexcept override;

		// Default : 1024
		void setSampleCount(uint32_t sampleCount);
		uint32_t getSampleCount() const noexcept;

		// Default : 1.0
		void setMipLevelOffset(float mipLevelOffset);
		float getMipLevelOffset() const noexcept;

		// Must remain valid until the rendering
		void setEnvironmentMap(Image& environmentMap);

		void updateResources(CommandBuffer& commandBuffer) override;

		EnvironmentIrradiancePass& operator=(const EnvironmentIrradiancePass& other) = default;
		EnvironmentIrradiancePass& operator=(EnvironmentIrradiancePass&& other) noexcept = default;

	protected:
		void initializeFrameGraphPass(FrameGraphPass& frameGraphPass, FrameGraphBuilder& frameGraphBuilder, const FrameGraphSettings& settings) override;
		bool bindResources(FrameGraphContext& context, const FrameGraphSettings& settings) override;

	private:
		RenderResourceManager* m_resourceManager;

		bool m_updateResources;
		uint32_t m_sampleCount;
		float m_mipLevelOffset;
		uint32_t m_resolution;
		Image* m_environmentMap;

		Ptr<BufferAllocation> m_irradianceBuffer;
		Ptr<DescriptorSet> m_irradianceDescriptorSet;

		Ptr<Sampler> m_sampler;
	};
}

#endif
