/*
	Copyright 2021 Jordi SUBIRANA

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

using namespace at;

namespace
{
	const Vector3f frontVector(1.0f, 0.0f, 0.0f);
	const Vector3f rightVector(0.0f, -1.0f, 0.0f);
}

FirstPersonCameraSystem::FirstPersonCameraSystem() :
	System(),
	m_front(false),
	m_back(false),
	m_right(false),
	m_left(false)
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
	const float metersPerSec = zoomOffset * 5.0f;
	const auto rotationMatrix = Matrix4f::createRotation(transform->getRotation());

	Vector3f frontOffset;
	if (m_front)
		frontOffset += frontVector;
	if (m_back)
		frontOffset -= frontVector;

	Vector3f sideOffset;
	if (m_right)
		sideOffset += rightVector;
	if (m_left)
		sideOffset -= rightVector;
	
	auto offset = frontOffset + sideOffset;
	if (offset.getNorm() > 1e-6)
		offset.normalize();
	offset *= timeStep.getSeconds() * metersPerSec;
	offset = rotationMatrix * offset;

	transform->translate(offset);
}

void FirstPersonCameraSystem::onEvent(Event& event)
{
	//const Vector3f frontVector(0.0f, 1.0f, 0.0f);
	//const Vector3f rightVector(1.0f, 0.0f, 0.0f);
	const Vector3f frontVector(1.0f, 0.0f, 0.0f);
	const Vector3f rightVector(0.0f, -1.0f, 0.0f);

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
		auto& keyEvent = static_cast<KeyEvent&>(event);

		// We don't care about repeat events
		if (keyEvent.state == KeyState::Repeat)
			return;

		bool pressed = keyEvent.state == KeyState::Press;

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
	}
	else if (event.is<MouseMoveEvent>())
	{
		auto& mouseEvent = static_cast<MouseMoveEvent&>(event);

		const auto& position = mouseEvent.position;

		const auto delta = position - m_lastPosition;

		//transform->rotate({ delta.x * scale, delta.y * scale, 0.0f });
		transform->rotate({ 0.0f, delta.y * cameraScale, - delta.x * cameraScale });

		m_lastPosition = position;
	}
}