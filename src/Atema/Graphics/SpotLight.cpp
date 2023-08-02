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

#include <Atema/Graphics/SpotLight.hpp>

using namespace at;

SpotLight::SpotLight() :
	m_range(1.0f),
	m_angle(Math::toRadians(45.0f))
{
}

LightType SpotLight::getType() const noexcept
{
	return LightType::Spot;
}

void SpotLight::setPosition(const Vector3f& position)
{
	m_position = position;
}

const Vector3f& SpotLight::getPosition() const noexcept
{
	return m_position;
}

void SpotLight::setDirection(const Vector3f& direction)
{
	m_direction = direction.getNormalized();
}

const Vector3f& SpotLight::getDirection() const noexcept
{
	return m_direction;
}

void SpotLight::setRange(float range)
{
	m_range = range;
}

float SpotLight::getRange() const noexcept
{
	return m_range;
}

void SpotLight::setAngle(float angle)
{
	m_angle = angle;
}

float SpotLight::getAngle() const noexcept
{
	return m_angle;
}
