// ----------------------------------------------------------------------
// Copyright (C) 2016 Jordi SUBIRANA
//
// This file is part of ATEMA.
//
// ATEMA is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ATEMA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ATEMA.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------

#include "WindowImpl_Win32.hpp"

#if defined(ATEMA_SYSTEM_WINDOWS)

#include <Atema/Core/Error.hpp>

#include <cstring>
#include <locale>
#include <codecvt>

#define ATEMA_CLASS_NAME L"Atema Window"

namespace at
{	
	//STATIC
	std::atomic_uint Window::Impl::s_count;
	
	LRESULT CALLBACK Window::Impl::global_process_message(HWND handle, UINT message, WPARAM wparam, LPARAM lparam)
	{
		if (message == WM_CREATE)
		{
			LONG_PTR window_ptr = reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCT*>(lparam)->lpCreateParams);

			SetWindowLongPtr(handle, GWLP_USERDATA, window_ptr);
		}
		
		Window::Impl *window = reinterpret_cast<Window::Impl*>(GetWindowLongPtr(handle, GWLP_USERDATA));
		
		if (window)
		{
			if(window->process_message(message, wparam, lparam))
				return (0);
		}
		
		return (DefWindowProcW(handle, message, wparam, lparam));
	}
	
	//PUBLIC
	Window::Impl::Impl() :
		m_handle(),
		m_should_close(false),
		m_fullscreen(false)
	{
		static bool init_ok = false;
		
		s_count++;
		
		if (!init_ok)
		{
			WNDCLASSW window_class;
			window_class.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			window_class.lpfnWndProc   = &Window::Impl::global_process_message;
			window_class.cbClsExtra    = 0;
			window_class.cbWndExtra    = 0;
			window_class.hInstance     = GetModuleHandle(nullptr);
			window_class.hIcon         = nullptr;
			window_class.hCursor       = nullptr;
			window_class.hbrBackground = nullptr;
			window_class.lpszMenuName  = nullptr;
			window_class.lpszClassName = ATEMA_CLASS_NAME;
			RegisterClassW(&window_class);
			
			s_count = 1;
		}
	}
	
	Window::Impl::~Impl()
	{
		s_count--;
		
		close();
		
		if (s_count == 0)
		{
			UnregisterClassW(ATEMA_CLASS_NAME, GetModuleHandle(nullptr));
		}
	}
	
	void Window::Impl::reset(unsigned int w, unsigned int h, const std::string& name, const Window::Style& style)
	{
		DWORD win32_style = 0;
		int x = 0;
		int y = 0;
		int width = static_cast<int>(w);
		int height = static_cast<int>(h);
		
		if (style.fullscreen)
		{
			DEVMODE win32_mode;
			
			std::memset(&win32_mode, 0, sizeof(DEVMODE));
			
			win32_mode.dmBitsPerPel = 32;
			win32_mode.dmPelsWidth  = width;
			win32_mode.dmPelsHeight = height;
			win32_mode.dmFields	   = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			win32_mode.dmSize	   = sizeof(DEVMODE);

			m_fullscreen = true;
			
			if (ChangeDisplaySettings(&win32_mode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
				ATEMA_ERROR("Video mode not supported.")
		}
		else
		{
			RECT rectangle = {0, 0, width, height};
			
			win32_style |= WS_VISIBLE;
			
			if (style.frame)
			{
				win32_style |= WS_CAPTION | WS_MINIMIZEBOX;
				
				if (style.resizable)
					win32_style |= WS_MAXIMIZEBOX | WS_SIZEBOX;
				
				if (style.closable)
					win32_style |= WS_SYSMENU;
			}
			else
			{
				win32_style |= WS_POPUP;
			}
			
			AdjustWindowRect(&rectangle, win32_style, false);
			width  = rectangle.right - rectangle.left;
			height = rectangle.bottom - rectangle.top;
			
			m_fullscreen = false;
		}
		
		x = (GetSystemMetrics(SM_CXSCREEN) - width)/2;
		y = (GetSystemMetrics(SM_CYSCREEN) - height)/2;
		
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::wstring name_w = converter.from_bytes(name.c_str());
		m_handle = CreateWindowW(ATEMA_CLASS_NAME, name_w.data(), win32_style, x, y, width, height, nullptr, nullptr, GetModuleHandle(nullptr), this);
		
		HCURSOR cursor = static_cast<HCURSOR>(LoadImage(nullptr, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_SHARED));
		
		SetCursor(cursor);
	}
	
	void Window::Impl::set_position(int x, int y)
	{
		SetWindowPos(m_handle, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
	
	void Window::Impl::set_title(const std::string& name)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::wstring name_w = converter.from_bytes(name.c_str());
		SetWindowTextW(m_handle, name_w.data());
	}
	
	void Window::Impl::close()
	{
		if (m_handle)
			DestroyWindow(m_handle);
	}
	
	bool Window::Impl::should_close() const noexcept
	{
		return (m_should_close);
	}
	
	void Window::Impl::process_events()
	{
		MSG message;
		
		while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessageW(&message);
		}
	}
	
	WindowHandle Window::Impl::get_handle()
	{
		return (m_handle);
	}
	
	const WindowHandle Window::Impl::get_handle() const
	{
		return (m_handle);
	}
	
	//PRIVATE
	bool Window::Impl::process_message(UINT message, WPARAM wparam, LPARAM lparam)
	{
		switch (message)
		{
			case WM_DESTROY:
				break;
			case WM_CLOSE:
				m_should_close = true;
				return (true);
			default:
				break;
		}
		
		return (false);
	}
}

#undef ATEMA_CLASS_NAME

#endif