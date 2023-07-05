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

#include <Atema/Graphics/Light.hpp>
#include <Atema/Renderer/Renderer.hpp>

using namespace at;

Light::Light() :
	m_color(1.0f, 1.0f, 1.0f),
	m_ambientStrength(0.5f),
	m_diffuseStrength(0.5f),
	m_castShadows(false),
	m_shadowMapSize(DefaultShadowMapSize),
	m_shadowMapFormat(ImageFormat::D16_UNORM),
	m_shadowCascadeCount(1),
	m_shadowDepthBias(0.07f)
{
}

void Light::setColor(const Color& color)
{
	m_color = color;
}

const Color& Light::getColor() const noexcept
{
	return m_color;
}

void Light::setAmbientStrength(float strength)
{
	m_ambientStrength = strength;
}

void Light::setDiffuseStrength(float strength)
{
	m_diffuseStrength = strength;
}

float Light::getAmbientStrength() const noexcept
{
	return m_ambientStrength;
}

float Light::getDiffuseStrength() const noexcept
{
	return m_diffuseStrength;
}

void Light::setCastShadows(bool castShadows)
{
	m_castShadows = castShadows;
}

bool Light::castShadows() const noexcept
{
	return m_castShadows;
}

void Light::setShadowMapSize(uint32_t size)
{
	m_shadowMapSize = size;
}

uint32_t Light::getShadowMapSize() const noexcept
{
	return m_shadowMapSize;
}

void Light::setShadowMapFormat(ImageFormat format)
{
	ATEMA_ASSERT(Renderer::isDepthImageFormat(format), "Shadow mapping requires a depth ImageFormat");

	m_shadowMapFormat = format;
}

ImageFormat Light::getShadowMapFormat() const noexcept
{
	return m_shadowMapFormat;
}

void Light::setShadowCascadeCount(size_t cascadeCount)
{
	m_shadowCascadeCount = cascadeCount;
}

size_t Light::getShadowCascadeCount() const noexcept
{
	return m_shadowCascadeCount;
}

void Light::setShadowDepthBias(float depthBias)
{
	m_shadowDepthBias = depthBias;
}

float Light::getShadowDepthBias() const noexcept
{
	return m_shadowDepthBias;
}
