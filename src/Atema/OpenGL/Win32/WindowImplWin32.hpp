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

#ifndef ATEMA_RENDERER_WINDOWIMPL_WIN32_HPP
#define ATEMA_RENDERER_WINDOWIMPL_WIN32_HPP

#include <Atema/Config.hpp>
#include <Atema/OpenGL/Window.hpp>
#include <atomic>

#ifdef ATEMA_SYSTEM_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX

namespace at
{
	class OpenGLWindow::Impl
	{
	public:
		Impl();
		virtual ~Impl();

		void setPosition(int x, int y);
		void setSize(unsigned width, unsigned height);
		void setTitle(const std::string& title);

		void close();
		bool shouldClose() const noexcept;

		void processEvents();
		void swapBuffers();

		WindowHandle getHandle();
		const WindowHandle getHandle() const;

	private:
		void create(unsigned w, unsigned h, const std::string& title);
		void createContext();
		static LRESULT CALLBACK globalProcessMessage(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);
		static std::atomic_uint s_count;

		bool processMessage(UINT message, WPARAM wparam, LPARAM lparam);

		WindowHandle m_handle;
		bool m_shouldClose;
		bool m_fullscreen;

		struct
		{
			HDC hdc;
			HGLRC hglrc;
		} m_context;
	};
}

#endif

#endif