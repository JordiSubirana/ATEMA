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

#ifndef ATEMA_GRAPHICS_EQUIRECTANGULARTOCUBEMAPPASS_CPP
#define ATEMA_GRAPHICS_EQUIRECTANGULARTOCUBEMAPPASS_CPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/Passes/AbstractCubemapPass.hpp>
#include <Atema/Graphics/RenderResourceManager.hpp>

namespace at
{
	struct EquirectangularToCubemapFrameGraphSettings : public AbstractCubemapPass::BaseFrameGraphSettings
	{
		FrameGraphTextureHandle environmentMap = FrameGraph::InvalidTextureHandle;
	};

	class ATEMA_GRAPHICS_API EquirectangularToCubemapPass : public CubemapPass<EquirectangularToCubemapFrameGraphSettings>
	{
	public:
		EquirectangularToCubemapPass() = delete;
		EquirectangularToCubemapPass(RenderResourceManager& resourceManager);
		EquirectangularToCubemapPass(const EquirectangularToCubemapPass& other) = default;
		EquirectangularToCubemapPass(EquirectangularToCubemapPass&& other) noexcept = default;
		~EquirectangularToCubemapPass() = default;

		const char* getName() const noexcept override;

		EquirectangularToCubemapPass& operator=(const EquirectangularToCubemapPass& other) = default;
		EquirectangularToCubemapPass& operator=(EquirectangularToCubemapPass&& other) noexcept = default;

	protected:
		void initializeFrameGraphPass(FrameGraphPass& frameGraphPass, FrameGraphBuilder& frameGraphBuilder, const FrameGraphSettings& settings) override;
		bool bindResources(FrameGraphContext& context, const FrameGraphSettings& settings) override;

	private:
		Ptr<Sampler> m_sampler;
	};
}

#endif
