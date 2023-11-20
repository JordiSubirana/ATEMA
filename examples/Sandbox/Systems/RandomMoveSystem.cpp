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

#include "RandomMoveSystem.hpp"
#include "../Components/RandomMoveComponent.hpp"
#include "../Settings.hpp"
#include "../Resources.hpp"

using namespace at;

namespace
{
	constexpr size_t targetThreadCount = 8;

	const size_t threadCount = std::min(targetThreadCount, TaskManager::instance().getSize());

	void moveEntity(Transform& transform, RandomMoveComponent& randomMove, float seconds)
	{
		Vector3f randomDir(randFloat(), randFloat(), randFloat());
		randomDir.normalize();

		/*Vector3f dir;
		dir.x = Math::lerp(randomMove.direction.x, randomDir.x, randomMove.strength);
		dir.y = Math::lerp(randomMove.direction.y, randomDir.y, randomMove.strength);
		dir.z = Math::lerp(randomMove.direction.z, randomDir.z, randomMove.strength);*/

		Vector3f dir;
		dir.x = randomMove.direction.x + randomDir.x * randomMove.strength;
		dir.y = randomMove.direction.y + randomDir.y * randomMove.strength;
		dir.z = randomMove.direction.z + randomDir.z * randomMove.strength;
		if (dir.getNorm() > 0.01f)
			dir.normalize();

		auto& pos = transform.getTranslation();
		Vector3f dirToCenter = randomMove.origin - pos;
		auto dist = dirToCenter.getNorm();
		auto rangePercent = dist / randomMove.radius;
		if (rangePercent > 0.3f)
		{
			dirToCenter /= dist;
			auto factor = rangePercent * 0.7f + 0.3f;

			dir.x = Math::lerp(dir.x, dirToCenter.x, factor);
			dir.y = Math::lerp(dir.y, dirToCenter.y, factor);
			dir.z = Math::lerp(dir.z, dirToCenter.z, factor);
			dir.normalize();
		}

		randomMove.direction = dir;

		transform.translate(dir * randomMove.speed * seconds);
	}
}

RandomMoveSystem::RandomMoveSystem() : System()
{
}

RandomMoveSystem::~RandomMoveSystem()
{
}

void RandomMoveSystem::update(TimeStep timeStep)
{
	if (!Settings::instance().moveObjects)
		return;

	float seconds = timeStep.getSeconds();

	auto& entityManager = getEntityManager();

	auto entities = entityManager.getUnion<Transform, RandomMoveComponent>();

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

		auto task = taskManager.createTask([this, &entities, firstIndex, lastIndex, seconds](size_t threadIndex)
			{
				uint32_t i = static_cast<uint32_t>(firstIndex);
				for (auto it = entities.begin() + firstIndex; it != entities.begin() + lastIndex; it++)
				{
					auto& transform = entities.get<Transform>(*it);
					auto& randomMove = entities.get<RandomMoveComponent>(*it);

					moveEntity(transform, randomMove, seconds);

					i++;
				}
			});

		tasks.push_back(task);

		firstIndex += size;
	}

	for (auto& task : tasks)
		task->wait();
}
