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

#include "Scene.hpp"
#include "Resources.hpp"

using namespace at;

ObjectData::ObjectData()
{
	transform = Matrix4f::identity();
}

ObjectFrameData::ObjectFrameData(const ObjectData& object, uint32_t frameCount, Ptr<DescriptorPool> descriptorPool)
{
	m_uniformBuffers.reserve(frameCount);
	m_descriptorSets.reserve(frameCount);

	for (uint32_t j = 0; j < frameCount; j++)
	{
		auto uniformBuffer = Buffer::create({ BufferUsage::Uniform, sizeof(UniformObjectElement), true });

		m_uniformBuffers.push_back(uniformBuffer);

		auto descriptorSet = descriptorPool->createSet();

		descriptorSet->update(0, uniformBuffer);
		descriptorSet->update(1, object.texture, object.sampler);

		m_descriptorSets.push_back(descriptorSet);
	}
}

Ptr<Buffer> ObjectFrameData::getBuffer(uint32_t frameIndex)
{
	return m_uniformBuffers[frameIndex];
}

Ptr<DescriptorSet> ObjectFrameData::getDescriptorSet(uint32_t frameIndex)
{
	return m_descriptorSets[frameIndex];
}

Scene::Scene(const at::Ptr<at::CommandPool>& commandPool)
{
	loadResources(commandPool);

	m_objects.reserve(object_count);

	const auto scale = 30.0f;
	const auto origin = -scale * (object_row / 2.0f);

	for (size_t i = 0; i < object_row; i++)
	{
		for (size_t j = 0; j < object_row; j++)
		{
			ObjectData object;
			
			object.vertexBuffer = m_modelData->vertexBuffer;
			object.indexBuffer = m_modelData->indexBuffer;
			object.indexCount = m_modelData->indexCount;
			object.texture = m_materialData->texture;
			object.sampler = m_materialData->sampler;

			object.position.x = scale * static_cast<float>(i) + origin;
			object.position.y = scale * static_cast<float>(j) + origin;

			m_objects.push_back(object);
		}
	}
}

Scene::~Scene()
{
	m_objects.clear();
}

void Scene::updateObjects(at::TimeStep timeStep)
{
	const auto scale = 30.0f;
	const auto origin = -scale * (object_row / 2.0f);

	const auto basisChange = rotation4f({ toRadians(90.0f), 0.0f, 0.0f });

	for (size_t i = 0; i < object_row; i++)
	{
		for (size_t j = 0; j < object_row; j++)
		{
			const auto index = i * object_row + j;

			const auto rotScale = 10.0f + 70.0f * (static_cast<float>(index) / object_count);

			auto& object = m_objects[index];

			object.rotation.z += timeStep.getSeconds() * toRadians(rotScale);

			auto rotationMatrix = rotation4f(object.rotation);
			auto translationMatrix = translation(object.position);

			object.transform = translationMatrix * rotationMatrix * basisChange;
		}
	}
}

const std::vector<ObjectData>& Scene::getObjects() const noexcept
{
	return m_objects;
}

void Scene::loadResources(const at::Ptr<at::CommandPool>& commandPool)
{
	m_modelData = std::make_shared<ModelData>(model_path, commandPool);

	m_materialData = std::make_shared<MaterialData>(model_texture_path, commandPool);
}
