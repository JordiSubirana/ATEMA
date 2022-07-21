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

	std::vector<BasicVertex> planeVertices =
	{
		{{ -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f}, { 0.0f, 1.0f, 0.0f}, { 0.0f, 0.0f }},
		{{ -1.0f, +1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f}, { 0.0f, 1.0f, 0.0f}, { 0.0f, 1.0f }},
		{{ +1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f}, { 0.0f, 1.0f, 0.0f}, { 1.0f, 0.0f }},
		{{ +1.0f, +1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f}, { 0.0f, 1.0f, 0.0f}, { 1.0f, 1.0f }}
	};

	std::vector<uint32_t> planeIndices =
	{
		0, 2, 1,
		1, 2, 3
	};

	Ptr<Buffer> createPlaneVertices(Ptr<CommandPool> commandPool, const Vector3f& center, const Vector2f& size)
	{
		auto vertices = planeVertices;

		for (auto& vertex : vertices)
		{
			auto& pos = vertex.position;

			pos.x *= size.x / 2.0f;
			pos.y *= size.y / 2.0f;

			pos += center;

			vertex.texCoords.x *= size.x;
			vertex.texCoords.y *= size.y;
		}

		// Fill staging buffer
		size_t bufferSize = sizeof(vertices[0]) * vertices.size();

		auto stagingBuffer = Buffer::create({ BufferUsage::Transfer, bufferSize, true });

		auto bufferData = stagingBuffer->map();

		memcpy(bufferData, static_cast<void*>(vertices.data()), static_cast<size_t>(bufferSize));

		stagingBuffer->unmap();

		// Create vertex buffer
		auto vertexBuffer = Buffer::create({ BufferUsage::Vertex, bufferSize });

		// Copy staging buffer to vertex buffer
		auto commandBuffer = commandPool->createBuffer({ true });

		commandBuffer->begin();

		commandBuffer->copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		commandBuffer->end();

		Renderer::instance().submitAndWait({ commandBuffer });

		return vertexBuffer;
	}

	Ptr<Buffer> createPlaneIndices(Ptr<CommandPool> commandPool)
	{
		const auto& indices = planeIndices;

		// Fill staging buffer
		size_t bufferSize = sizeof(indices[0]) * indices.size();

		auto stagingBuffer = Buffer::create({ BufferUsage::Transfer, bufferSize, true });

		auto bufferData = stagingBuffer->map();

		memcpy(bufferData, indices.data(), bufferSize);

		stagingBuffer->unmap();

		// Create vertex buffer
		auto indexBuffer = Buffer::create({ BufferUsage::Index, bufferSize });

		// Copy staging buffer to vertex buffer
		auto commandBuffer = commandPool->createBuffer({ true });

		commandBuffer->begin();

		commandBuffer->copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		commandBuffer->end();

		Renderer::instance().submitAndWait({ commandBuffer });

		return indexBuffer;
	}
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
	m_systems.clear();

	m_entityManager.clear();
	
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
		ATEMA_BENCHMARK("Application update");

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
	AABBf sceneAABB;

	{
		// Resources
		auto modelData = std::make_shared<ModelData>(modelMeshPath);

		auto materialData = std::make_shared<MaterialData>(modelTexturePath, modelTextureExtension);

		// Create objects
		auto aabbSize = modelData->aabb.getSize();
		aabbSize.z = 0.0f;
		const auto radius = (aabbSize.getNorm() / 2.0f) * 3.0f;
		const auto origin = -radius * (objectRow / 2.0f);

		const Vector2f velocityReference(objectRow / 2, objectRow / 2);
		const auto maxDistance = velocityReference.getNorm();

		for (size_t i = 0; i < objectRow; i++)
		{
			for (size_t j = 0; j < objectRow; j++)
			{
				auto entity = m_entityManager.createEntity();

				// Transform component
				auto& transform = m_entityManager.createComponent<Transform>(entity);

				Vector3f position;
				position.x = origin + radius * static_cast<float>(i);
				position.y = origin + radius * static_cast<float>(j);

				transform.setTranslation(position);

				// Graphics component
				auto& graphics = m_entityManager.createComponent<GraphicsComponent>(entity);

				graphics.vertexBuffer = modelData->vertexBuffer;
				graphics.indexBuffer = modelData->indexBuffer;
				graphics.indexCount = modelData->indexCount;
				graphics.material = materialData;
				graphics.aabb = modelData->aabb;

				sceneAABB.extend(transform.getMatrix() * graphics.aabb);

				// Velocity component
				auto& velocity = m_entityManager.createComponent<VelocityComponent>(entity);

				//const auto distance = Vector2f(i, j).getNorm();
				const auto distance = (Vector2f(i, j) - velocityReference).getNorm();

				const auto percent = distance / maxDistance;

				velocity.speed = percent * 3.14159f * 2.0f;
			}
		}
	}

	// Create ground
	{
		// Resources
		auto commandPool = Renderer::instance().getCommandPool(QueueType::Graphics);

		auto materialData = std::make_shared<MaterialData>(groundTexturePath, groundTextureExtension);

		Vector2f planeSize = { sceneAABB.max.x - sceneAABB.min.x, sceneAABB.max.y - sceneAABB.min.y };
		planeSize += Vector2f(1000.0f, 1000.0f);

		auto vertexBuffer = createPlaneVertices(commandPool, sceneAABB.getCenter(), planeSize);

		auto indexBuffer = createPlaneIndices(commandPool);

		// Entity
		auto entity = m_entityManager.createEntity();

		// Transform component
		auto& transform = m_entityManager.createComponent<Transform>(entity);

		transform.setTranslation({ 0.0f, 0.0f, -2.0f });

		// Graphics component
		auto& graphics = m_entityManager.createComponent<GraphicsComponent>(entity);

		graphics.vertexBuffer = vertexBuffer;
		graphics.indexBuffer = indexBuffer;
		graphics.indexCount = static_cast<uint32_t>(planeIndices.size());
		graphics.material = materialData;
		graphics.aabb = sceneAABB;
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
	transform.translate({ 1.0f, 0.0f, 1.0f });

	// Create camera
	auto& camera = m_entityManager.createComponent<CameraComponent>(entity);
	camera.display = false;
	camera.isAuto = false;
	camera.useTarget = false;
}
