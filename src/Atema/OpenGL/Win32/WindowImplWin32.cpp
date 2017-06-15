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

#include "WindowImplWin32.hpp"
#include <glad/glad_wgl.h>
#include <mutex>

#ifdef ATEMA_SYSTEM_WINDOWS

#include <Atema/Core/Error.hpp>

#include <locale>
#include <codecvt>

#define ATEMA_CLASS_NAME L"AtemaWindow"

namespace at
{
	//STATIC
	std::atomic_uint OpenGLWindow::Impl::s_count = 0;

	LRESULT CALLBACK OpenGLWindow::Impl::globalProcessMessage(HWND handle, UINT message, WPARAM wparam, LPARAM lparam)
	{
		if (message == WM_CREATE)
		{
			LONG_PTR window_ptr = reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCT*>(lparam)->lpCreateParams);

			SetWindowLongPtr(handle, GWLP_USERDATA, window_ptr);
		}

		OpenGLWindow::Impl *window = reinterpret_cast<OpenGLWindow::Impl*>(GetWindowLongPtr(handle, GWLP_USERDATA));

		if (window)
		{
			if (window->processMessage(message, wparam, lparam))
				return (0);
		}

		return (DefWindowProcW(handle, message, wparam, lparam));
	}

	//PUBLIC
	OpenGLWindow::Impl::Impl() :
		m_handle(),
		m_shouldClose(false),
		m_fullscreen(false)
	{
		if (!s_count)
		{
			WNDCLASSW window_class;
			window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			window_class.lpfnWndProc = &OpenGLWindow::Impl::globalProcessMessage;
			window_class.cbClsExtra = 0;
			window_class.cbWndExtra = 0;
			window_class.hInstance = GetModuleHandle(nullptr);
			window_class.hIcon = nullptr;
			window_class.hCursor = nullptr;
			window_class.hbrBackground = nullptr;
			window_class.lpszMenuName = nullptr;
			window_class.lpszClassName = ATEMA_CLASS_NAME;
			RegisterClassW(&window_class);

			s_count = 1;
		}

		create(256, 256, ""); // Default values for the window

		s_count++;
	}

	OpenGLWindow::Impl::~Impl()
	{
		Impl::close();
	}

	void OpenGLWindow::Impl::create(unsigned w, unsigned h, const std::string& title)
	{
		DWORD win32_style = 0;
		int x;
		int y;
		int width = static_cast<int>(w);
		int height = static_cast<int>(h);

		//TODO: Make window style !
		bool fullscreen = false;
		bool frame = true;
		bool resizable = true;
		bool closable = true;

		if (fullscreen)
		{
			DEVMODE win32_mode;

			std::memset(&win32_mode, 0, sizeof(DEVMODE));

			//TODO: Make these parameters custom
			win32_mode.dmBitsPerPel = 32;
			win32_mode.dmPelsWidth = width;
			win32_mode.dmPelsHeight = height;
			win32_mode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			win32_mode.dmSize = sizeof(DEVMODE);

			m_fullscreen = true;

			if (ChangeDisplaySettings(&win32_mode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
				ATEMA_ERROR("Video mode not supported.");
		}
		else
		{
			RECT rectangle = { 0, 0, width, height };

			win32_style |= WS_VISIBLE;

			if (frame)
			{
				win32_style |= WS_CAPTION | WS_MINIMIZEBOX;

				if (resizable)
					win32_style |= WS_MAXIMIZEBOX | WS_SIZEBOX;

				if (closable)
					win32_style |= WS_SYSMENU;
			}
			else
			{
				win32_style |= WS_POPUP;
			}

			AdjustWindowRect(&rectangle, win32_style, false);
			width = rectangle.right - rectangle.left;
			height = rectangle.bottom - rectangle.top;

			m_fullscreen = false;
		}

		x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
		y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::wstring name_w = converter.from_bytes(title.c_str());
		m_handle = CreateWindowW(ATEMA_CLASS_NAME, name_w.data(), win32_style, x, y, width, height, nullptr, nullptr, GetModuleHandle(nullptr), this);

		HCURSOR cursor = static_cast<HCURSOR>(LoadImage(nullptr, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_SHARED));

		SetCursor(cursor);

		createContext();
	}

	void OpenGLWindow::Impl::createContext()
	{
		int pixel_format;
		HGLRC tmp_context;

		PIXELFORMATDESCRIPTOR descriptor;
		memset(&descriptor, 0, sizeof(PIXELFORMATDESCRIPTOR));
		descriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		descriptor.nVersion = 1;

		//TODO: Make these parameters custom
		descriptor.cColorBits = 32;
		descriptor.cAlphaBits = 8; // 8 if 32-bits color
		descriptor.cDepthBits = 24;
		descriptor.cStencilBits = 0;
		descriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		descriptor.iPixelType = PFD_TYPE_RGBA;

		m_context.hdc = GetDC(m_handle);

		if (!m_context.hdc)
			ATEMA_ERROR("Invalid device context.");

		pixel_format = ChoosePixelFormat(m_context.hdc, &descriptor);

		if (!pixel_format)
			ATEMA_ERROR("Pixel format choice failed.");

		if (!SetPixelFormat(m_context.hdc, pixel_format, &descriptor))
			ATEMA_ERROR("Pixel format choice failed.");

		tmp_context = wglCreateContext(m_context.hdc);
		wglMakeCurrent(m_context.hdc, tmp_context);

		if (!gladLoadWGL(m_context.hdc))
			ATEMA_ERROR("Loading of WGL extensions failed.");

		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(tmp_context);

		//Update of context settings about pixel format (needed?)
		/*
		if (DescribePixelFormat(m_context.hdc, pixel_format, sizeof(PIXELFORMATDESCRIPTOR), &descriptor) != 0)
		{
		settings.bitsPerPixel = descriptor.cColorBits + descriptor.cAlphaBits;
		settings.depthBits = descriptor.cDepthBits;
		settings.stencilBits = descriptor.cDepthBits;
		}//*/

		HGLRC targetContext = nullptr; // Context to share

		if (wglCreateContextAttribsARB)
		{
			// Atema needs at least OpenGL 3.3
			int attributes[] =
			{
				WGL_CONTEXT_MAJOR_VERSION_ARB,	3,
				WGL_CONTEXT_MINOR_VERSION_ARB,	3,
				WGL_CONTEXT_PROFILE_MASK_ARB,	WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
				0 // End of attributes
			};

			m_context.hglrc = wglCreateContextAttribsARB(m_context.hdc, targetContext, attributes);
		}

		// The creation failed. We try another method
		if (!m_context.hglrc)
		{
			m_context.hglrc = wglCreateContext(m_context.hdc);

			if (targetContext)
			{
				static std::mutex mutex;
				std::lock_guard<std::mutex> locker(mutex);

				if (!wglShareLists(targetContext, m_context.hglrc))
					ATEMA_ERROR("Sharing OpenGL context failed.");
			}
		}

		if (!m_context.hglrc)
			ATEMA_ERROR("OpenGL context creation failed.");

		wglMakeCurrent(m_context.hdc, m_context.hglrc);
	}

	void OpenGLWindow::Impl::setPosition(int x, int y)
	{
		SetWindowPos(m_handle, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

	void OpenGLWindow::Impl::setSize(unsigned width, unsigned height)
	{
		SetWindowPos(m_handle, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
	}

	void OpenGLWindow::Impl::setTitle(const std::string& title)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::wstring name_w = converter.from_bytes(title.c_str());
		SetWindowTextW(m_handle, name_w.data());
	}

	void OpenGLWindow::Impl::close()
	{
		if (m_handle)
		{
			DestroyWindow(m_handle);

			s_count--;

			if (wglGetCurrentContext() == m_context.hglrc)
				wglMakeCurrent(nullptr, nullptr);

			wglDeleteContext(m_context.hglrc);

			ReleaseDC(m_handle, m_context.hdc);

			if (s_count == 0)
			{
				UnregisterClassW(ATEMA_CLASS_NAME, GetModuleHandle(nullptr));
			}
		}

		m_handle = nullptr;
	}

	bool OpenGLWindow::Impl::shouldClose() const noexcept
	{
		return (m_shouldClose);
	}

	void OpenGLWindow::Impl::processEvents()
	{
		MSG message;

		while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessageW(&message);
		}
	}

	void OpenGLWindow::Impl::swapBuffers()
	{
		SwapBuffers(m_context.hdc);
	}

	WindowHandle OpenGLWindow::Impl::getHandle()
	{
		return (m_handle);
	}

	const WindowHandle OpenGLWindow::Impl::getHandle() const
	{
		return (m_handle);
	}

	//PRIVATE
	bool OpenGLWindow::Impl::processMessage(UINT message, WPARAM wparam, LPARAM lparam)
	{
		switch (message)
		{
			case WM_DESTROY:
				break;
			case WM_CLOSE:
				m_shouldClose = true;
				return (true);
			default:
				break;
		}

		return (false);
	}
}

#undef ATEMA_CLASS_NAME

#endif