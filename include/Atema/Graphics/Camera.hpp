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

#ifndef ATEMA_GRAPHICS_CAMERA_HPP
#define ATEMA_GRAPHICS_CAMERA_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/Enums.hpp>
#include <Atema/Renderer/Viewport.hpp>
#include <Atema/Math/Vector.hpp>
#include <Atema/Math/Matrix.hpp>
#include <Atema/Math/Frustum.hpp>
#include <Atema/Math/Rect.hpp>

namespace at
{
	class ATEMA_GRAPHICS_API Camera
	{
	public:
		Camera();
		Camera(const Camera& other) = default;
		Camera(Camera&& other) noexcept = default;
		virtual ~Camera() = default;

		// Sets the eye position
		void setPosition(const Vector3f& position);
		// Sets the direction the camera is looking at
		void setDirection(const Vector3f& direction);
		// Sets the up vector
		void setUp(const Vector3f& up);
		// Sets the distance between the eye & the near plane
		void setNearPlane(float nearPlane);
		// Sets the distance between the eye & the far plane
		void setFarPlane(float farPlane);
		void setViewport(const Viewport& viewport);
		void setScissor(const Recti& scissor);

		void setView(const Vector3f& position, const Vector3f& direction, const Vector3f& up);

		const Vector3f& getPosition() const noexcept;
		const Vector3f& getDirection() const noexcept;
		const Vector3f& getUp() const noexcept;
		float getNearPlane() const noexcept;
		float getFarPlane() const noexcept;
		const Viewport& getViewport() const noexcept;
		const Recti& getScissor() const noexcept;

		// View-projection matrix
		const Matrix4f& getMatrix() const noexcept;
		const Matrix4f& getViewMatrix() const noexcept;
		virtual const Matrix4f& getProjectionMatrix() const noexcept = 0;

		const Frustumf& getFrustum() const noexcept;

		Camera& operator=(const Camera& other) = default;
		Camera& operator=(Camera&& other) noexcept = default;

	protected:
		// This is called when near/far values change
		// Must call updateMatrixAndFrustum
		virtual void updateProjectionMatrix() = 0;
		// This must be called when the projection matrices changes
		void updateMatrixAndFrustum();

	private:
		void updateViewMatrix();

		Vector3f m_position;
		Vector3f m_direction;
		Vector3f m_up;

		float m_nearPlane;
		float m_farPlane;

		Viewport m_viewport;
		Recti m_scissor;

		Matrix4f m_matrix;
		Matrix4f m_viewMatrix;
		Frustumf m_frustum;
	};
}

#endif