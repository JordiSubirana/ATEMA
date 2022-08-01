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

#include <Atema/Core/Application.hpp>
#include <Atema/Core/Error.hpp>
#include <Atema/Core/Timer.hpp>
#include <Atema/Core/Benchmark.hpp>

#include <thread>

// Since Windows 10 October Update 20H2
// We need to explicitly set timer precision with timeBeginPeriod
// https://randomascii.wordpress.com/2020/10/04/windows-timer-resolution-the-great-rule-change/
#ifdef ATEMA_SYSTEM_WINDOWS
#include <windows.h>
#include <mmsystem.h>
#include <timeapi.h>
#endif

using namespace at;
using namespace std::chrono_literals;

namespace
{
	Application* s_instance = nullptr;

	constexpr auto durationMargin = 2ms;

	void sleepFor(const std::chrono::duration<double, std::milli>& duration)
	{
		const auto waitStart = std::chrono::high_resolution_clock::now();

		if (duration > durationMargin)
		{
#ifdef ATEMA_SYSTEM_WINDOWS
			timeBeginPeriod(1);
#endif

			// Take a margin to avoid sleeping for too long
			std::this_thread::sleep_for(duration - durationMargin);

#ifdef ATEMA_SYSTEM_WINDOWS
			timeEndPeriod(1);
#endif
		}

		auto waitEnd = std::chrono::high_resolution_clock::now();

		while (waitEnd - waitStart < duration)
		{
			waitEnd = std::chrono::high_resolution_clock::now();
		}
	}
}

Application::Application() :
	m_close(false),
	m_fpsLimit(0)
{
	ATEMA_ASSERT(!s_instance, "A single application must exist");
	
	s_instance = this;
}

Application::~Application()
{
	if (s_instance == this)
		s_instance = nullptr;
}

Application& Application::instance()
{
	ATEMA_ASSERT(s_instance, "No application was created");
	
	return *s_instance;
}

void Application::run()
{
	Timer timer;

	while (!m_close)
	{
		const auto frameBegin = std::chrono::high_resolution_clock::now();
		
		{
			ATEMA_BENCHMARK("Application::processEvents");

			processEvents();
		}

		update(timer.getStep());

		if (m_fpsLimit > 0)
		{
			const auto frameEnd = std::chrono::high_resolution_clock::now();

			const auto minFrameTime = std::chrono::microseconds(1000000ll / static_cast<long long>(m_fpsLimit));

			const auto frameTime = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - frameBegin);

			if (frameTime < minFrameTime)
			{
				ATEMA_BENCHMARK("Application idle (fps limit)");

				std::chrono::duration<double, std::milli> targetWaitDuration = minFrameTime - frameTime;

				sleepFor(targetWaitDuration);
			}
		}
	}
}

void Application::close()
{
	m_close = true;
}

void Application::pushEvent(Event& event)
{
	m_nextEvents.push(event);
}

void Application::setFpsLimit(uint32_t fpsLimit)
{
	m_fpsLimit = fpsLimit;
}

void Application::processEvents()
{
	while (!m_currentEvents.empty())
	{
		auto& event = m_currentEvents.front();

		if (!event.isHandled())
			onEvent(event);

		m_currentEvents.pop();
	}

	m_currentEvents.swap(m_nextEvents);
}
