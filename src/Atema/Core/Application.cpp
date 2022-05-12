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

using namespace at;

namespace
{
	Application* s_instance = nullptr;
}

Application::Application() : m_close(false)
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
		processEvents();

		update(timer.getStep());
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
