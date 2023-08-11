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

#include "SceneUpdateSystem.hpp"
#include "../Components/VelocityComponent.hpp"
#include "../Settings.hpp"

using namespace at;

namespace
{
	constexpr size_t targetThreadCount = 8;

	const size_t threadCount = std::min(targetThreadCount, TaskManager::instance().getSize());
}

SceneUpdateSystem::SceneUpdateSystem() : System()
{
}

SceneUpdateSystem::~SceneUpdateSystem()
{
}

void SceneUpdateSystem::update(TimeStep timeStep)
{
	if (!Settings::instance().moveObjects)
		return;

	auto& entityManager = getEntityManager();

	auto entities = entityManager.getUnion<Transform, VelocityComponent>();

	auto& taskManager = TaskManager::instance();

	std::vector<Ptr<Task>> tasks;
	tasks.reserve(threadCount);

	size_t firstIndex = 0;
	const size_t size = entities.size() / threadCount;

	for (size_t taskIndex = 0; taskIndex < threadCount; taskIndex++)
	{
		auto lastIndex = firstIndex + size;

		if (taskIndex == threadCount - 1)
		{
			const auto remainingSize = entities.size() - lastIndex;

			lastIndex += remainingSize;
		}

		auto task = taskManager.createTask([this, &entities, firstIndex, lastIndex, timeStep](size_t threadIndex)
			{
				uint32_t i = static_cast<uint32_t>(firstIndex);
				for (auto it = entities.begin() + firstIndex; it != entities.begin() + lastIndex; it++)
				{
					auto& transform = entities.get<Transform>(*it);
					auto& velocity = entities.get<VelocityComponent>(*it);

					Vector3f rotation;
					rotation.z = velocity.speed * timeStep.getSeconds();

					transform.rotate(rotation);

					i++;
				}
			});

		tasks.push_back(task);

		firstIndex += size;
	}

	for (auto& task : tasks)
		task->wait();
}
