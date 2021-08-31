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

#include <Atema/Core/TaskManager.hpp>
#include <Atema/Core/Error.hpp>

#include <chrono>

using namespace at;
using namespace std::chrono_literals;

// Task
Task::Task(const std::function<void(size_t)>& function) :
	m_function(function),
	m_finished(false)
{
}

Task::~Task()
{
}

void Task::start(size_t threadIndex)
{
	m_function(threadIndex);

	{
		std::unique_lock<std::mutex> lock(m_mutex);
		
		m_finished = true;
	}

	m_condition.notify_all();
}

void Task::wait() noexcept
{
	std::unique_lock<std::mutex> lock(m_mutex);

	m_condition.wait(lock, [this]()
	{
		return m_finished;
	});
}

// TaskManager
TaskManager::TaskManager() :
	m_exit(false)
{
	// Number of cores or 4 by default
	static const size_t coreCount = std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 4;

	initialize(coreCount);
}

TaskManager::~TaskManager()
{
	m_exit = true;

	for (auto& thread : m_threads)
		thread->join();
}

TaskManager& TaskManager::instance()
{
	static TaskManager s_instance;

	return s_instance;
}

size_t TaskManager::getSize() const noexcept
{
	return m_threads.size();
}

Ptr<Task> TaskManager::createTask(const std::function<void()>& function)
{
	auto task = createTask([function](size_t)
		{
			function();
		});

	return task;
}

Ptr<Task> TaskManager::createTask(const std::function<void(size_t)>& function)
{
	auto task = std::make_shared<Task>(function);

	{
		std::unique_lock<std::mutex> lock(m_taskMutex);

		m_tasks.push(task);
	}

	m_condition.notify_one();

	return task;
}

void TaskManager::initialize(size_t size)
{
	m_threads.reserve(size);

	for (size_t i = 0; i < size; i++)
	{
		auto thread = std::make_shared<std::thread>([this, i]()
			{
				while (!m_exit)
				{
					Ptr<Task> task;

					{
						std::unique_lock<std::mutex> lock(m_taskMutex);

						m_condition.wait(lock, [this]()
							{
								return !m_tasks.empty() || m_exit;
							});

						if (m_exit)
							break;
						
						task = m_tasks.front();
						m_tasks.pop();
					}

					task->start(i);
				}
			});

		m_threads.push_back(thread);
	}
}
