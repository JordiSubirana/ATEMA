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

#include "SandboxApplication.hpp"

#include <Atema/Core/Variant.hpp>
#include <Atema/Shader.hpp>
#include <Atema/Core/Library.hpp>
#include <Atema/Renderer/RenderWindow.hpp>

#include "Components/GraphicsComponent.hpp"
#include "Components/VelocityComponent.hpp"
#include "Components/CameraComponent.hpp"
#include "Systems/SceneUpdateSystem.hpp"
#include "Systems/GraphicsSystem.hpp"
#include "Systems/CameraSystem.hpp"
#include "Systems/FirstPersonCameraSystem.hpp"
#include "Resources.hpp"

#include <fstream>

using namespace at;

namespace
{
	std::vector<std::string> systemNames =
	{
		"SceneUpdateSystem",
		"CameraSystem",
		"FirstPersonCameraSystem",
		"GraphicsSystem"
	};
}

SandboxApplication::SandboxApplication():
	Application(),
	m_frameCount(0),
	m_frameDuration(0.0f)
{
	// Let default settings for now
	Renderer::Settings settings;

	Renderer::create<VulkanRenderer>(settings);

	// Window / SwapChain
	RenderWindow::Settings renderWindowSettings;
	renderWindowSettings.colorFormat = ImageFormat::RGBA8_UNORM;

	m_window = Renderer::instance().createRenderWindow(renderWindowSettings);
	m_window->getEventDispatcher().addListener([this](Event& event)
		{
			onEvent(event);
		});

	// Create systems
	auto sceneUpdateSystem = std::make_shared<SceneUpdateSystem>();
	sceneUpdateSystem->setEntityManager(m_entityManager);

	m_systems.push_back(sceneUpdateSystem);

	auto cameraSystem = std::make_shared<CameraSystem>(m_window);
	cameraSystem->setEntityManager(m_entityManager);

	m_systems.push_back(cameraSystem);

	auto firstPersonCameraSystem = std::make_shared<FirstPersonCameraSystem>();
	firstPersonCameraSystem->setEntityManager(m_entityManager);

	m_systems.push_back(firstPersonCameraSystem);

	auto graphicsSystem = std::make_shared<GraphicsSystem>(m_window);
	graphicsSystem->setEntityManager(m_entityManager);

	m_systems.push_back(graphicsSystem);

	// Create entities
	createScene();
	
	createCamera();

	createPlayer();
}

SandboxApplication::~SandboxApplication()
{
	m_entityManager.clear();
	m_systems.clear();

	m_modelData.reset();
	m_materialData.reset();
	
	m_window.reset();

	Renderer::destroy();
}

void SandboxApplication::onEvent(at::Event& event)
{
	for (auto& system : m_systems)
	{
		if (event.isHandled())
			return;

		system->onEvent(event);
	}

	// Manage cursor depending on which camera is active
	if (event.is<KeyEvent>())
	{
		auto& keyEvent = static_cast<KeyEvent&>(event);

		if (keyEvent.key == Key::Space && keyEvent.state == KeyState::Press)
		{
			m_window->setCursorEnabled(!m_window->isCursorEnabled());
		}
	}
}

void SandboxApplication::update(at::TimeStep ms)
{
	{
		ATEMA_BENCHMARK("Application update")

			m_frameDuration += ms.getSeconds();

		if (m_window->shouldClose())
		{
			close();
			return;
		}

		m_window->processEvents();

		size_t systemCount = 0;
		for (auto& system : m_systems)
		{
			ATEMA_BENCHMARK(systemNames[systemCount]);

			system->update(ms);

			systemCount++;
		}

		//m_window->swapBuffers();

		m_frameCount++;
	}

	if (m_frameDuration >= 0.5f)
	{
		const auto frameTime = m_frameDuration / static_cast<float>(m_frameCount);
		const auto fps = static_cast<unsigned>(1.0f / frameTime);

		m_window->setTitle("Atema (" + std::to_string(fps) + " fps / " + std::to_string(frameTime * 1000.0f) + " ms)");

		BenchmarkManager::instance().print(m_frameCount);
		BenchmarkManager::instance().reset();

		m_frameCount = 0;
		m_frameDuration = 0.0f;
	}
}

void SandboxApplication::createScene()
{
	// Resources
	m_modelData = std::make_shared<ModelData>(modelMeshPath);

	m_materialData = std::make_shared<MaterialData>(modelTexturePath, modelTextureExtension);

	// Create objects
	const auto origin = -modelScale * (objectRow / 2.0f);

	const Vector2f velocityReference(objectRow / 2, objectRow / 2);
	//const auto maxDistance = Vector2f(objectRow, objectRow).getNorm();
	const auto maxDistance = velocityReference.getNorm();

	for (size_t i = 0; i < objectRow; i++)
	{
		for (size_t j = 0; j < objectRow; j++)
		{
			auto entity = m_entityManager.createEntity();

			// Transform component
			auto& transform = m_entityManager.createComponent<Transform>(entity);

			Vector3f position;
			position.x = modelScale * static_cast<float>(i) + origin;
			position.y = modelScale * static_cast<float>(j) + origin;

			transform.setTranslation(position);
			
			// Graphics component
			auto& graphics = m_entityManager.createComponent<GraphicsComponent>(entity);

			graphics.vertexBuffer = m_modelData->vertexBuffer;
			graphics.indexBuffer = m_modelData->indexBuffer;
			graphics.indexCount = m_modelData->indexCount;
			graphics.color = m_materialData->color;
			graphics.normal = m_materialData->normal;
			graphics.ambientOcclusion = m_materialData->ambientOcclusion;
			graphics.emissive = m_materialData->emissive;
			graphics.metalness = m_materialData->metalness;
			graphics.roughness = m_materialData->roughness;
			graphics.sampler = m_materialData->sampler;
			graphics.aabb = m_modelData->aabb;

			// Velocity component
			auto& velocity = m_entityManager.createComponent<VelocityComponent>(entity);

			//const auto distance = Vector2f(i, j).getNorm();
			const auto distance = (Vector2f(i, j) - velocityReference).getNorm();
			
			const auto percent = distance / maxDistance;
			
			velocity.speed = percent * 3.14159f * 2.0f;
		}
	}
}

void SandboxApplication::createCamera()
{
	const auto entity = m_entityManager.createEntity();

	// Create default transform
	auto& transform = m_entityManager.createComponent<Transform>(entity);

	// Create camera
	auto& camera = m_entityManager.createComponent<CameraComponent>(entity);
	camera.display = true;
	camera.isAuto = true;
	camera.useTarget = true;
	camera.target = Vector3f(0.0f, 0.0f, zoomOffset / 2.0f);
}

void SandboxApplication::createPlayer()
{
	const auto entity = m_entityManager.createEntity();

	// Create default transform
	auto& transform = m_entityManager.createComponent<Transform>(entity);

	// Create camera
	auto& camera = m_entityManager.createComponent<CameraComponent>(entity);
	camera.display = false;
	camera.isAuto = false;
	camera.useTarget = false;
}
