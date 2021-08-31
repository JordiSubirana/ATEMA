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

Scene::Scene()
{
	loadResources();

	m_objects.reserve(objectCount);

	const auto origin = -modelScale * (objectRow / 2.0f);

	for (size_t i = 0; i < objectRow; i++)
	{
		for (size_t j = 0; j < objectRow; j++)
		{
			ObjectData object;
			
			object.vertexBuffer = m_modelData->vertexBuffer;
			object.indexBuffer = m_modelData->indexBuffer;
			object.indexCount = m_modelData->indexCount;
			object.texture = m_materialData->texture;
			object.sampler = m_materialData->sampler;

			object.position.x = modelScale * static_cast<float>(i) + origin;
			object.position.y = modelScale * static_cast<float>(j) + origin;

			m_objects.push_back(object);
		}
	}
}

Scene::~Scene()
{
	m_objects.clear();
}

void Scene::updateObjects(at::TimeStep timeStep, size_t threadCount)
{
	const auto origin = -modelScale * (objectRow / 2.0f);

	const auto basisChange = rotation4f({ toRadians(90.0f), 0.0f, 0.0f });

	auto& taskManager = TaskManager::instance();

	// Divide the updates in max groups
	const size_t taskCount = threadCount ? threadCount : taskManager.getSize();

	std::vector<Ptr<Task>> tasks;
	tasks.reserve(taskCount);

	size_t firstIndex = 0;
	size_t size = m_objects.size() / taskCount;

	for (size_t i = 0; i < taskCount; i++)
	{
		auto lastIndex = firstIndex + size;

		if (i == taskCount - 1)
		{
			const auto remainingSize = m_objects.size() - lastIndex;

			lastIndex += remainingSize;
		}

		auto task = taskManager.createTask([this, firstIndex, lastIndex, timeStep, basisChange]()
			{
				for (size_t j = firstIndex; j < lastIndex; j++)
				{
					const auto rotScale = 10.0f + 250.0f * (static_cast<float>(j) / objectCount);

					auto& object = m_objects[j];

					object.rotation.z += timeStep.getSeconds() * toRadians(rotScale);

					auto rotationMatrix = rotation4f(object.rotation);
					auto translationMatrix = translation(object.position);

					object.transform = translationMatrix * rotationMatrix * basisChange;
				}
			});

		tasks.push_back(task);

		firstIndex += size;
	}

	for (auto& task : tasks)
		task->wait();
}

const std::vector<ObjectData>& Scene::getObjects() const noexcept
{
	return m_objects;
}

void Scene::loadResources()
{
	m_modelData = std::make_shared<ModelData>(modelMeshPath);

	m_materialData = std::make_shared<MaterialData>(modelTexturePath);
}
