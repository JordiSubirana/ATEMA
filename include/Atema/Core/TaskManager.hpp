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

#ifndef ATEMA_CORE_THREADPOOL_HPP
#define ATEMA_CORE_THREADPOOL_HPP

#include <Atema/Core/Config.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Core/Pointer.hpp>

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>

namespace at
{
	class ATEMA_CORE_API Task
	{
	public:
		Task() = delete;
		Task(const std::function<void(size_t)>& function);
		virtual ~Task();

		void start(size_t threadIndex);

		void wait() noexcept;
		
	private:
		std::function<void(size_t)> m_function;
		bool m_finished;
		std::mutex m_mutex;
		std::condition_variable m_condition;
	};

	class ATEMA_CORE_API TaskManager : public NonCopyable
	{
	public:
		TaskManager();
		TaskManager(size_t size);
		virtual ~TaskManager();
		
		static TaskManager& getInstance();

		size_t getSize() const noexcept;
		
		Ptr<Task> createTask(const std::function<void()>& function);
		Ptr<Task> createTask(const std::function<void(size_t)>& function);

	private:
		void initialize(size_t size);

		std::atomic_bool m_exit;
		std::vector<Ptr<std::thread>> m_threads;
		std::mutex m_taskMutex;
		std::queue<Ptr<Task>> m_tasks;
		std::condition_variable m_condition;
	};
}

#endif
