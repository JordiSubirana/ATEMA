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

#include "FirstPersonCameraSystem.hpp"
#include "../Components/CameraComponent.hpp"
#include "../Resources.hpp"
#include "../Settings.hpp"

using namespace at;

namespace
{
	const Vector3f frontVector(1.0f, 0.0f, 0.0f);
	const Vector3f rightVector(0.0f, -1.0f, 0.0f);
	const Vector3f upVector(0.0f, 0.0f, 1.0f);
}

FirstPersonCameraSystem::FirstPersonCameraSystem(const at::Ptr<at::RenderWindow>& renderWindow) :
	System(),
	m_window(renderWindow.get()),
	m_front(false),
	m_back(false),
	m_right(false),
	m_left(false),
	m_up(false),
	m_down(false),
	m_cameraRotationEnabled(false),
	m_initMousePosition(false)
{
}

FirstPersonCameraSystem::~FirstPersonCameraSystem()
{
}

void FirstPersonCameraSystem::update(TimeStep timeStep)
{
	// Get components
	CameraComponent* camera = nullptr;
	Transform* transform = nullptr;

	auto selection = getEntityManager().getUnion<Transform, CameraComponent>();

	for (auto& entity : selection)
	{
		camera = &selection.get<CameraComponent>(entity);

		if (!camera->isAuto)
		{
			transform = &selection.get<Transform>(entity);

			break;
		}
	}

	if (!camera || !transform)
		return;

	// Update components
	const auto rotationMatrix = Matrix4f::createRotation(transform->getRotation());

	Vector3f offset;

	if (m_front)
		offset += frontVector;
	if (m_back)
		offset -= frontVector;
	if (m_right)
		offset += rightVector;
	if (m_left)
		offset -= rightVector;
	if (m_up)
		offset += upVector;
	if (m_down)
		offset -= upVector;

	if (offset.getNorm() > 1e-6)
		offset.normalize();
	offset *= timeStep.getSeconds() * Settings::instance().cameraSpeed;
	offset = rotationMatrix.transformPosition(offset);

	transform->translate(offset);
}

void FirstPersonCameraSystem::onEvent(Event& event)
{
	CameraComponent* camera = nullptr;
	Transform* transform = nullptr;

	auto selection = getEntityManager().getUnion<Transform, CameraComponent>();

	for (auto& entity : selection)
	{
		camera = &selection.get<CameraComponent>(entity);

		if (!camera->isAuto)
		{
			transform = &selection.get<Transform>(entity);

			break;
		}
	}

	if (!camera || !transform)
		return;

	if (event.is<KeyEvent>())
	{
		const auto& keyEvent = static_cast<KeyEvent&>(event);

		// We don't care about repeat events
		if (keyEvent.state == KeyState::Repeat)
			return;

		const bool pressed = keyEvent.state == KeyState::Press;

		// Front
		if (keyEvent.key == Key::W)
			m_front = pressed;
		// Back
		else if (keyEvent.key == Key::S)
			m_back = pressed;
		// Left
		else if (keyEvent.key == Key::A)
			m_left = pressed;
		// Right
		else if (keyEvent.key == Key::D)
			m_right = pressed;
		// Up
		else if (keyEvent.key == Key::E)
			m_up = pressed;
		// Down
		else if (keyEvent.key == Key::Q)
			m_down = pressed;
	}
	else if (event.is<MouseButtonEvent>())
	{
		const auto& mouseEvent = static_cast<MouseButtonEvent&>(event);

		if (mouseEvent.button != MouseButton::Right)
			return;

		if (mouseEvent.state == MouseButtonState::Press)
		{
			m_cameraRotationEnabled = true;
			m_initMousePosition = true;
			m_lastPosition = mouseEvent.position;
		}
		else if (m_cameraRotationEnabled)
		{
			m_cameraRotationEnabled = false;
			rotate(*transform, mouseEvent.position);
		}

		m_window->setCursorEnabled(!m_cameraRotationEnabled);
	}
	else if (m_cameraRotationEnabled && event.is<MouseMoveEvent>())
	{
		const auto& mouseEvent = static_cast<MouseMoveEvent&>(event);

		rotate(*transform, mouseEvent.position);
	}
}

void FirstPersonCameraSystem::rotate(at::Transform& transform, const at::Vector2f& mousePosition)
{
	if (m_initMousePosition)
	{
		m_lastPosition = mousePosition;
		m_initMousePosition = false;
		return;
	}

	const auto delta = mousePosition - m_lastPosition;

	transform.rotate({ 0.0f, delta.y * cameraScale, -delta.x * cameraScale });

	m_lastPosition = mousePosition;
}
