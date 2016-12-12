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

#include "WindowInputHandlerImpl_Win32.hpp"

#if defined(ATEMA_SYSTEM_WINDOWS)

#include <Atema/Core/Error.hpp>

#include <uchar.h>

namespace at
{	
	std::mutex WindowInputHandler::Impl::s_mutex;
	std::unordered_map<WindowHandle, std::list<WindowInputHandler::Impl*>> WindowInputHandler::Impl::s_handlers;
	
	//STATIC
	LRESULT CALLBACK WindowInputHandler::Impl::global_process_message(HWND handle, UINT message, WPARAM wparam, LPARAM lparam)
	{
		auto handlers = s_handlers.find(handle);
		
		if (handlers != s_handlers.end())
		{
			for (auto handler : handlers->second)
			{
				handler->process_message(message, wparam, lparam);
				
				if (handler->m_callback)
					return (CallWindowProcW(reinterpret_cast<WNDPROC>(handler->m_callback), handle, message, wparam, lparam));
			}
		}
		
		return (DefWindowProcW(handle, message, wparam, lparam));
	}
	
	//PUBLIC
	WindowInputHandler::Impl::Impl() :
		m_handle(nullptr),
		m_callback(0)
	{
		
	}
	
	WindowInputHandler::Impl::~Impl() noexcept
	{
		try
		{
			std::lock_guard<std::mutex> lock(s_mutex);
			
			if (m_handle)
				s_handlers[m_handle].remove(this);
		}
		catch (...)
		{
			
		}
	}
	
	void WindowInputHandler::Impl::reset(WindowHandle handle)
	{
		std::lock_guard<std::mutex> lock(s_mutex);
		LONG_PTR error;
		
		m_handle = handle;
		
		if (!m_handle)
			return;
		
		m_callback = GetWindowLongPtrW(m_handle, GWLP_WNDPROC);
		
		SetLastError(0);
		error = SetWindowLongPtrW(m_handle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(global_process_message));
		
		if (GetLastError)
			ATEMA_ERROR("Could not handle properly the events.")
		
		if (m_handle)
			s_handlers[m_handle].push_back(this);
	}
	
	WindowHandle WindowInputHandler::Impl::get_handle()
	{
		return (m_handle);
	}
	
	const WindowHandle WindowInputHandler::Impl::get_handle() const
	{
		return (m_handle);
	}
	
	//PRIVATE
	void WindowInputHandler::Impl::process_message(UINT message, WPARAM wparam, LPARAM lparam)
	{
		/*
		switch (message)
		{
			case WM_DESTROY:
			{
				break;
			}	
			case WM_WINDOWPOSCHANGING:
			{
				WINDOWPOS* pos = reinterpret_cast<WINDOWPOS*>(lParam);
				
				break;
			}
			case WM_CHAR:
			{
				// http://msdn.microsoft.com/en-us/library/ms646267(VS.85).aspx
				char32_t character = static_cast<char32_t>(wParam);
				bool repeated = (HIWORD(lParam) & KF_REPEAT);

				break;
			}
			case WM_CLOSE:
			{
				break;
			}
			case WM_ENTERSIZEMOVE:
			{
				break;
			}
			case WM_EXITSIZEMOVE:
			{
				break;
			}
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			{
				// http://msdn.microsoft.com/en-us/library/ms646267(VS.85).aspx
				bool repeated = ((HIWORD(lParam) & KF_REPEAT) != 0);
				if (m_keyRepeat || !repeated)
				{
					WindowEvent event;
					event.type = WindowEventType_KeyPressed;
					event.key.code = ConvertVirtualKey(wParam, lParam);
					event.key.alt = ((GetAsyncKeyState(VK_MENU) & 0x8000) != 0);
					event.key.control = ((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0);
					event.key.repeated = repeated;
					event.key.shift = ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0);
					event.key.system = (((GetAsyncKeyState(VK_LWIN) & 0x8000) != 0) || ((GetAsyncKeyState(VK_RWIN) & 0x8000) != 0));
					m_parent->PushEvent(event);
				}

				break;
			}
			case WM_KEYUP:
			case WM_SYSKEYUP:
			{
				// http://msdn.microsoft.com/en-us/library/ms646267(VS.85).aspx
				WindowEvent event;
				event.type = WindowEventType_KeyReleased;
				event.key.code = ConvertVirtualKey(wParam, lParam);
				event.key.alt = ((GetAsyncKeyState(VK_MENU) & 0x8000) != 0);
				event.key.control = ((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0);
				event.key.shift = ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0);
				event.key.system = ((GetAsyncKeyState(VK_LWIN) & 0x8000) || (GetAsyncKeyState(VK_RWIN) & 0x8000));
				m_parent->PushEvent(event);

				break;
			}
			case WM_KILLFOCUS:
			{
				break;
			}
			case WM_LBUTTONDBLCLK:
			{
				// Cet évènement est généré à la place d'un WM_LBUTTONDOWN lors d'un double-clic.
				// Comme nous désirons quand même notifier chaque clic, nous envoyons les deux évènements.
				WindowEvent event;
				event.mouseButton.button = Mouse::Left;
				event.mouseButton.x = GET_X_LPARAM(lParam);
				event.mouseButton.y = GET_Y_LPARAM(lParam);

				event.type = WindowEventType_MouseButtonDoubleClicked;
				m_parent->PushEvent(event);

				event.type = WindowEventType_MouseButtonPressed;
				m_parent->PushEvent(event);

				break;
			}
			case WM_LBUTTONDOWN:
			{
				WindowEvent event;
				event.type = WindowEventType_MouseButtonPressed;
				event.mouseButton.button = Mouse::Left;
				event.mouseButton.x = GET_X_LPARAM(lParam);
				event.mouseButton.y = GET_Y_LPARAM(lParam);
				m_parent->PushEvent(event);

				break;
			}
			case WM_LBUTTONUP:
			{
				WindowEvent event;
				event.type = WindowEventType_MouseButtonReleased;
				event.mouseButton.button = Mouse::Left;
				event.mouseButton.x = GET_X_LPARAM(lParam);
				event.mouseButton.y = GET_Y_LPARAM(lParam);
				m_parent->PushEvent(event);

				break;
			}
			case WM_MBUTTONDBLCLK:
			{
				WindowEvent event;
				event.mouseButton.button = Mouse::Middle;
				event.mouseButton.x = GET_X_LPARAM(lParam);
				event.mouseButton.y = GET_Y_LPARAM(lParam);

				event.type = WindowEventType_MouseButtonDoubleClicked;
				m_parent->PushEvent(event);

				event.type = WindowEventType_MouseButtonPressed;
				m_parent->PushEvent(event);

				break;
			}
			case WM_MBUTTONDOWN:
			{
				WindowEvent event;
				event.type = WindowEventType_MouseButtonPressed;
				event.mouseButton.button = Mouse::Middle;
				event.mouseButton.x = GET_X_LPARAM(lParam);
				event.mouseButton.y = GET_Y_LPARAM(lParam);
				m_parent->PushEvent(event);

				break;
			}
			case WM_MBUTTONUP:
			{
				WindowEvent event;
				event.type = WindowEventType_MouseButtonReleased;
				event.mouseButton.button = Mouse::Middle;
				event.mouseButton.x = GET_X_LPARAM(lParam);
				event.mouseButton.y = GET_Y_LPARAM(lParam);
				m_parent->PushEvent(event);

				break;
			}
			// Nécessite un appel précédent à TrackMouseEvent (Fait dans WM_MOUSEMOVE)
			// http://msdn.microsoft.com/en-us/library/windows/desktop/ms645615(v=vs.85).aspx
			case WM_MOUSELEAVE:
			{
				m_mouseInside = false;

				WindowEvent event;
				event.type = WindowEventType_MouseLeft;
				m_parent->PushEvent(event);
				break;
			}
			case WM_MOUSEMOVE:
			{
				int currentX = GET_X_LPARAM(lParam);
				int currentY = GET_Y_LPARAM(lParam);

				if (!m_mouseInside)
				{
					m_mouseInside = true;

					// On créé un évènement pour être informé de la sortie de la fenêtre
					TRACKMOUSEEVENT mouseEvent;
					mouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
					mouseEvent.dwFlags = TME_LEAVE;
					mouseEvent.hwndTrack = m_handle;

					TrackMouseEvent(&mouseEvent);

					WindowEvent event;
					event.type = WindowEventType_MouseEntered;
					m_parent->PushEvent(event);

					event.type = WindowEventType_MouseMoved;

					// Le delta sera 0
					event.mouseMove.deltaX = 0;
					event.mouseMove.deltaY = 0;

					event.mouseMove.x = currentX;
					event.mouseMove.y = currentY;

					m_mousePos.x = currentX;
					m_mousePos.y = currentY;

					m_parent->PushEvent(event);
					break;
				}

				// Si la souris n'a pas bougé (Ou qu'on veut ignorer l'évènement)
				if (m_mousePos.x == currentX && m_mousePos.y == currentY)
					break;

				WindowEvent event;
				event.type = WindowEventType_MouseMoved;
				event.mouseMove.deltaX = currentX - m_mousePos.x;
				event.mouseMove.deltaY = currentY - m_mousePos.y;
				event.mouseMove.x = currentX;
				event.mouseMove.y = currentY;

				m_mousePos.x = currentX;
				m_mousePos.y = currentY;

				m_parent->PushEvent(event);
				break;
			}
			case WM_MOUSEWHEEL:
			{
				if (m_smoothScrolling)
				{
					WindowEvent event;
					event.type = WindowEventType_MouseWheelMoved;
					event.mouseWheel.delta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam))/WHEEL_DELTA;
					m_parent->PushEvent(event);
				}
				else
				{
					m_scrolling += GET_WHEEL_DELTA_WPARAM(wParam);
					if (std::abs(m_scrolling) >= WHEEL_DELTA)
					{
						WindowEvent event;
						event.type = WindowEventType_MouseWheelMoved;
						event.mouseWheel.delta = static_cast<float>(m_scrolling/WHEEL_DELTA);
						m_parent->PushEvent(event);

						m_scrolling %= WHEEL_DELTA;
					}
				}
				break;
			}
			case WM_MOVE:
			{
				if (m_sizemove && (m_style & WindowStyle_Threaded) == 0)
					break;

				RECT windowRect;
				GetWindowRect(m_handle, &windowRect);

				Vector2i position(windowRect.left, windowRect.top);
				if (m_position != position)
				{
					m_position = position;

					WindowEvent event;
					event.type = WindowEventType_Moved;
					event.position.x = position.x;
					event.position.y = position.y;
					m_parent->PushEvent(event);
				}
				break;
			}
			case WM_RBUTTONDBLCLK:
			{
				WindowEvent event;
				event.mouseButton.button = Mouse::Right;
				event.mouseButton.x = GET_X_LPARAM(lParam);
				event.mouseButton.y = GET_Y_LPARAM(lParam);

				event.type = WindowEventType_MouseButtonDoubleClicked;
				m_parent->PushEvent(event);

				event.type = WindowEventType_MouseButtonPressed;
				m_parent->PushEvent(event);

				break;
			}
			case WM_RBUTTONDOWN:
			{
				WindowEvent event;
				event.type = WindowEventType_MouseButtonPressed;
				event.mouseButton.button = Mouse::Right;
				event.mouseButton.x = GET_X_LPARAM(lParam);
				event.mouseButton.y = GET_Y_LPARAM(lParam);
				m_parent->PushEvent(event);

				break;
			}
			case WM_RBUTTONUP:
			{
				WindowEvent event;
				event.type = WindowEventType_MouseButtonReleased;
				event.mouseButton.button = Mouse::Right;
				event.mouseButton.x = GET_X_LPARAM(lParam);
				event.mouseButton.y = GET_Y_LPARAM(lParam);
				m_parent->PushEvent(event);

				break;
			}
			case WM_SETFOCUS:
			{
				WindowEvent event;
				event.type = WindowEventType_GainedFocus;
				m_parent->PushEvent(event);

				break;
			}
			case WM_SIZE:
			{
				if (m_sizemove && (m_style & WindowStyle_Threaded) == 0)
					break;

				if (wParam == SIZE_MINIMIZED)
					break;

				RECT rect;
				GetClientRect(m_handle, &rect);

				Vector2ui size(rect.right-rect.left, rect.bottom-rect.top); // On récupère uniquement la taille de la zone client
				if (m_size == size)
					break;

				m_size = size;

				WindowEvent event;
				event.type = WindowEventType_Resized;
				event.size.width = size.x;
				event.size.height = size.y;
				m_parent->PushEvent(event);
				break;
			}
			case WM_UNICHAR:
			{
				// http://msdn.microsoft.com/en-us/library/windows/desktop/ms646288(v=vs.85).aspx
				if (wParam != UNICODE_NOCHAR)
				{
					bool repeated = ((HIWORD(lParam) & KF_REPEAT) != 0);
					if (m_keyRepeat || !repeated)
					{
						WindowEvent event;
						event.type = WindowEventType_TextEntered;
						event.text.character = static_cast<char32_t>(wParam);
						event.text.repeated = repeated;
						m_parent->PushEvent(event);
					}

					return true;
				}
			}
			case WM_XBUTTONDBLCLK:
			{
				WindowEvent event;
				if (HIWORD(wParam) == XBUTTON1)
					event.mouseButton.button = Mouse::XButton1;
				else
					event.mouseButton.button = Mouse::XButton2;

				event.mouseButton.x = GET_X_LPARAM(lParam);
				event.mouseButton.y = GET_Y_LPARAM(lParam);

				event.type = WindowEventType_MouseButtonDoubleClicked;
				m_parent->PushEvent(event);

				event.type = WindowEventType_MouseButtonPressed;
				m_parent->PushEvent(event);

				break;
			}
			case WM_XBUTTONDOWN:
			{
				WindowEvent event;
				event.type = WindowEventType_MouseButtonPressed;

				if (HIWORD(wParam) == XBUTTON1)
					event.mouseButton.button = Mouse::XButton1;
				else
					event.mouseButton.button = Mouse::XButton2;

				event.mouseButton.x = GET_X_LPARAM(lParam);
				event.mouseButton.y = GET_Y_LPARAM(lParam);
				m_parent->PushEvent(event);

				break;
			}
			case WM_XBUTTONUP:
			{
				WindowEvent event;
				event.type = WindowEventType_MouseButtonReleased;

				if (HIWORD(wParam) == XBUTTON1)
					event.mouseButton.button = Mouse::XButton1;
				else
					event.mouseButton.button = Mouse::XButton2;

				event.mouseButton.x = GET_X_LPARAM(lParam);
				event.mouseButton.y = GET_Y_LPARAM(lParam);
				m_parent->PushEvent(event);

				break;
			}
			default:
				break;
		}
		//*/
	}
}

#endif