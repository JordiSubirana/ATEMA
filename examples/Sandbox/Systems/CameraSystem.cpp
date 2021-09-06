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

#include "CameraSystem.hpp"
#include "../Components/CameraComponent.hpp"
#include "../Resources.hpp"

using namespace at;

CameraSystem::CameraSystem() :
	System(),
	m_totalTime(0.0f)
{
}

CameraSystem::~CameraSystem()
{
}

void CameraSystem::update(TimeStep timeStep)
{
	const auto windowSize = Renderer::instance().getMainWindow()->getSize();

	// Update automatic cameras
	auto selection = getEntityManager().getUnion<Transform, CameraComponent>();

	for (auto& entity : selection)
	{
		auto& camera = selection.get<CameraComponent>(entity);

		camera.aspectRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);

		if (camera.isAuto)
		{
			auto& transform = selection.get<Transform>(entity);
			
			const auto angle = m_totalTime * zoomSpeed;

			const auto sin = std::sin(angle);
			const auto sinSlow = std::sin(angle / 2.0f + 3.14159f);

			const auto sign = (sin + 1.0f) / 2.0f;
			const auto signSlow = (sinSlow + 1.0f) / 2.0f;

			auto radius = modelScale * objectRow;
			radius = sign * radius + (1.0f - sign) * zoomOffset;

			const auto pos = toCartesian({ radius, angle / 3.0f });

			const auto z = signSlow * radius + (1.0f - signSlow) * zoomOffset;

			transform.setTranslation({ pos.x, pos.y, z });
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
}
