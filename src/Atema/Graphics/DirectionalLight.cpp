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

#include <Atema/Graphics/DirectionalLight.hpp>

using namespace at;

DirectionalLight::DirectionalLight() :
	m_direction(Vector3f(1.0f, 1.0f, -1.0f).normalize()),
	m_shadowMaxDepth(1000.0f)
{
}

LightType DirectionalLight::getType() const noexcept
{
	return LightType::Directional;
}

void DirectionalLight::setDirection(const Vector3f& direction)
{
	m_direction = direction.getNormalized();

	onLightDataUpdated();
}

const Vector3f& DirectionalLight::getDirection() const noexcept
{
	return m_direction;
}

void DirectionalLight::setShadowMaxDepth(float maxDepth)
{
	m_shadowMaxDepth = maxDepth;

	onShadowDataUpdated();
}

float DirectionalLight::getShadowMaxDepth() const noexcept
{
	return m_shadowMaxDepth;
}
