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

#include <Atema/Graphics/PerspectiveCamera.hpp>

using namespace at;

PerspectiveCamera::PerspectiveCamera() :
	Camera(),
	m_fov(70.0f),
	m_aspectRatio(1.0f)
{
	updateProjectionMatrix();
}

void PerspectiveCamera::setFOV(float fov)
{
	m_fov = fov;

	updateProjectionMatrix();
}

void PerspectiveCamera::setAspectRatio(float aspectRatio)
{
	m_aspectRatio = aspectRatio;

	updateProjectionMatrix();
}

float PerspectiveCamera::getFOV() const noexcept
{
	return m_fov;
}

float PerspectiveCamera::getAspectRatio() const noexcept
{
	return m_aspectRatio;
}

const Matrix4f& PerspectiveCamera::getProjectionMatrix() const noexcept
{
	return m_projectionMatrix;
}

void PerspectiveCamera::updateProjectionMatrix()
{
	m_projectionMatrix = Matrix4f::createPerspective(Math::toRadians(m_fov), m_aspectRatio, getNearPlane(), getFarPlane());
	m_projectionMatrix[1][1] *= -1;

	updateMatrixAndFrustum();
}
