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

#include <Atema/Graphics/RenderScene.hpp>
#include <Atema/Graphics/AbstractFrameRenderer.hpp>
#include <Atema/Graphics/Material.hpp>
#include <Atema/Core/Benchmark.hpp>

using namespace at;

RenderScene::RenderScene(RenderResourceManager& resourceManager, AbstractFrameRenderer& frameRenderer) :
	RenderResource(resourceManager),
	m_frameRenderer(&frameRenderer),
	m_camera(nullptr)
{
}

RenderScene::~RenderScene()
{
	clear();
}

bool RenderScene::isValid() const noexcept
{
	return m_camera;
}

void RenderScene::setCamera(const Camera& camera)
{
	m_camera = &camera;
}

void RenderScene::setSkyBox(Ptr<SkyBox> skyBox)
{
	m_skyBox = std::move(skyBox);
}

void RenderScene::addLight(Light& light)
{
	if (m_renderLightIndices.find(&light) == m_renderLightIndices.end())
	{
		m_renderLights.emplace_back(std::make_shared<RenderLight>(getResourceManager(), light));
		m_renderLightIndices[&light] = m_renderLights.size() - 1;
	}
}

void RenderScene::addRenderable(Renderable& renderable)
{
	if (m_renderObjectIndices.find(&renderable) == m_renderObjectIndices.end())
	{
		m_renderObjects.emplace_back(renderable.createRenderObject(*this));
		m_renderObjectIndices[&renderable] = m_renderObjects.size() - 1;
	}
}

void RenderScene::removeLight(const Light& light)
{
	const auto it = m_renderLightIndices.find(&light);

	if (it != m_renderLightIndices.end())
	{
		const size_t currentIndex = it->second;

		auto& current = m_renderLights[currentIndex];
		auto& last = m_renderLights.back();

		auto& lastLight = last->getLight();

		destroyAfterUse(std::move(current));

		std::swap(current, last);
		m_renderLights.resize(m_renderLights.size() - 1);

		m_renderLightIndices[&lastLight] = currentIndex;
		m_renderLightIndices.erase(it);
	}
}

void RenderScene::removeRenderable(const Renderable& renderable)
{
	const auto it = m_renderObjectIndices.find(&renderable);

	if (it != m_renderObjectIndices.end())
	{
		const size_t currentIndex = it->second;

		auto& current = m_renderObjects[currentIndex];
		auto& last = m_renderObjects.back();

		auto& lastRenderable = last->getRenderable();

		destroyAfterUse(std::move(current));

		std::swap(current, last);
		m_renderObjects.resize(m_renderObjects.size() - 1);

		m_renderObjectIndices[&lastRenderable] = currentIndex;
		m_renderObjectIndices.erase(it);
	}
}

void RenderScene::clear()
{
	clearLights();
	clearRenderables();

	for (auto& [id, renderData] : m_renderMaterials)
		renderData.connection.disconnect();
	m_renderMaterials.clear();

	for (auto& [id, renderData] : m_renderMaterialInstances)
		renderData.connection.disconnect();
	m_renderMaterialInstances.clear();
}

void RenderScene::clearLights()
{
	m_renderLightIndices.clear();

	for (auto& resource : m_renderLights)
		destroyAfterUse(std::move(resource));

	m_renderLights.clear();
}

void RenderScene::clearRenderables()
{
	m_renderObjectIndices.clear();

	for (auto& resource : m_renderObjects)
		destroyAfterUse(std::move(resource));

	m_renderObjects.clear();
}

void RenderScene::recompileMaterials()
{
	m_renderMaterialInstances.clear();
	m_renderMaterials.clear();
}

const Camera& RenderScene::getCamera() const noexcept
{
	return *m_camera;
}

const Ptr<SkyBox>& RenderScene::getSkyBox() const noexcept
{
	return m_skyBox;
}

RenderMaterial& RenderScene::getRenderMaterial(Ptr<Material> material)
{
	const auto key = material.get();

	const auto it = m_renderMaterials.find(key);

	if (it == m_renderMaterials.end())
	{
		auto renderMaterial = m_frameRenderer->createRenderMaterial(std::move(material));

		const auto renderMaterialPtr = renderMaterial.get();

		auto& renderData = m_renderMaterials[key];
		renderData.resource = std::move(renderMaterial);
		renderData.connection = key->onDestroy.connect([this, key]()
			{
				auto it = m_renderMaterials.find(key);

				if (it != m_renderMaterials.end())
				{
					// Destroy render resource
					destroyAfterUse(std::move(it->second.resource));

					// Disconnect
					it->second.connection.disconnect();

					m_renderMaterials.erase(it);
				}
			});

		return *renderMaterialPtr;
	}

	return *it->second.resource;
}

RenderMaterialInstance& RenderScene::getRenderMaterialInstance(MaterialInstance& materialInstance)
{
	auto key = &materialInstance;

	const auto it = m_renderMaterialInstances.find(key);

	if (it == m_renderMaterialInstances.end())
	{
		auto renderMaterialInstance = getRenderMaterial(materialInstance.getMaterial()).createInstance(materialInstance);

		const auto renderMaterialInstancePtr = renderMaterialInstance.get();

		auto& renderData = m_renderMaterialInstances[key];
		renderData.resource = std::move(renderMaterialInstance);
		renderData.connection = key->onDestroy.connect([this, key]()
			{
				auto it = m_renderMaterialInstances.find(key);

				if (it != m_renderMaterialInstances.end())
				{
					// Destroy render resource
					destroyAfterUse(std::move(it->second.resource));

					// Disconnect
					it->second.connection.disconnect();

					m_renderMaterialInstances.erase(it);
				}
			});

		return *renderMaterialInstancePtr;
	}

	return *it->second.resource;
}

const std::vector<Ptr<RenderObject>>& RenderScene::getRenderObjects() const noexcept
{
	return m_renderObjects;
}

const std::vector<Ptr<RenderLight>>& RenderScene::getRenderLights() const noexcept
{
	return m_renderLights;
}

void RenderScene::updateResources()
{
	{
		ATEMA_BENCHMARK("Objects");

		for (auto& resource : m_renderObjects)
			resource->update();
	}

	{
		ATEMA_BENCHMARK("Lights");

		for (auto& resource : m_renderLights)
			resource->update();
	}

	{
		ATEMA_BENCHMARK("Materials");

		for (auto& [id, renderData] : m_renderMaterials)
			renderData.resource->update();
	}

	{
		ATEMA_BENCHMARK("Material Instances");

		for (auto& [id, renderData] : m_renderMaterialInstances)
			renderData.resource->update();
	}
}
