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

#include <Atema/Renderer/ImageView.hpp>

using namespace at;

ImageView::ImageView(ImageFormat format, uint32_t baseLayer, uint32_t layerCount, uint32_t baseMipLevel, uint32_t mipLevelCount) :
	m_format(format),
	m_baseLayer(baseLayer),
	m_layerCount(layerCount),
	m_baseMipLevel(baseMipLevel),
	m_mipLevelCount(mipLevelCount)
{
}

ImageFormat ImageView::getFormat() const noexcept
{
	return m_format;
}

uint32_t ImageView::getBaseLayer() const noexcept
{
	return m_baseLayer;
}

uint32_t ImageView::getLayerCount() const noexcept
{
	return m_layerCount;
}

uint32_t ImageView::getBaseMipLevel() const noexcept
{
	return m_baseMipLevel;
}

uint32_t ImageView::getMipLevelCount() const noexcept
{
	return m_mipLevelCount;
}
