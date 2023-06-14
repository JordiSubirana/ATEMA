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

#include <Atema/Graphics/Camera.hpp>

using namespace at;

Camera::Camera() :
	m_position(0.0f, 0.0f, 0.0f),
	m_direction(0.0f, 1.0f, 0.0f),
	m_up(0.0f, 0.0f, 1.0f),
	m_nearPlane(0.1f),
	m_farPlane(1000.0f),
	m_viewport({0.0f, 0.0f}, {512.0f, 512.0f}),
	m_scissor(512, 512)
{
	updateViewMatrix();
}

void Camera::setPosition(const Vector3f& position)
{
	m_position = position;

	updateViewMatrix();

	updateMatrixAndFrustum();
}

void Camera::setDirection(const Vector3f& direction)
{
	m_direction = direction;

	updateViewMatrix();

	updateMatrixAndFrustum();
}

void Camera::setUp(const Vector3f& up)
{
	m_up = up;

	updateViewMatrix();

	updateMatrixAndFrustum();
}

void Camera::setNearPlane(float nearPlane)
{
	m_nearPlane = nearPlane;

	updateProjectionMatrix();
}

void Camera::setFarPlane(float farPlane)
{
	m_farPlane = farPlane;

	updateProjectionMatrix();
}

void Camera::setViewport(const Viewport& viewport)
{
	m_viewport = viewport;
}

void Camera::setScissor(const Recti& scissor)
{
	m_scissor = scissor;
}

void Camera::setView(const Vector3f& position, const Vector3f& direction, const Vector3f& up)
{
	m_position = position;
	m_direction = direction;
	m_up = up;

	updateViewMatrix();

	updateMatrixAndFrustum();
}

const Vector3f& Camera::getPosition() const noexcept
{
	return m_position;
}

const Vector3f& Camera::getDirection() const noexcept
{
	return m_direction;
}

const Vector3f& Camera::getUp() const noexcept
{
	return m_up;
}

float Camera::getNearPlane() const noexcept
{
	return m_nearPlane;
}

float Camera::getFarPlane() const noexcept
{
	return m_farPlane;
}

const Viewport& Camera::getViewport() const noexcept
{
	return m_viewport;
}

const Recti& Camera::getScissor() const noexcept
{
	return m_scissor;
}

const Matrix4f& Camera::getMatrix() const noexcept
{
	return m_matrix;
}

const Matrix4f& Camera::getViewMatrix() const noexcept
{
	return m_viewMatrix;
}

const Frustumf& Camera::getFrustum() const noexcept
{
	return m_frustum;
}

void Camera::updateMatrixAndFrustum()
{
	m_matrix = getProjectionMatrix() * m_viewMatrix;

	m_frustum.set(m_matrix);
}

void Camera::updateViewMatrix()
{
	m_viewMatrix = Matrix4f::createLookAt(m_position, m_position + m_direction, m_up);
}
