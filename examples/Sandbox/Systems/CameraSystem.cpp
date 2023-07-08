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

#include "CameraSystem.hpp"

#include <Atema/Window/WindowResizeEvent.hpp>

#include "../Components/CameraComponent.hpp"
#include "../Components/GraphicsComponent.hpp"
#include "../Resources.hpp"
#include "../Scene.hpp"

using namespace at;

CameraSystem::CameraSystem(const at::Ptr<at::RenderWindow>& renderWindow) :
	System(),
	m_window(renderWindow.get()),
	m_totalTime(0.0f),
	m_size(renderWindow->getSize())
{
}

CameraSystem::~CameraSystem()
{
}

void CameraSystem::update(TimeStep timeStep)
{
	AABBf sceneAABB = Scene::instance().getAABB();
	AABBf objectAABB;
	{
		auto& entityManager = getEntityManager();
		auto entities = entityManager.getUnion<Transform, GraphicsComponent>();

		for (auto& entity : entities)
		{
			auto& graphics = entityManager.getComponent<GraphicsComponent>(entity);

			// Don't consider the ground
			if (graphics.staticModel->getModel()->getAABB().getSize().z > 0.1f)
			{
				objectAABB = graphics.staticModel->getModel()->getAABB();
				break;
			}
		}
	}

	auto sceneSize = sceneAABB.getSize();
	sceneSize.z = 0.0f;
	auto objectSize = objectAABB.getSize();
	objectSize.z = 0.0f;

	const auto sceneRadius = sceneSize.getNorm() / 2.0f;
	const auto objectRadius = objectSize.getNorm() / 2.0f;

	const auto radiusMargin = objectRadius;

	// Update automatic cameras
	auto selection = getEntityManager().getUnion<Transform, CameraComponent>();

	for (auto& entity : selection)
	{
		auto& camera = selection.get<CameraComponent>(entity);

		camera.aspectRatio = static_cast<float>(m_size.x) / static_cast<float>(m_size.y);

		if (camera.isAuto)
		{
			camera.target = sceneAABB.getCenter();
			camera.target.z = sceneAABB.max.z * 0.5f;

			auto& transform = selection.get<Transform>(entity);

			const auto metersPerSec = objectRadius * 2.0f * Math::Pi<float>;
			auto perimeter = sceneRadius * 2.0f * Math::Pi<float>;
			perimeter = std::sqrt(perimeter * 2.0f);
			const auto loopsPerSec = metersPerSec / perimeter * 0.5f;
			const auto angle = m_totalTime * loopsPerSec * Math::Pi<float>;

			const auto sin = std::sin(angle);
			const auto sinSlow = std::sin(angle / 2.0f);

			const auto percent = (sin + 1.0f) / 2.0f;
			const auto percentSlow = (sinSlow + 1.0f) / 2.0f;

			const auto radius = percent * sceneRadius * 2.0f + (1.0f - percent) * objectRadius;

			const auto radiusPercent = (radius - objectRadius) / (sceneRadius * 2.0f - objectRadius);

			const auto pos = toCartesian({ radius + radiusMargin, angle });

			auto z = percentSlow * objectAABB.max.z * 2.0f + (1.0f - percentSlow) * sceneRadius;

			z = Math::lerp(objectAABB.max.z * 1.0f, z, radiusPercent);

			transform.setTranslation({ pos.x + camera.target.x, pos.y + camera.target.y, z });
		}
	}

	m_totalTime += timeStep.getSeconds();
}

void CameraSystem::onEvent(Event& event)
{
	if (event.is<KeyEvent>())
	{
		auto& keyEvent = static_cast<KeyEvent&>(event);

		if (keyEvent.key == Key::Space && keyEvent.state == KeyState::Press)
		{
			auto selection = getEntityManager().getUnion<Transform, CameraComponent>();

			// Enable camera if we have only one camera
			if (selection.size() == 1)
			{
				auto& camera = selection.get<CameraComponent>(*selection.begin());

				camera.display = true;
			}
			// Toggle cameras if we have multiple cameras
			else
			{
				for (auto& entity : selection)
				{
					auto& camera = selection.get<CameraComponent>(entity);

					camera.display = !camera.display;
				}
			}
		}
	}
	else if (event.is<WindowResizeEvent>())
	{
		auto& windowResizeEvent = static_cast<WindowResizeEvent&>(event);

		if (windowResizeEvent.window == m_window)
			m_size = windowResizeEvent.size;
	}
}
