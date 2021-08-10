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

#include <Atema/Renderer/Renderer.hpp>
#include <Atema/Core/Window.hpp>

using namespace at;

Ptr<Renderer> Renderer::s_renderer = nullptr;

Renderer::Renderer(const Settings& settings) : m_settings(settings)
{
	m_mainWindow = Window::create(settings.mainWindowSettings);
}

Renderer::~Renderer()
{
}

Renderer& Renderer::getInstance()
{
	if (!s_renderer)
	{
		ATEMA_ERROR("No Renderer available");
	}

	return *s_renderer;
}

void Renderer::destroy()
{
	s_renderer.reset();
}

const Renderer::Settings& Renderer::getSettings() const noexcept
{
	return m_settings;
}

Ptr<Window> Renderer::getMainWindow() const noexcept
{
	return m_mainWindow;
}