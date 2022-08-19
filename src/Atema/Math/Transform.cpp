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

#include <Atema/Math/Transform.hpp>

using namespace at;

Transform::Transform() :
	m_scale(1.0f, 1.0f, 1.0f),
	m_transformValid(false),
	m_transform(1.0f)
{
}

Transform::Transform(const Vector3f& translation, const Vector3f& rotation, const Vector3f& scale) :
	Transform()
{
	set(translation, rotation, scale);
}

Transform::~Transform()
{
}

Transform& Transform::translate(const Vector3f& translation)
{
	setTranslation(m_translation + translation);

	return *this;
}

Transform& Transform::rotate(const Vector3f& rotation)
{
	setRotation(m_rotation + rotation);

	return *this;
}

Transform& Transform::scale(const Vector3f& scale)
{
	setScale(m_scale + scale);

	return *this;
}

void Transform::set(const Vector3f& translation, const Vector3f& rotation, const Vector3f& scale)
{
	m_translation = translation;
	m_rotation = rotation;
	m_scale = scale;

	m_transformValid = false;
}

void Transform::setTranslation(const Vector3f& translation)
{
	m_translation = translation;

	m_transformValid = false;
}

void Transform::setRotation(const Vector3f& rotation)
{
	m_rotation = rotation;

	m_transformValid = false;
}

void Transform::setScale(const Vector3f& scale)
{
	m_scale = scale;

	m_transformValid = false;
}

const Vector3f& Transform::getTranslation() const noexcept
{
	return m_translation;
}

const Vector3f& Transform::getRotation() const noexcept
{
	return m_rotation;
}

const Vector3f& Transform::getScale() const noexcept
{
	return m_scale;
}

const Matrix4f& Transform::getMatrix() const noexcept
{
	if (!m_transformValid)
	{
		m_transform = Matrix4f::createTranslation(m_translation) * Matrix4f::createRotation(m_rotation) * Matrix4f::createScale(m_scale);
		m_transformValid = true;
	}

	return m_transform;
}
