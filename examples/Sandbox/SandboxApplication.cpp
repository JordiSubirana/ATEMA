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
#include <Atema/Core/Library.hpp>
#include <Atema/Graphics/VertexFormat.hpp>
#include <Atema/Graphics/Loaders/ObjLoader.hpp>
#include <Atema/Renderer/RenderWindow.hpp>

#include "Components/GraphicsComponent.hpp"
#include "Components/VelocityComponent.hpp"
#include "Components/CameraComponent.hpp"
#include "Systems/SceneUpdateSystem.hpp"
#include "Systems/GraphicsSystem.hpp"
#include "Systems/CameraSystem.hpp"
#include "Systems/FirstPersonCameraSystem.hpp"
#include "Systems/GuiSystem.hpp"
#include "Resources.hpp"
#include "Scene.hpp"

#include <fstream>

using namespace at;

namespace
{
	std::vector<std::string> systemNames =
	{
		"SceneUpdateSystem",
		"CameraSystem",
		"FirstPersonCameraSystem",
		"GuiSystem",
		"GraphicsSystem"
	};

	std::vector<ModelLoader::StaticVertex> planeVertices =
	{
		{{ -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f}, { 0.0f, 1.0f, 0.0f}},
		{{ -1.0f, +1.0f, 0.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f}, { 0.0f, 1.0f, 0.0f}},
		{{ +1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f}, { 0.0f, 1.0f, 0.0f}},
		{{ +1.0f, +1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f}, { 0.0f, 1.0f, 0.0f}}
	};

	std::vector<uint32_t> planeIndices =
	{
		0, 2, 1,
		1, 2, 3
	};

	Ptr<Model> createPlaneModel(const Vector3f& center, const Vector2f& size)
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

		const auto vertexFormat = VertexFormat::create(DefaultVertexFormat::XYZ_UV_Normal_Tangent_Bitangent);

		ModelLoader::Settings settings(vertexFormat);

		auto mesh = ModelLoader::loadMesh(vertices, planeIndices, settings);

		auto model = std::make_shared<Model>();
		model->addMesh(mesh);

		return model;
	}

	constexpr size_t getObjectRadius(size_t row, size_t col)
	{
		return std::max(row, col);
	}

	constexpr size_t getRadiusBegin(size_t radius)
	{
		return radius * radius;
	}

	constexpr size_t getRadiusCount(size_t radius)
	{
		return radius * 2 + 1;
	}

	constexpr size_t getObjectIndex(size_t row, size_t col)
	{
		const auto radius = getObjectRadius(row, col);
		const auto begin = getRadiusBegin(radius);

		return begin + col + (radius - row);
	}

	Vector2u getObjectPosition(size_t radius, size_t offset)
	{
		auto row = static_cast<uint32_t>(radius);
		if (offset > radius)
		{
			const auto count = getRadiusCount(radius);

			row = static_cast<uint32_t>(count - offset - 1);
		}

		return { row, offset - (radius - row) };
	}
}

SandboxApplication::SandboxApplication():
	Application(),
	m_frameCount(0),
	m_frameDuration(0.0f),
	m_objectRows(0)
{
	// Let default settings for now
	Renderer::Settings settings;

	Renderer::create<VulkanRenderer>(settings);

	// Window / SwapChain
	RenderWindow::Settings renderWindowSettings;
	renderWindowSettings.title = "Atema - Sandbox";
	renderWindowSettings.width = 1280;
	renderWindowSettings.height = 720;
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

	auto firstPersonCameraSystem = std::make_shared<FirstPersonCameraSystem>(m_window);
	firstPersonCameraSystem->setEntityManager(m_entityManager);

	m_systems.push_back(firstPersonCameraSystem);

	auto guiSystem = std::make_shared<GuiSystem>(m_window);
	guiSystem->setEntityManager(m_entityManager);

	m_systems.push_back(guiSystem);

	auto graphicsSystem = std::make_shared<GraphicsSystem>(m_window);
	graphicsSystem->setEntityManager(m_entityManager);

	m_systems.push_back(graphicsSystem);

	// Resources
	m_modelData = std::make_shared<ModelData>(modelMeshPath);

	if (!m_modelData->model->getMaterials().empty())
	{
		for (const auto& material : m_modelData->model->getMaterials())
			m_materialData.emplace_back(std::make_shared<MaterialData>(*material));
	}
	else
	{
		m_materialData.emplace_back(std::make_shared<MaterialData>(modelTexturePath, modelTextureExtension));
	}

	// Create entities
	createScene();
	
	createCamera();

	createPlayer();
}

SandboxApplication::~SandboxApplication()
{
	m_systems.clear();

	m_entityManager.clear();

	m_modelData.reset();
	m_materialData.clear();
	
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
}

void SandboxApplication::update(at::TimeStep ms)
{
	{
		checkSettings();
	}

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

	if (m_frameDuration >= Settings::instance().metricsUpdateTime)
	{
		auto& guiSystem = static_cast<GuiSystem&>(*m_systems[3]);

		guiSystem.updateBenchmarks(m_frameCount);

		BenchmarkManager::instance().reset();

		m_frameCount = 0;
		m_frameDuration = 0.0f;
	}
}

void SandboxApplication::checkSettings()
{
	const auto& settings = Settings::instance();

	if (m_objectRows != settings.objectRows)
	{
		updateScene();
	}

	if (settings.enableFpsLimit)
		setFpsLimit(settings.fpsLimit);
	else
		setFpsLimit(0);
}

void SandboxApplication::createScene()
{
	updateScene();

	// Create ground
	{
		// Resources
		auto materialData = std::make_shared<MaterialData>(groundTexturePath, groundTextureExtension);

		const Vector2f planeSize(1000.0f, 1000.0f);

		// Entity
		auto entity = m_entityManager.createEntity();

		// Transform component
		auto& transform = m_entityManager.createComponent<Transform>(entity);

		transform.setTranslation({ 0.0f, 0.0f, -2.0f });

		// Graphics component
		auto& graphics = m_entityManager.createComponent<GraphicsComponent>(entity);

		graphics.model = createPlaneModel({ 0, 0, 0 }, planeSize);
		graphics.materials.emplace_back(materialData);
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
	camera.farPlane = 10000.0f;
}

void SandboxApplication::createPlayer()
{
	const auto entity = m_entityManager.createEntity();

	// Create default transform
	auto& transform = m_entityManager.createComponent<Transform>(entity);
	//transform.translate({ -220.0f, -220.0f, 20.0f });
	transform.translate({ 0.0f, 0.0f, 20.0f });

	// Create camera
	auto& camera = m_entityManager.createComponent<CameraComponent>(entity);
	camera.display = false;
	camera.isAuto = false;
	camera.useTarget = false;
	camera.farPlane = 10000.0f;
}

void SandboxApplication::updateScene()
{
	const auto newObjectRows = Settings::instance().objectRows;

	if (m_objectRows == newObjectRows)
		return;

	const auto newSize = static_cast<size_t>(newObjectRows) * static_cast<size_t>(newObjectRows);

	// We want to remove existing objects
	if (m_objectRows > newObjectRows)
	{
		auto firstIndex = getRadiusBegin(newObjectRows);

		for (size_t i = firstIndex; i < m_objects.size(); i++)
			m_entityManager.removeEntity(m_objects[i]);

		m_objects.resize(newSize);
	}
	// We want to add new objects
	else
	{
		m_objects.resize(newSize);

		const auto firstRow = m_objectRows;

		for (size_t r = firstRow; r < newObjectRows; r++)
		{
			const auto count = getRadiusCount(r);

			for (size_t i = 0; i < count; i++)
			{
				const auto pos = getObjectPosition(r, i);

				const auto entity = m_entityManager.createEntity();

				m_objects[getObjectIndex(pos.x, pos.y)] = entity;

				// Graphics component
				auto& graphics = m_entityManager.createComponent<GraphicsComponent>(entity);

				graphics.model = m_modelData->model;
				graphics.materials = m_materialData;
				graphics.castShadows = true;

				// Transform component
				m_entityManager.createComponent<Transform>(entity);

				// Velocity component
				m_entityManager.createComponent<VelocityComponent>(entity);
			}
		}
	}

	// Update components
	{
		auto& sceneAABB = Scene::instance().getAABB();
		sceneAABB = AABBf();

		auto aabbSize = m_modelData->model->getAABB().getSize();
		aabbSize.z = 0.0f;
		const auto radius = (aabbSize.getNorm() / 2.0f) * 3.0f;
		const auto origin = -radius * (static_cast<float>(newObjectRows) / 2.0f);

		const Vector2f velocityReference(newObjectRows / 2, newObjectRows / 2);
		const auto maxDistance = velocityReference.getNorm();

		for (size_t i = 0; i < newObjectRows; i++)
		{
			for (size_t j = 0; j < newObjectRows; j++)
			{
				const auto entity = m_objects[getObjectIndex(i, j)];

				// Graphics component
				auto& graphics = m_entityManager.getComponent<GraphicsComponent>(entity);

				// Transform component
				auto& transform = m_entityManager.getComponent<Transform>(entity);

				Vector3f position;
				position.x = origin + radius * static_cast<float>(i);
				position.y = origin + radius * static_cast<float>(j);

				transform.setTranslation(position);

				// Update Scene aabb
				sceneAABB.extend(transform.getMatrix() * graphics.model->getAABB());

				// Velocity component
				auto& velocity = m_entityManager.getComponent<VelocityComponent>(entity);

				//const auto distance = Vector2f(i, j).getNorm();
				const auto distance = (Vector2f(i, j) - velocityReference).getNorm();

				const auto percent = (newObjectRows == 1) ? 0.0f : distance / maxDistance;

				velocity.speed = percent * 3.14159f * 2.0f;
				velocity.speed = percent * 3.14159f * 0.25f;
				velocity.speed = 0.0f;
			}
		}
	}

	// Update parameters
	m_objectRows = newObjectRows;
}
