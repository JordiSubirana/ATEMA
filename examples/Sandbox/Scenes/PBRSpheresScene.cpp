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

#include "PBRSpheresScene.hpp"
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

	//const std::filesystem::path SkyBoxPath = ResourcePath::textures() / "graveyard_pathways_4k.hdr";
	const std::filesystem::path SkyBoxPath = ResourcePath::textures() / "kloppenheim_02_4k.hdr";

	const bool IsSkyBoxCubeMap = false;

	const std::filesystem::path GroundTexturePath = ResourcePath::textures() / "Rocks";
	const std::string GroundTextureExtension = "png";
}

PBRSpheresScene::PBRSpheresScene() :
	m_objectRows(0)
{
}

PBRSpheresScene::~PBRSpheresScene()
{
}

void PBRSpheresScene::createModels()
{
	auto& entityManager = getEntityManager();

	auto& sceneAABB = getAABB();
	sceneAABB = AABBf();

	// PBR Spheres

	//if (false)
	{
		ModelLoader::Settings settings(VertexFormat::create(DefaultVertexFormat::XYZ_UV_NTB));

		float radius = 1.0f;
		float space = 3.0f * radius;
		float z = 20.0f;

		size_t subdivisions = 50;

		size_t count = 5;

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

				sceneAABB.extend(transform.getMatrix() * graphics.staticModel->getModel()->getAABB());

				addEntity(entity);
			}
		}
	}
}

void PBRSpheresScene::createLights()
{
	return;

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

	if (false)
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

void PBRSpheresScene::createSkyBox()
{
	setSkyBox(SkyBoxPath, IsSkyBoxCubeMap);
}

void PBRSpheresScene::initCamera(CameraComponent& camera)
{
	Scene::initCamera(camera);

	auto& aabb = getAABB();
	auto aabbSize = aabb.getSize();
	
	camera.cameraZ = 1.2f * aabb.max.z;
	camera.minRadius = 2.5f * std::max(aabbSize.x, aabbSize.y);
	camera.maxRadius = 4.0f * std::max(aabbSize.x, aabbSize.y);
}
