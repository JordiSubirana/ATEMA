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

#include <Atema/Core/Application.hpp>
#include <Atema/Core/ApplicationLayer.hpp>
#include <Atema/Core/Timer.hpp>

using namespace at;

Application::Application() : m_close(false)
{
}

Application::~Application()
{
}

Application& Application::instance()
{
	static Application s_instance;

	return s_instance;
}

void Application::addLayer(ApplicationLayer* layer)
{
	m_layers.push_back(layer);
}

void Application::removeLayer(ApplicationLayer* layer)
{
	auto it = std::find(m_layers.begin(), m_layers.end(), layer);

	if (it != m_layers.end())
		m_layers.erase(it);
}

void Application::run()
{
	Timer timer;

	while (!m_close)
	{
		processEvents();

		updateLayers(timer.getStep());
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

		for (auto& layer : m_layers)
		{
			if (!event.isHandled())
				layer->onEvent(event);
		}

		m_currentEvents.pop();
	}

	m_currentEvents.swap(m_nextEvents);
}

void Application::updateLayers(TimeStep timeStep)
{
	for (auto& layer : m_layers)
	{
		layer->update(timeStep);
	}
}
