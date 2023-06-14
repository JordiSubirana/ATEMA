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

#include <Atema/Graphics/OrthographicCamera.hpp>

using namespace at;

OrthographicCamera::OrthographicCamera() :
	Camera(),
	m_size(512.0f, 512.0f)
{
	updateProjectionMatrix();
}

void OrthographicCamera::setSize(const Vector2f& size)
{
	m_size = size;

	updateProjectionMatrix();
}

const Vector2f& OrthographicCamera::getSize() const noexcept
{
	return m_size;
}

const Matrix4f& OrthographicCamera::getProjectionMatrix() const noexcept
{
	return m_projectionMatrix;
}

void OrthographicCamera::updateProjectionMatrix()
{
	const auto halfSize = m_size / 2.0f;

	m_projectionMatrix = Matrix4f::createOrtho(-halfSize.x, halfSize.x, -halfSize.y, halfSize.y, getNearPlane(), getFarPlane());
	m_projectionMatrix[1][1] *= -1;

	updateMatrixAndFrustum();
}
