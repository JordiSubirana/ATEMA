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

#ifndef ATEMA_RENDERER_RENDERWINDOW_HPP
#define ATEMA_RENDERER_RENDERWINDOW_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Renderer/Enums.hpp>
#include <Atema/Renderer/RenderFrame.hpp>
#include <Atema/Window/Window.hpp>

namespace at
{
	class ATEMA_RENDERER_API RenderWindow : public Window
	{
	public:
		struct Settings : public Window::Settings
		{
			ImageFormat colorFormat = ImageFormat::BGRA8_SRGB;
			ImageFormat depthFormat = ImageFormat::D32_SFLOAT;
		};

		RenderWindow() = delete;
		virtual ~RenderWindow();

		static Ptr<RenderWindow> create(const Settings& settings);

		virtual ImageFormat getColorFormat() const noexcept = 0;
		virtual ImageFormat getDepthFormat() const noexcept = 0;

		virtual RenderFrame& acquireFrame() = 0;

		virtual Ptr<RenderPass> getRenderPass() const noexcept = 0;
		
	protected:
		RenderWindow(const RenderWindow::Settings& settings);
	};
}

#endif
