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

#ifndef ATEMA_GRAPHICS_LIGHTRESOURCE_HPP
#define ATEMA_GRAPHICS_LIGHTRESOURCE_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/Light.hpp>
#include <Atema/Graphics/RenderResource.hpp>
#include <Atema/Graphics/ShaderData.hpp>
#include <Atema/Renderer/Sampler.hpp>

namespace at
{
	class ATEMA_GRAPHICS_API RenderLight : public RenderResource
	{
	public:
		RenderLight() = delete;
		RenderLight(const Light& light);
		RenderLight(const RenderLight& other) = default;
		RenderLight(RenderLight&& other) noexcept = default;
		~RenderLight() = default;

		void setShadowData(const std::vector<ShadowData>& cascades);

		void update(RenderFrame& renderFrame, CommandBuffer& commandBuffer) override;

		const Light& getLight() const noexcept;

		const DescriptorSet& getLightDescriptorSet() const noexcept;
		const DescriptorSet& getShadowDescriptorSet() const noexcept;

		const Ptr<Image>& getShadowMap() const noexcept;

		RenderLight& operator=(const RenderLight& other) = default;
		RenderLight& operator=(RenderLight&& other) noexcept = default;

	private:
		void createShadowResources();

		const Light& m_light;

		Ptr<Buffer> m_lightBuffer;
		Ptr<DescriptorSet> m_lightDescriptorSet;

		Ptr<Buffer> m_shadowBuffer;
		Ptr<DescriptorSet> m_shadowDescriptorSet;

		Ptr<Image> m_shadowMap;
		Ptr<Sampler> m_sampler;

		bool m_updateShadowData;
		CascadedShadowData m_shadowData;
	};
}

#endif
