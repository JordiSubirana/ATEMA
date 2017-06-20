/*
	Copyright 2017 Jordi SUBIRANA

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

#ifndef ATEMA_RENDERER_WINDOW_HPP
#define ATEMA_RENDERER_WINDOW_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Core/WindowHandle.hpp>
#include <Atema/Renderer/RenderTarget.hpp>
#include <string>

namespace at
{
	class ATEMA_RENDERER_API Window : public AbstractRenderTarget
	{
	public:
		class ATEMA_RENDERER_API Implementation : public AbstractRenderTarget::Implementation
		{
		public:
			Implementation();
			virtual ~Implementation() = default;

			virtual void setPosition(int x, int y) = 0;
			virtual void setSize(unsigned width, unsigned height) = 0;
			virtual void setTitle(const std::string& title) = 0;

			virtual void close() = 0;
			virtual bool shouldClose() const = 0;

			virtual void processEvents() = 0;
			virtual void swapBuffers() = 0;

			virtual WindowHandle getHandle() = 0;
			virtual const WindowHandle getHandle() const = 0;

			virtual void setDrawArea(int x, int y, unsigned w, unsigned h) = 0;

			virtual void clearColor(const Color& color) = 0;
			virtual void clearDepth() = 0;
		};

		Window() = delete;
		Window(unsigned width, unsigned height, const std::string& title, RenderSystem *system = nullptr);
		virtual ~Window();

		void setPosition(int x, int y);
		void setSize(unsigned width, unsigned height);
		void setTitle(const std::string& title);

		void close();
		bool shouldClose() const noexcept;

		void processEvents();
		void swapBuffers();

		WindowHandle getHandle();
		const WindowHandle getHandle() const;

		void setDrawArea(int x, int y, unsigned w, unsigned h) override;

		void clearColor(const Color& color) override;
		void clearDepth() override;

	private:
		Implementation *m_impl;
	};
}

#endif