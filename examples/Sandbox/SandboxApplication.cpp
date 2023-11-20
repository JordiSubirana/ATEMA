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
#include <Atema/Graphics/Primitive.hpp>

#include "Components/GraphicsComponent.hpp"
#include "Components/VelocityComponent.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/LightComponent.hpp"
#include "Systems/SceneUpdateSystem.hpp"
#include "Systems/RandomMoveSystem.hpp"
#include "Systems/GraphicsSystem.hpp"
#include "Systems/CameraSystem.hpp"
#include "Systems/FirstPersonCameraSystem.hpp"
#include "Systems/GuiSystem.hpp"
#include "Resources.hpp"
#include "Scene.hpp"
#include "Scenes/TestScene.hpp"
#include "Scenes/TardisScene.hpp"
#include "Scenes/PBRSpheresScene.hpp"

#include <fstream>

using namespace at;

namespace
{
	std::vector<std::string> systemNames =
	{
		"SceneUpdateSystem",
		"RandomMoveSystem",
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

		const auto vertexFormat = VertexFormat::create(DefaultVertexFormat::XYZ_UV_NTB);

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
	m_sceneType(Settings::SceneType::None)
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

	Graphics::instance().initializeShaderLibraries(ShaderLibraryManager::instance());

	// Create systems
	auto sceneUpdateSystem = std::make_shared<SceneUpdateSystem>();
	sceneUpdateSystem->setEntityManager(m_entityManager);

	m_systems.push_back(sceneUpdateSystem);

	auto randomMoveSystem = std::make_shared<RandomMoveSystem>();
	randomMoveSystem->setEntityManager(m_entityManager);

	m_systems.push_back(randomMoveSystem);

	auto cameraSystem = std::make_shared<CameraSystem>(m_window);
	cameraSystem->setEntityManager(m_entityManager);

	m_systems.push_back(cameraSystem);

	auto firstPersonCameraSystem = std::make_shared<FirstPersonCameraSystem>(m_window);
	firstPersonCameraSystem->setEntityManager(m_entityManager);

	m_systems.push_back(firstPersonCameraSystem);

	auto guiSystem = std::make_shared<GuiSystem>(m_window);
	guiSystem->setEntityManager(m_entityManager);

	m_systems.push_back(guiSystem);

	m_guiSystem = guiSystem.get();

	auto graphicsSystem = std::make_shared<GraphicsSystem>(m_window);
	graphicsSystem->setEntityManager(m_entityManager);

	m_systems.push_back(graphicsSystem);

	m_graphicsSystem = graphicsSystem.get();

	updateScene();
}

SandboxApplication::~SandboxApplication()
{
	m_systems.clear();

	m_scene.reset();

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
}

void SandboxApplication::update(at::TimeStep ms)
{
	checkSettings();

	updateScene();

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
		auto& guiSystem = static_cast<GuiSystem&>(*m_guiSystem);

		guiSystem.updateBenchmarks(m_frameCount);
		guiSystem.updateStats(m_frameCount);

		BenchmarkManager::instance().reset();

		m_frameCount = 0;
		m_frameDuration = 0.0f;
	}
}

void SandboxApplication::checkSettings()
{
	const auto& settings = Settings::instance();

	if (settings.enableFpsLimit)
		setFpsLimit(settings.fpsLimit);
	else
		setFpsLimit(0);

	// Scene related settings are checked in updateScene()
}

void SandboxApplication::updateScene()
{
	const auto& settings = Settings::instance();

	if (!m_scene || m_sceneType != settings.sceneType)
	{
		m_sceneType = settings.sceneType;

		m_scene.reset();

		m_entityManager.clear();

		switch (m_sceneType)
		{
			case Settings::SceneType::None:
			{
				return;
			}
			case Settings::SceneType::Test:
			{
				m_scene = std::make_unique<TestScene>();
				break;
			}
			case Settings::SceneType::Tardis:
			{
				m_scene = std::make_unique<TardisScene>();
				break;
			}
			case Settings::SceneType::PBRSpheres:
			{
				m_scene = std::make_unique<PBRSpheresScene>();
				break;
			}
			default:
			{
				m_scene = std::make_unique<Scene>();
			}
		}

		m_scene->onEntityAdded.connect([this](EntityHandle entity)
			{
				onEntityAdded(entity);
			});

		m_scene->onEntityRemoved.connect([this](EntityHandle entity)
			{
				onEntityRemoved(entity);
			});

		m_scene->initialize(m_entityManager, m_graphicsSystem->getRenderScene());
	}

	m_scene->update();
}

void SandboxApplication::onEntityAdded(at::EntityHandle entity)
{
	for (auto& system : m_systems)
		system->onEntityAdded(entity);
}

void SandboxApplication::onEntityRemoved(at::EntityHandle entity)
{
	for (auto& system : m_systems)
		system->onEntityRemoved(entity);
}
