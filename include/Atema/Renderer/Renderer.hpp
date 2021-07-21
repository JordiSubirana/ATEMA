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

#ifndef ATEMA_RENDERER_RENDERER_HPP
#define ATEMA_RENDERER_RENDERER_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Core/Window.hpp>

namespace at
{
	class ATEMA_RENDERER_API Renderer
	{
	public:
		struct Settings
		{
			Window::Settings mainWindowSettings;

			size_t maxFramesInFlight = 2;

			bool sampleShading = true;
		};
		
		Renderer() = delete;
		virtual ~Renderer();

		virtual void initialize() = 0;

		const Settings& getSettings() const noexcept;
		
		Ptr<Window> getMainWindow() const noexcept;

	protected:
		Renderer(const Settings& settings);

	private:
		Settings m_settings;
		Ptr<Window> m_mainWindow;
	};
}

#endif
