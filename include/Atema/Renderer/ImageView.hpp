/*
	Copyright 2022 Jordi SUBIRANA

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

#ifndef ATEMA_RENDERER_IMAGEVIEW_HPP
#define ATEMA_RENDERER_IMAGEVIEW_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Math/Vector.hpp>
#include <Atema/Renderer/Enums.hpp>

namespace at
{
	class ATEMA_RENDERER_API ImageView : public NonCopyable
	{
	public:
		ImageView() = delete;
		virtual ~ImageView() = default;

		ImageFormat getFormat() const noexcept;

		uint32_t getBaseLayer() const noexcept;
		uint32_t getLayerCount() const noexcept;

		uint32_t getBaseMipLevel() const noexcept;
		uint32_t getMipLevelCount() const noexcept;

	protected:
		ImageView(ImageFormat format, uint32_t baseLayer, uint32_t layerCount, uint32_t baseMipLevel, uint32_t mipLevelCount);

	private:
		ImageFormat m_format;
		uint32_t m_baseLayer;
		uint32_t m_layerCount;
		uint32_t m_baseMipLevel;
		uint32_t m_mipLevelCount;
	};
}

#endif
