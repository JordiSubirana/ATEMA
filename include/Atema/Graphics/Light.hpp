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

#ifndef ATEMA_GRAPHICS_LIGHT_HPP
#define ATEMA_GRAPHICS_LIGHT_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/Enums.hpp>
#include <Atema/Renderer/Color.hpp>
#include <Atema/Renderer/Enums.hpp>
#include <Atema/Core/Signal.hpp>

namespace at
{
	class ATEMA_GRAPHICS_API Light
	{
	public:
		static constexpr uint32_t DefaultShadowMapSize = 1024;

		Light();
		Light(const Light& other) = default;
		Light(Light&& other) noexcept = default;
		virtual ~Light() = default;

		virtual LightType getType() const noexcept = 0;

		void setColor(const Color& color);
		const Color& getColor() const noexcept;

		void setAmbientStrength(float strength);
		void setDiffuseStrength(float strength);
		float getAmbientStrength() const noexcept;
		float getDiffuseStrength() const noexcept;

		void setCastShadows(bool castShadows);
		bool castShadows() const noexcept;

		// Will generate a size*size shadow map
		void setShadowMapSize(uint32_t size);
		uint32_t getShadowMapSize() const noexcept;

		// Default : ImageFormat::D16_UNORM
		void setShadowMapFormat(ImageFormat format);
		ImageFormat getShadowMapFormat() const noexcept;

		// How many cascades are used in the shadow map
		// Default : 1
		void setShadowCascadeCount(size_t cascadeCount);
		size_t getShadowCascadeCount() const noexcept;

		void setShadowDepthBias(float depthBias);
		float getShadowDepthBias() const noexcept;

		Light& operator=(const Light& other) = default;
		Light& operator=(Light&& other) noexcept = default;

		Signal<> onShadowMapDataChanged;

	private:
		Color m_color;
		float m_ambientStrength;
		float m_diffuseStrength;
		bool m_castShadows;
		uint32_t m_shadowMapSize;
		ImageFormat m_shadowMapFormat;
		size_t m_shadowCascadeCount;
		float m_shadowDepthBias;
	};
}

#endif
