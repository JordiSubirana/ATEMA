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

#include "GraphicsSystem.hpp"

#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Window/WindowResizeEvent.hpp>
#include <Atema/Graphics/DefaultLightingModels.hpp>

#include "../Resources.hpp"
#include "../Settings.hpp"
#include "../Components/GraphicsComponent.hpp"
#include "../Components/CameraComponent.hpp"
#include "../Components/LightComponent.hpp"

#include <fstream>

using namespace at;

namespace
{
	constexpr size_t targetThreadCount = 8;

	const size_t threadCount = std::min(targetThreadCount, TaskManager::instance().getSize());
}

GraphicsSystem::GraphicsSystem(const Ptr<RenderWindow>& renderWindow) :
	System(),
	m_renderWindow(renderWindow),
	m_baseDepthBias(Settings::instance().baseDepthBias),
	m_shadowMapSize(Settings::instance().shadowMapSize),
	m_shadowCascadeCount(Settings::instance().shadowCascadeCount),
	m_frustumRotation(0.0f)
{
	ATEMA_ASSERT(renderWindow, "Invalid RenderWindow");

	Graphics::instance().addLightingModel(DefaultLightingModels::getPhong());
	Graphics::instance().addLightingModel(DefaultLightingModels::getEmissive());

	m_frameRenderer.getRenderScene().setCamera(m_camera);

	// Remove unused resources if they are not used during 3 cycles (arbitrary)
	Graphics::instance().setMaxUnusedCounter(static_cast<uint32_t>(Renderer::FramesInFlight) * 3);

	// Create size dependent resources
	onResize(renderWindow->getSize());
}

GraphicsSystem::~GraphicsSystem()
{
	Renderer::instance().waitForIdle();

	Graphics::instance().clear();
}

void GraphicsSystem::update(TimeStep timeStep)
{
	updateRenderables();

	// Ensure the settings did not change
	checkSettings();

	// Update frustum rotation
	m_frustumRotation += (Math::Pi<float> / 4.0f) * timeStep.getSeconds();

	// Start frame
	updateFrame();

	// Clear resources that have been unused for too long
	Graphics::instance().clearUnused();
}

void GraphicsSystem::onEvent(Event& event)
{
	if (event.is<WindowResizeEvent>())
	{
		const auto& windowResizeEvent = static_cast<WindowResizeEvent&>(event);

		onResize(windowResizeEvent.size);
	}
}

void GraphicsSystem::onEntityAdded(at::EntityHandle entity)
{
	auto& entityManager = getEntityManager();
	auto& renderScene = m_frameRenderer.getRenderScene();

	if (entityManager.hasComponent<GraphicsComponent>(entity))
	{
		auto& graphics = entityManager.getComponent<GraphicsComponent>(entity);

		renderScene.addRenderable(*graphics.staticModel);
	}

	if (entityManager.hasComponent<LightComponent>(entity))
	{
		auto& light = entityManager.getComponent<LightComponent>(entity);

		renderScene.addLight(*light.light);
	}
}

void GraphicsSystem::onEntityRemoved(at::EntityHandle entity)
{
	auto& entityManager = getEntityManager();
	auto& renderScene = m_frameRenderer.getRenderScene();

	if (entityManager.hasComponent<GraphicsComponent>(entity))
	{
		auto& graphics = entityManager.getComponent<GraphicsComponent>(entity);

		renderScene.removeRenderable(*graphics.staticModel);

		destroyAfterUse(std::move(graphics.staticModel));
	}

	if (entityManager.hasComponent<LightComponent>(entity))
	{
		auto& light = entityManager.getComponent<LightComponent>(entity);

		renderScene.removeLight(*light.light);

		destroyAfterUse(std::move(light.light));
	}
}

void GraphicsSystem::checkSettings()
{
	const auto& settings = Settings::instance();

	if (!Math::equals(m_baseDepthBias, settings.baseDepthBias))
	{
		m_baseDepthBias = settings.baseDepthBias;

		auto lightEntities = getEntityManager().getUnion<LightComponent>();

		for (auto& entity : lightEntities)
		{
			auto& lightComponent = lightEntities.get<LightComponent>(entity);
			lightComponent.light->setShadowDepthBias(m_baseDepthBias);
		}
	}

	if (m_shadowMapSize != settings.shadowMapSize)
	{
		m_shadowMapSize = settings.shadowMapSize;

		auto lightEntities = getEntityManager().getUnion<LightComponent>();

		for (auto& entity : lightEntities)
		{
			auto& lightComponent = lightEntities.get<LightComponent>(entity);
			lightComponent.light->setShadowMapSize(m_shadowMapSize);
		}
	}

	if (m_shadowCascadeCount != settings.shadowCascadeCount)
	{
		m_shadowCascadeCount = settings.shadowCascadeCount;

		auto lightEntities = getEntityManager().getUnion<LightComponent>();

		for (auto& entity : lightEntities)
		{
			auto& lightComponent = lightEntities.get<LightComponent>(entity);
			lightComponent.light->setShadowCascadeCount(m_shadowCascadeCount);
		}
	}

	m_frameRenderer.enableDebugRenderer(settings.enableDebugRenderer);
	m_frameRenderer.enableDebugGBuffer(settings.enableDebugGBuffer);
	m_frameRenderer.enableDebugShadowMaps(settings.enableDebugShadowMaps);
}

void GraphicsSystem::onResize(const Vector2u& size)
{
	Viewport viewport;
	viewport.size.x = static_cast<float>(size.x);
	viewport.size.y = static_cast<float>(size.y);

	m_frameRenderer.resize(size);

	m_camera.setViewport(viewport);
	m_camera.setScissor(Recti(size.x, size.y));
}

void GraphicsSystem::updateFrame()
{
	updateCamera();

	m_frameRenderer.initializeFrame();

	Benchmark benchmark("RenderWindow::acquireFrame");

	auto& renderFrame = m_renderWindow.lock()->acquireFrame();

	benchmark.stop();

	destroyPendingResources(renderFrame);

	m_frameRenderer.render(renderFrame);
}

void GraphicsSystem::updateRenderables()
{
	ATEMA_BENCHMARK("Update renderables");

	auto entities = getEntityManager().getUnion<Transform, GraphicsComponent>();

	auto& taskManager = TaskManager::instance();

	std::vector<Ptr<Task>> tasks;
	tasks.reserve(threadCount);

	size_t firstIndex = 0;
	const size_t size = entities.size() / threadCount;

	for (size_t taskIndex = 0; taskIndex < threadCount; taskIndex++)
	{
		auto lastIndex = firstIndex + size;

		if (taskIndex == threadCount - 1)
		{
			const auto remainingSize = entities.size() - lastIndex;

			lastIndex += remainingSize;
		}

		auto task = taskManager.createTask([this, &entities, firstIndex, lastIndex](size_t threadIndex)
			{
				for (auto it = entities.begin() + firstIndex; it != entities.begin() + lastIndex; it++)
				{
					auto& transform = entities.get<Transform>(*it);
					auto& graphics = entities.get<GraphicsComponent>(*it);

					graphics.staticModel->setTransform(transform);
				}
			});

		tasks.push_back(task);

		firstIndex += size;
	}

	for (auto& task : tasks)
		task->wait();
}

void GraphicsSystem::updateCamera()
{
	ATEMA_BENCHMARK("Update camera");

	auto entities = getEntityManager().getUnion<Transform, CameraComponent>();

	for (auto& entity : entities)
	{
		auto& camera = entities.get<CameraComponent>(entity);

		if (camera.display)
		{
			auto& transform = entities.get<Transform>(entity);

			Vector3f cameraPos = transform.getTranslation();
			const Vector3f cameraUp(0.0f, 0.0f, 1.0f);

			Vector3f cameraTarget = camera.target;

			// If the camera doesn't use target, calculate target from transform rotation
			if (!camera.useTarget)
			{
				cameraTarget = cameraPos + Matrix4f::createRotation(transform.getRotation()).transformVector({ 1.0f, 0.0f, 0.0f });
			}

			m_camera.setView(cameraPos, (cameraTarget - cameraPos).normalize(), cameraUp);
			m_camera.setFOV(camera.fov);
			m_camera.setAspectRatio(camera.aspectRatio);
			m_camera.setNearPlane(camera.nearPlane);
			m_camera.setFarPlane(camera.farPlane);

			break;
		}
	}
}

void GraphicsSystem::destroyAfterUse(Ptr<void> resource)
{
	m_resourcesToDestroy.emplace_back(std::move(resource));
}

void GraphicsSystem::destroyPendingResources(RenderFrame& renderFrame)
{
	for (auto& resource : m_resourcesToDestroy)
		renderFrame.destroyAfterUse(std::move(resource));

	m_resourcesToDestroy.clear();
}
