/*
	Copyright 2023 Jordi SUBIRANA

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

#include "TestScene.hpp"
#include "../Resources.hpp"
#include "../Components/GraphicsComponent.hpp"
#include "../Components/LightComponent.hpp"
#include "../Components/CameraComponent.hpp"
#include "../Components/VelocityComponent.hpp"

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
#include "../Settings.hpp"

using namespace at;

namespace
{
	// Resources

	ResourceLoader::ModelSettings getTardisModelSettings()
	{
		ResourceLoader::ModelSettings settings;
		settings.modelPath = ResourcePath::models() / "tardis.obj";
		settings.overrideMaterial = true;
		settings.modelTexturePath = ResourcePath::textures() / "tardis";
		settings.modelTextureExtension = "png";
		settings.vertexTransformation = Transform({ 0.0f, 0.0f, 0.0f }, { at::Math::HalfPi<float>, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }).getMatrix();

		return settings;
	}

	const std::filesystem::path GroundTexturePath = ResourcePath::textures() / "Rocks";
	const std::string GroundTextureExtension = "png";

	const std::filesystem::path UVCheckerTexturePath = ResourcePath::textures() / "uv-checker_color.png";

	// Object grid management

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

TestScene::TestScene() :
	m_objectRows(0)
{
}

TestScene::~TestScene()
{
}

void TestScene::update()
{
	updateModels();
}

void TestScene::createModels()
{
	auto& entityManager = getEntityManager();

	// Main objects
	m_model = ResourceLoader::loadModel(getTardisModelSettings());

	updateModels();

	// Ground

	//if (false)
	{
		// Resources
		auto materialData = ResourceLoader::loadMaterialData(GroundTexturePath, GroundTextureExtension);

		ModelLoader::Settings settings(VertexFormat::create(DefaultVertexFormat::XYZ_UV_NTB));

		auto mesh = Primitive::createPlane(settings, Vector3f(0.0f, 0.0f, 1.0), 1000.0f, 1000.0f, 1, 1);
		mesh->setMaterialID(0);

		// Entity
		auto entity = entityManager.createEntity();

		// Transform component
		auto& transform = entityManager.createComponent<Transform>(entity);

		//transform.setTranslation({ 0.0f, 0.0f, -2.0f });

		// Graphics component
		auto& graphics = entityManager.createComponent<GraphicsComponent>(entity);

		auto model = std::make_shared<Model>();
		model->addMesh(mesh);
		model->addMaterialData(materialData);
		//model->addMaterialInstance(DefaultMaterials::getPhongInstance(*materialData));
		model->addMaterialInstance(DefaultMaterials::getPBRInstance(*materialData));

		graphics.staticModel = std::make_shared<StaticModel>();
		graphics.staticModel->setModel(model);
		graphics.staticModel->setTransform(transform);
		graphics.staticModel->setCastShadows(false);

		addEntity(entity);
	}

	// PBR Spheres

	//if (false)
	{
		ModelLoader::Settings settings(VertexFormat::create(DefaultVertexFormat::XYZ_UV_NTB));

		float radius = 1.0f;
		float space = 3.0f * radius;
		float z = 20.0f;

		size_t subdivisions = 50;

		size_t count = 3;

		float step = 1.0f / static_cast<float>(count - 1);

		float xbegin = -space * static_cast<float>(count - 1) / 2.0f;
		xbegin = 0;

		auto mesh = Primitive::createUVSphere(settings, radius, subdivisions, subdivisions);
		mesh->setMaterialID(0);

		for (size_t x = 0; x < count; x++)
		{
			for (size_t y = 0; y < count; y++)
			{
				// Entity
				auto entity = entityManager.createEntity();

				// Transform component
				auto& transform = entityManager.createComponent<Transform>(entity);

				transform.setTranslation({ xbegin + space * x, 0.0f, z + space * y });

				// Graphics component
				auto& graphics = entityManager.createComponent<GraphicsComponent>(entity);

				auto materialData = std::make_shared<MaterialData>();
				materialData->set(MaterialData::BaseColor, Color::White);
				materialData->set(MaterialData::Metalness, step * static_cast<float>(y));
				materialData->set(MaterialData::Roughness, step * static_cast<float>(x));

				auto model = std::make_shared<Model>();
				model->addMesh(mesh);
				model->addMaterialData(materialData);
				//model->addMaterialInstance(DefaultMaterials::getPhongInstance(*materialData));
				model->addMaterialInstance(DefaultMaterials::getPBRInstance(*materialData));

				graphics.staticModel = std::make_shared<StaticModel>();
				graphics.staticModel->setModel(model);
				graphics.staticModel->setTransform(transform);
				graphics.staticModel->setCastShadows(true);

				addEntity(entity);
			}
		}
	}

	// Sphere (UV texture)

	if (false)
	{
		ModelLoader::Settings settings(VertexFormat::create(DefaultVertexFormat::XYZ_UV_NTB));

		auto mesh = Primitive::createUVSphere(settings, 2.0f, 10, 10);
		mesh->setMaterialID(0);

		// Entity
		auto entity = entityManager.createEntity();

		// Transform component
		auto& transform = entityManager.createComponent<Transform>(entity);

		transform.setTranslation({ 0.0f, 0.0f, 8.0f });

		// Graphics component
		auto& graphics = entityManager.createComponent<GraphicsComponent>(entity);

		auto materialData = std::make_shared<MaterialData>();
		materialData->set(MaterialData::BaseColor, Color::White);
		materialData->set(MaterialData::BaseColorMap, MaterialData::Texture(UVCheckerTexturePath));

		auto model = std::make_shared<Model>();
		model->addMesh(mesh);
		model->addMaterialData(materialData);
		//model->addMaterialInstance(DefaultMaterials::getPhongInstance(*materialData));
		model->addMaterialInstance(DefaultMaterials::getPBRInstance(*materialData));

		graphics.staticModel = std::make_shared<StaticModel>();
		graphics.staticModel->setModel(model);
		graphics.staticModel->setTransform(transform);
		graphics.staticModel->setCastShadows(true);

		addEntity(entity);
	}
}

void TestScene::createLights()
{
	//return;

	auto& entityManager = getEntityManager();

	DirectionalLight refLight;
	refLight.setShadowMaxDepth(1000.0f);
	refLight.setColor(Color(1.0f, 1.0f, 1.0f));
	refLight.setIntensity(8.0f);
	refLight.setIndirectIntensity(0.1f);
	refLight.setCastShadows(true);
	refLight.setShadowMapSize(4096);
	refLight.setShadowCascadeCount(8);
	refLight.setShadowDepthBias(0.07f);

	// Directional light #1

	//if (false)
	{
		auto entity = entityManager.createEntity();

		auto light = std::make_shared<DirectionalLight>();
		*light = refLight;
		light->setDirection({ -1.0f, -1.0f, -1.0f });
		light->setColor(Color(1.0f, 1.0f, 1.0f));

		auto& lightComponent = entityManager.createComponent<LightComponent>(entity);
		lightComponent.light = std::move(light);

		addEntity(entity);
	}

	// Directional light #2

	if (false)
	{
		auto entity = entityManager.createEntity();

		auto light = std::make_shared<DirectionalLight>();
		*light = refLight;
		light->setDirection({ 1.0f, -1.2f, -1.0f });
		light->setColor(Color(1.0f, 1.0f, 1.0f));

		auto& lightComponent = entityManager.createComponent<LightComponent>(entity);
		lightComponent.light = std::move(light);

		addEntity(entity);
	}

	// Point lights

	//if (false)
	{
		// Parameters
		const float lightRadius = 2.0f;
		const float lightIntensity = 10.0f;
		const float lightZ = 1.0f;

		const float meshRadius = .02f;
		const size_t meshSubdivisions = 6;

		const int rows = 20;
		const int height = 15;
		const float offset = 2.0f * lightRadius;
		const float zoffset = 1.0f * lightRadius;

		//-----
		ModelLoader::Settings settings(VertexFormat::create(DefaultVertexFormat::XYZ_UV_NTB));
		auto mesh = Primitive::createUVSphere(settings, meshRadius, meshSubdivisions, meshSubdivisions);
		mesh->setMaterialID(0);

		PointLight refPointLight;
		refPointLight.setColor(Color(1.0f, 1.0f, 1.0f));
		refPointLight.setIntensity(lightIntensity);
		refPointLight.setIndirectIntensity(0.05f);
		refPointLight.setCastShadows(false);
		refPointLight.setShadowMapSize(4096);
		refPointLight.setShadowCascadeCount(8);
		refPointLight.setShadowDepthBias(0.07f);

		const int halfRows = rows / 2;

		for (int x = -halfRows; x <= halfRows; x++)
		{
			for (int y = -halfRows; y <= halfRows; y++)
			{
				for (int z = 0; z < height; z++)
				{
					auto entity = entityManager.createEntity();

					Vector3f position;
					position.x = offset * static_cast<float>(x);
					position.y = offset * static_cast<float>(y);
					position.z = lightZ + zoffset * static_cast<float>(z);

					Color color;
					color.r = (static_cast<float>(rand() % 255) / 255.0f);
					color.g = (static_cast<float>(rand() % 255) / 255.0f);
					color.b = (static_cast<float>(rand() % 255) / 255.0f);

					// Light component
					auto light = std::make_shared<PointLight>();
					*light = refPointLight;
					light->setPosition(position);
					light->setRadius(lightRadius);
					light->setColor(color);

					auto& lightComponent = entityManager.createComponent<LightComponent>(entity);
					lightComponent.light = std::move(light);

					// Transform component
					auto& transform = entityManager.createComponent<Transform>(entity);

					transform.setTranslation(position);

					// Graphics component
					//*
					auto& graphics = entityManager.createComponent<GraphicsComponent>(entity);

					auto materialData = std::make_shared<MaterialData>();
					materialData->set(MaterialData::EmissiveColor, Color(color.toVector4f() * lightIntensity));

					auto model = std::make_shared<Model>();
					model->addMesh(mesh);
					model->addMaterialData(materialData);
					model->addMaterialInstance(DefaultMaterials::getEmissiveInstance(*materialData));

					graphics.staticModel = std::make_shared<StaticModel>();
					graphics.staticModel->setModel(model);
					graphics.staticModel->setTransform(transform);
					graphics.staticModel->setCastShadows(false);
					//*/

					addEntity(entity);
				}
			}
		}
	}

	// Spot lights

	if (false)
	{
		// Parameters
		const Vector3f lightDir = Vector3f(-1.0f, 0.0f, -1.0f).getNormalized();
		const float lightRange = 30.0f;
		const float lightAngle = Math::toRadians(90.0f);
		const float lightIntensity = 4.0f;
		const float lightZ = 3.0f;

		const float meshRange = 0.5f;
		const size_t meshVerticalSubdivisions = 4;
		const size_t meshHorizontalSubdivisions = 1;

		const int rows = 50;
		const float offset = 1.0f * lightRange;

		//-----
		ModelLoader::Settings settings(VertexFormat::create(DefaultVertexFormat::XYZ_UV_NTB));
		auto mesh = Primitive::createConeFromAngle(settings, lightDir, meshRange, lightAngle, meshVerticalSubdivisions, meshHorizontalSubdivisions);
		mesh->setMaterialID(0);

		SpotLight refSpotLight;
		refSpotLight.setColor(Color(1.0f, 1.0f, 1.0f));
		refSpotLight.setIndirectIntensity(0.05f);
		refSpotLight.setIntensity(0.8f);
		refSpotLight.setCastShadows(false);
		refSpotLight.setShadowMapSize(4096);
		refSpotLight.setShadowCascadeCount(8);
		refSpotLight.setShadowDepthBias(0.07f);

		const int halfRows = rows / 2;

		for (int x = -halfRows; x <= halfRows; x++)
		{
			for (int y = -halfRows; y <= halfRows; y++)
			{
				auto entity = entityManager.createEntity();

				Vector3f position(offset * static_cast<float>(x), offset * static_cast<float>(y), lightZ);

				Color color;
				color.r = lightIntensity * (static_cast<float>(rand() % 255) / 255.0f);
				color.g = lightIntensity * (static_cast<float>(rand() % 255) / 255.0f);
				color.b = lightIntensity * (static_cast<float>(rand() % 255) / 255.0f);

				// Light component
				auto light = std::make_shared<SpotLight>();
				*light = refSpotLight;
				light->setPosition(position);
				light->setDirection(lightDir);
				light->setRange(lightRange);
				light->setAngle(lightAngle);
				light->setColor(color);

				auto& lightComponent = entityManager.createComponent<LightComponent>(entity);
				lightComponent.light = std::move(light);

				// Transform component
				auto& transform = entityManager.createComponent<Transform>(entity);

				transform.setTranslation(position);

				// Graphics component
				auto& graphics = entityManager.createComponent<GraphicsComponent>(entity);

				auto materialData = std::make_shared<MaterialData>();
				materialData->set(MaterialData::EmissiveColor, color);

				auto model = std::make_shared<Model>();
				model->addMesh(mesh);
				model->addMaterialData(materialData);
				model->addMaterialInstance(DefaultMaterials::getEmissiveInstance(*materialData));

				graphics.staticModel = std::make_shared<StaticModel>();
				graphics.staticModel->setModel(model);
				graphics.staticModel->setTransform(transform);
				graphics.staticModel->setCastShadows(true);

				addEntity(entity);
			}
		}
	}
}

void TestScene::updateModels()
{
	auto& entityManager = getEntityManager();

	const auto newObjectRows = Settings::instance().objectRows;

	if (m_objectRows == newObjectRows)
		return;

	const auto newSize = static_cast<size_t>(newObjectRows) * static_cast<size_t>(newObjectRows);

	// We want to remove existing objects
	if (m_objectRows > newObjectRows)
	{
		auto firstIndex = getRadiusBegin(newObjectRows);

		for (size_t i = firstIndex; i < m_objects.size(); i++)
		{
			removeEntity(m_objects[i]);
			entityManager.removeEntity(m_objects[i]);
		}

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

				const auto entity = entityManager.createEntity();

				m_objects[getObjectIndex(pos.x, pos.y)] = entity;

				// Graphics component
				auto& graphics = entityManager.createComponent<GraphicsComponent>(entity);

				graphics.staticModel = std::make_shared<StaticModel>();
				graphics.staticModel->setModel(m_model);
				graphics.staticModel->setCastShadows(true);

				// Transform component
				entityManager.createComponent<Transform>(entity);

				// Velocity component
				entityManager.createComponent<VelocityComponent>(entity);

				addEntity(entity);
			}
		}
	}

	// Update components
	{
		auto& sceneAABB = getAABB();
		sceneAABB = AABBf();

		auto aabb = m_model->getAABB();

		auto aabbSize = aabb.getSize();
		aabbSize.z = 0.0f;
		const auto radius = (aabbSize.getNorm() / 2.0f) * 3.0f;

		Vector2f origin;
		origin.x = -aabb.getCenter().x - radius * (static_cast<float>(newObjectRows - 1) / 2.0f);
		origin.y = -aabb.getCenter().y - radius * (static_cast<float>(newObjectRows - 1) / 2.0f);

		const Vector2f velocityReference(newObjectRows / 2, newObjectRows / 2);
		const auto maxDistance = velocityReference.getNorm();

		for (size_t i = 0; i < newObjectRows; i++)
		{
			for (size_t j = 0; j < newObjectRows; j++)
			{
				const auto entity = m_objects[getObjectIndex(i, j)];

				// Graphics component
				auto& graphics = entityManager.getComponent<GraphicsComponent>(entity);

				// Transform component
				auto& transform = entityManager.getComponent<Transform>(entity);

				Vector3f position;
				position.x = origin.x + radius * static_cast<float>(i);
				position.y = origin.y + radius * static_cast<float>(j);

				transform.setTranslation(position);
				transform.setRotation(Vector3f(0.0f, 0.0f, 0.0f));

				// Update Scene aabb
				sceneAABB.extend(transform.getMatrix() * graphics.staticModel->getModel()->getAABB());

				// Velocity component
				auto& velocity = entityManager.getComponent<VelocityComponent>(entity);

				const auto distance = (Vector2f(i, j) - velocityReference).getNorm();

				const auto percent = (newObjectRows == 1) ? 0.0f : distance / maxDistance;

				velocity.speed = percent * 3.14159f * 0.25f;
			}
		}
	}

	// Update parameters
	m_objectRows = newObjectRows;
}
