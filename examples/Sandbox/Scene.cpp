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

#include "Scene.hpp"
#include "Components/GraphicsComponent.hpp"
#include "Components/LightComponent.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/VelocityComponent.hpp"

#include <Atema/Graphics/DefaultMaterials.hpp>
#include <Atema/Graphics/MaterialData.hpp>
#include <Atema/Graphics/Model.hpp>
#include <Atema/Graphics/Primitive.hpp>
#include <Atema/Graphics/RenderScene.hpp>
#include <Atema/Graphics/SkyBox.hpp>
#include <Atema/Graphics/StaticModel.hpp>
#include <Atema/Graphics/Loaders/DefaultImageLoader.hpp>
#include <Atema/Graphics/Loaders/ImageLoader.hpp>
#include <Atema/Graphics/Loaders/ModelLoader.hpp>
#include <Atema/Graphics/Pipelines/EnvironmentPipeline.hpp>
#include <Atema/Renderer/Renderer.hpp>

using namespace at;

namespace
{
	Scene* s_currentScene = nullptr;
}

Scene::Scene() :
	m_entityManager(nullptr),
	m_renderScene(nullptr)
{
	s_currentScene = this;
}

Scene::~Scene()
{
	for (auto& entity : m_entities)
		removeEntity(entity);

	if (s_currentScene == this)
		s_currentScene = nullptr;
}

Scene& Scene::getCurrent()
{
	ATEMA_ASSERT(s_currentScene, "No Scene has been created");

	return *s_currentScene;
}

void Scene::initialize(EntityManager& entityManager, at::RenderScene& renderScene)
{
	m_entityManager = &entityManager;
	m_renderScene = &renderScene;

	createModels();

	createLights();

	createCamera();

	createPlayer();

	createSkyBox();
}

void Scene::update()
{
}

AABBf& Scene::getAABB()
{
	return m_aabb;
}

void Scene::createModels()
{
	auto& entityManager = getEntityManager();
	auto& sceneAABB = getAABB();
	sceneAABB = AABBf();

	// Cube
	{
		ModelLoader::Settings settings(VertexFormat::create(DefaultVertexFormat::XYZ_UV_NTB));

		auto mesh = Primitive::createBox(settings, 1.0f, 1.0f, 1.0f, 1, 1, 1);
		mesh->setMaterialID(0);

		// Entity
		auto entity = entityManager.createEntity();

		// Transform component
		auto& transform = entityManager.createComponent<Transform>(entity);

		transform.setTranslation({ 0.0f, 0.0f, 0.5f });

		// Graphics component
		auto& graphics = entityManager.createComponent<GraphicsComponent>(entity);

		auto materialData = std::make_shared<MaterialData>();
		materialData->set(MaterialData::BaseColor, Color::White);
		materialData->set(MaterialData::Roughness, 0.7f);
		materialData->set(MaterialData::Metalness, 0.4f);

		auto model = std::make_shared<Model>();
		model->addMesh(mesh);
		model->addMaterialData(materialData);
		model->addMaterialInstance(DefaultMaterials::getPBRInstance(*materialData));

		graphics.staticModel = std::make_shared<StaticModel>();
		graphics.staticModel->setModel(model);
		graphics.staticModel->setTransform(transform);
		graphics.staticModel->setCastShadows(true);

		sceneAABB.extend(transform.getMatrix() * graphics.staticModel->getModel()->getAABB());

		addEntity(entity);
	}

	// Plane
	{
		ModelLoader::Settings settings(VertexFormat::create(DefaultVertexFormat::XYZ_UV_NTB));

		auto mesh = Primitive::createPlane(settings, Vector3f(0.0f, 0.0f, 1.0f), 10.0f, 10.0f, 100, 100);
		mesh->setMaterialID(0);

		// Entity
		auto entity = entityManager.createEntity();

		// Transform component
		auto& transform = entityManager.createComponent<Transform>(entity);

		transform.setTranslation({ 0.0f, 0.0f, 0.0f });

		// Graphics component
		auto& graphics = entityManager.createComponent<GraphicsComponent>(entity);

		auto materialData = std::make_shared<MaterialData>();
		materialData->set(MaterialData::BaseColor, Color::White);
		materialData->set(MaterialData::Roughness, 0.7f);
		materialData->set(MaterialData::Metalness, 0.4f);

		auto model = std::make_shared<Model>();
		model->addMesh(mesh);
		model->addMaterialData(materialData);
		model->addMaterialInstance(DefaultMaterials::getPBRInstance(*materialData));

		graphics.staticModel = std::make_shared<StaticModel>();
		graphics.staticModel->setModel(model);
		graphics.staticModel->setTransform(transform);
		graphics.staticModel->setCastShadows(false);

		addEntity(entity);
	}
}

void Scene::createLights()
{
	auto& entityManager = getEntityManager();

	// Directional light
	{
		auto entity = entityManager.createEntity();

		auto light = std::make_shared<DirectionalLight>();
		light->setDirection({ -1.0f, -1.0f, -1.0f });
		light->setColor(Color(1.0f, 1.0f, 1.0f));
		light->setIntensity(8.0f);
		light->setIndirectIntensity(0.1f);
		light->setCastShadows(true);
		light->setShadowCascadeCount(8);
		light->setShadowMapSize(4096);
		light->setShadowMaxDepth(1000.0f);
		light->setShadowDepthBias(0.07f);

		auto& lightComponent = entityManager.createComponent<LightComponent>(entity);
		lightComponent.light = std::move(light);

		addEntity(entity);
	}
}

void Scene::createCamera()
{
	auto& entityManager = getEntityManager();

	// Camera
	{
		const auto entity = entityManager.createEntity();

		// Create default transform
		auto& transform = entityManager.createComponent<Transform>(entity);

		// Create camera
		auto& camera = entityManager.createComponent<CameraComponent>(entity);
		
		initCamera(camera);

		addEntity(entity);
	}
}

void Scene::createPlayer()
{
	auto& entityManager = getEntityManager();

	// Player
	{
		const auto entity = entityManager.createEntity();

		// Create default transform
		auto& transform = entityManager.createComponent<Transform>(entity);
		transform.translate({ 0.0f, 0.0f, 20.0f });

		// Create camera
		auto& camera = entityManager.createComponent<CameraComponent>(entity);
		camera.display = false; // Needs to be set to true to switch to player camera
		camera.isAuto = false;
		camera.useTarget = false;
		camera.farPlane = 10000.0f;

		addEntity(entity);
	}
}

void Scene::createSkyBox()
{
	// Nothing by default
	m_renderScene->setSkyBox(nullptr);
}

void Scene::initCamera(CameraComponent& camera)
{
	camera.display = true; // This is the default camera
	camera.isAuto = true;
	camera.useTarget = true;
	camera.farPlane = 10000.0f;

	auto& aabb = getAABB();
	auto aabbSize = aabb.getSize();
	camera.target = aabb.getCenter();
	camera.cameraZ = aabb.max.z;
	camera.minRadius = 2.0f * std::max(aabbSize.x, aabbSize.y);
	camera.maxRadius = 2.0f * camera.minRadius;
}

void Scene::addEntity(at::EntityHandle entity)
{
	onEntityAdded(entity);

	m_entities.emplace(entity);
}

void Scene::removeEntity(at::EntityHandle entity)
{
	onEntityRemoved(entity);

	m_entities.erase(entity);
}

EntityManager& Scene::getEntityManager() const
{
	ATEMA_ASSERT(m_entityManager, "Scene was not correctly initialized : missing EntityManager");

	return *m_entityManager;
}

void Scene::setSkyBox(const std::filesystem::path& texturePath, bool cubemap)
{
	Ptr<SkyBox> skyBox = std::make_shared<SkyBox>();

	ImageLoader::Settings imageLoaderSettings;
	if (cubemap)
		imageLoaderSettings.type = ImageType::CubeMap;

	auto baseEnvironmentMap = DefaultImageLoader::load(texturePath, imageLoaderSettings);

	Image::Settings imageSettings;
	imageSettings.format = ImageFormat::RGBA16_SFLOAT;
	imageSettings.type = ImageType::CubeMap;
	imageSettings.usages = ImageUsage::RenderTarget | ImageUsage::ShaderSampling;

	imageSettings.width = imageSettings.height = 64;
	imageSettings.mipLevels = 1;
	skyBox->irradianceMap = Image::create(imageSettings);

	imageSettings.width = imageSettings.height = 1024;
	imageSettings.mipLevels = 5;
	skyBox->prefilteredMap = Image::create(imageSettings);

	Ptr<Image> environmentMap;
	if (baseEnvironmentMap->getType() == ImageType::CubeMap)
	{
		skyBox->environmentMap = baseEnvironmentMap;
	}
	else
	{
		const Vector2u imageSize = baseEnvironmentMap->getSize();

		imageSettings.usages |= ImageUsage::TransferSrc | ImageUsage::TransferDst;
		imageSettings.width = imageSettings.height = std::min(imageSize.x, imageSize.y);
		imageSettings.mipLevels = static_cast<uint32_t>(std::floor(std::log2(imageSettings.width))) + 1;
		skyBox->environmentMap = Image::create(imageSettings);

		environmentMap = skyBox->environmentMap;
	}

	EnvironmentPipeline environmentPipeline;
	environmentPipeline.setInput(baseEnvironmentMap);
	environmentPipeline.setOutput(environmentMap, skyBox->irradianceMap, skyBox->prefilteredMap);

	RenderContext renderContext;

	CommandBuffer::Settings commandBufferSettings;
	commandBufferSettings.secondary = false;
	commandBufferSettings.singleUse = true;

	auto commandBuffer = renderContext.createCommandBuffer(commandBufferSettings, QueueType::Graphics);

	commandBuffer->begin();

	environmentPipeline.beginRender();
	environmentPipeline.updateResources(*commandBuffer, renderContext);
	environmentPipeline.render(*commandBuffer, renderContext);

	commandBuffer->imageBarrier(*skyBox->irradianceMap,
		PipelineStage::BottomOfPipe, PipelineStage::FragmentShader,
		0, MemoryAccess::ShaderRead,
		ImageLayout::Attachment, ImageLayout::ShaderRead);

	commandBuffer->imageBarrier(*skyBox->prefilteredMap,
		PipelineStage::BottomOfPipe, PipelineStage::FragmentShader,
		0, MemoryAccess::ShaderRead,
		ImageLayout::Attachment, ImageLayout::ShaderRead);

	commandBuffer->end();

	Renderer::instance().submitAndWait({ commandBuffer });

	m_renderScene->setSkyBox(skyBox);
}
