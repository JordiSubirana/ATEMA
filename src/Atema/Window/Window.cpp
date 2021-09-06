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

#include <Atema/Window/Window.hpp>
#include <Atema/Core/Error.hpp>
#include <Atema/Window/KeyEvent.hpp>
#include <Atema/Window/MouseEvent.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifdef ATEMA_SYSTEM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#include <functional>

using namespace at;

// Implementation
namespace
{
	unsigned s_windowCount = 0;

	Key getKey(int glfwKey)
	{
		switch (glfwKey)
		{
			case GLFW_KEY_UNKNOWN: return Key::Unknown;
			case GLFW_KEY_SPACE: return Key::Space;
			case GLFW_KEY_APOSTROPHE: return Key::Apostrophe;
			case GLFW_KEY_COMMA: return Key::Comma;
			case GLFW_KEY_MINUS: return Key::Minus;
			case GLFW_KEY_PERIOD : return Key::Period;
			case GLFW_KEY_SLASH : return Key::Slash;
			case GLFW_KEY_0: return Key::Key0;
			case GLFW_KEY_1: return Key::Key1;
			case GLFW_KEY_2: return Key::Key2;
			case GLFW_KEY_3: return Key::Key3;
			case GLFW_KEY_4: return Key::Key4;
			case GLFW_KEY_5: return Key::Key5;
			case GLFW_KEY_6: return Key::Key6;
			case GLFW_KEY_7: return Key::Key7;
			case GLFW_KEY_8: return Key::Key8;
			case GLFW_KEY_9: return Key::Key9;
			case GLFW_KEY_SEMICOLON: return Key::Semicolon;
			case GLFW_KEY_EQUAL: return Key::Equal;
			case GLFW_KEY_A: return Key::A;
			case GLFW_KEY_B: return Key::B;
			case GLFW_KEY_C: return Key::C;
			case GLFW_KEY_D: return Key::D;
			case GLFW_KEY_E: return Key::E;
			case GLFW_KEY_F: return Key::F;
			case GLFW_KEY_G: return Key::G;
			case GLFW_KEY_H: return Key::H;
			case GLFW_KEY_I: return Key::I;
			case GLFW_KEY_J: return Key::J;
			case GLFW_KEY_K: return Key::K;
			case GLFW_KEY_L: return Key::L;
			case GLFW_KEY_M: return Key::M;
			case GLFW_KEY_N: return Key::N;
			case GLFW_KEY_O: return Key::O;
			case GLFW_KEY_P: return Key::P;
			case GLFW_KEY_Q: return Key::Q;
			case GLFW_KEY_R: return Key::R;
			case GLFW_KEY_S: return Key::S;
			case GLFW_KEY_T: return Key::T;
			case GLFW_KEY_U: return Key::U;
			case GLFW_KEY_V: return Key::V;
			case GLFW_KEY_W: return Key::W;
			case GLFW_KEY_X: return Key::X;
			case GLFW_KEY_Y: return Key::Y;
			case GLFW_KEY_Z: return Key::Z;
			case GLFW_KEY_LEFT_BRACKET: return Key::LeftBracket;
			case GLFW_KEY_BACKSLASH: return Key::Backslash;
			case GLFW_KEY_RIGHT_BRACKET: return Key::RightBracket;
			case GLFW_KEY_GRAVE_ACCENT: return Key::GraveAccent;
			case GLFW_KEY_WORLD_1: return Key::World1;
			case GLFW_KEY_WORLD_2: return Key::World2;
			case GLFW_KEY_ESCAPE: return Key::Escape;
			case GLFW_KEY_ENTER: return Key::Enter;
			case GLFW_KEY_TAB: return Key::Tab;
			case GLFW_KEY_BACKSPACE: return Key::Backspace;
			case GLFW_KEY_INSERT: return Key::Insert;
			case GLFW_KEY_DELETE: return Key::Delete;
			case GLFW_KEY_RIGHT: return Key::Right;
			case GLFW_KEY_LEFT: return Key::Left;
			case GLFW_KEY_DOWN: return Key::Down;
			case GLFW_KEY_UP: return Key::Up;
			case GLFW_KEY_PAGE_UP: return Key::PageUp;
			case GLFW_KEY_PAGE_DOWN: return Key::PageDown;
			case GLFW_KEY_HOME: return Key::Home;
			case GLFW_KEY_END: return Key::End;
			case GLFW_KEY_CAPS_LOCK: return Key::CapsLock;
			case GLFW_KEY_SCROLL_LOCK: return Key::ScrollLock;
			case GLFW_KEY_NUM_LOCK: return Key::NumLock;
			case GLFW_KEY_PRINT_SCREEN: return Key::PrintScreen;
			case GLFW_KEY_PAUSE: return Key::Pause;
			case GLFW_KEY_F1: return Key::F1;
			case GLFW_KEY_F2: return Key::F2;
			case GLFW_KEY_F3: return Key::F3;
			case GLFW_KEY_F4: return Key::F4;
			case GLFW_KEY_F5: return Key::F5;
			case GLFW_KEY_F6: return Key::F6;
			case GLFW_KEY_F7: return Key::F7;
			case GLFW_KEY_F8: return Key::F8;
			case GLFW_KEY_F9: return Key::F9;
			case GLFW_KEY_F10: return Key::F10;
			case GLFW_KEY_F11: return Key::F11;
			case GLFW_KEY_F12: return Key::F12;
			case GLFW_KEY_F13: return Key::F13;
			case GLFW_KEY_F14: return Key::F14;
			case GLFW_KEY_F15: return Key::F15;
			case GLFW_KEY_F16: return Key::F16;
			case GLFW_KEY_F17: return Key::F17;
			case GLFW_KEY_F18: return Key::F18;
			case GLFW_KEY_F19: return Key::F19;
			case GLFW_KEY_F20: return Key::F20;
			case GLFW_KEY_F21: return Key::F21;
			case GLFW_KEY_F22: return Key::F22;
			case GLFW_KEY_F23: return Key::F23;
			case GLFW_KEY_F24: return Key::F24;
			case GLFW_KEY_F25: return Key::F25;
			case GLFW_KEY_KP_0: return Key::KeyPad0;
			case GLFW_KEY_KP_1: return Key::KeyPad1;
			case GLFW_KEY_KP_2: return Key::KeyPad2;
			case GLFW_KEY_KP_3: return Key::KeyPad3;
			case GLFW_KEY_KP_4: return Key::KeyPad4;
			case GLFW_KEY_KP_5: return Key::KeyPad5;
			case GLFW_KEY_KP_6: return Key::KeyPad6;
			case GLFW_KEY_KP_7: return Key::KeyPad7;
			case GLFW_KEY_KP_8: return Key::KeyPad8;
			case GLFW_KEY_KP_9: return Key::KeyPad9;
			case GLFW_KEY_KP_DECIMAL: return Key::KeyPadDecimal;
			case GLFW_KEY_KP_DIVIDE: return Key::KeyPadDivide;
			case GLFW_KEY_KP_MULTIPLY: return Key::KeyPadMultiply;
			case GLFW_KEY_KP_SUBTRACT: return Key::KeyPadSubtract;
			case GLFW_KEY_KP_ADD: return Key::KeyPadAdd;
			case GLFW_KEY_KP_ENTER: return Key::KeyPadEnter;
			case GLFW_KEY_KP_EQUAL: return Key::KeyPadEqual;
			case GLFW_KEY_LEFT_SHIFT: return Key::ShiftLeft;
			case GLFW_KEY_LEFT_CONTROL: return Key::ControlLeft;
			case GLFW_KEY_LEFT_ALT: return Key::AltLeft;
			case GLFW_KEY_LEFT_SUPER: return Key::SuperLeft;
			case GLFW_KEY_RIGHT_SHIFT: return Key::ShiftRight;
			case GLFW_KEY_RIGHT_CONTROL: return Key::ControlRight;
			case GLFW_KEY_RIGHT_ALT: return Key::AltRight;
			case GLFW_KEY_RIGHT_SUPER: return Key::SuperRight;
			case GLFW_KEY_MENU: return Key::Menu;
			default:
			{
				break;
			}
		}

		return Key::Unknown;
	}

	Flags<KeyModifier> getKeyModifier(int glfwModifiers)
	{
		Flags<KeyModifier> modifiers;

		if (glfwModifiers & GLFW_MOD_SHIFT)
			modifiers |= KeyModifier::Shift;

		if (glfwModifiers & GLFW_MOD_CONTROL)
			modifiers |= KeyModifier::Control;

		if (glfwModifiers & GLFW_MOD_ALT)
			modifiers |= KeyModifier::Alt;

		if (glfwModifiers & GLFW_MOD_SUPER)
			modifiers |= KeyModifier::Super;

		if (glfwModifiers & GLFW_MOD_CAPS_LOCK)
			modifiers |= KeyModifier::CapsLock;

		if (glfwModifiers & GLFW_MOD_NUM_LOCK)
			modifiers |= KeyModifier::NumLock;
		
		return modifiers;
	}

	KeyState getKeyState(int glfwState)
	{
		switch (glfwState)
		{
			case GLFW_PRESS: return KeyState::Press;
			case GLFW_REPEAT: return KeyState::Repeat;
			case GLFW_RELEASE: return KeyState::Release;
			default:
			{
				ATEMA_ERROR("Invalid key state");
			}
		}

		return KeyState::Press;
	}

	MouseButton getMouseButton(int glfwButton)
	{
		switch (glfwButton)
		{
			case GLFW_MOUSE_BUTTON_LEFT: return MouseButton::Left;
			case GLFW_MOUSE_BUTTON_RIGHT: return MouseButton::Right;
			case GLFW_MOUSE_BUTTON_MIDDLE: return MouseButton::Middle;
			case GLFW_MOUSE_BUTTON_4: return MouseButton::Button1;
			case GLFW_MOUSE_BUTTON_5: return MouseButton::Button2;
			case GLFW_MOUSE_BUTTON_6: return MouseButton::Button3;
			case GLFW_MOUSE_BUTTON_7: return MouseButton::Button4;
			case GLFW_MOUSE_BUTTON_8: return MouseButton::Button5;
			default:
			{
				ATEMA_ERROR("Invalid mouse button");
			}
		}

		return MouseButton::Left;
	}

	MouseButtonState getMouseButtonState(int glfwState)
	{
		switch (glfwState)
		{
		case GLFW_PRESS: return MouseButtonState::Press;
		case GLFW_RELEASE: return MouseButtonState::Release;
		default:
		{
			ATEMA_ERROR("Invalid mouse button state");
		}
		}

		return MouseButtonState::Press;
	}
}

// Window Implementation
class Window::Implementation
{
public:
	Implementation() : m_window(nullptr), m_isCursorEnabled(true)
	{
		if (s_windowCount == 0)
		{
			if (!glfwInit())
			{
				ATEMA_ERROR("GLFW initialization failed");
			}
		}

		s_windowCount++;
	}
	
	virtual ~Implementation()
	{
		destroy(); // Ensure the window was deleted

		s_windowCount--;

		if (s_windowCount == 0)
			glfwTerminate();
	}

	void setResizedCallback(const std::function<void(unsigned, unsigned)>& callback)
	{
		m_resizedCallback = callback;
	}

	void seteventCallback(const std::function<void(Event&)>& callback)
	{
		m_eventCallback = callback;
	}

	void create(const Window::Settings& description)
	{
		destroy(); // Ensure there is no previous window

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, description.resizable ? GLFW_TRUE : GLFW_FALSE);
		
		m_window = glfwCreateWindow(
			description.width,
			description.height,
			description.title.c_str(),
			nullptr,
			nullptr);

		if (!m_window)
		{
			ATEMA_ERROR("Window creation failed");
		}

		glfwSetWindowUserPointer(m_window, this);

		glfwSetFramebufferSizeCallback(m_window, onFramebufferResized);
		glfwSetKeyCallback(m_window, onKeyEvent);
		glfwSetCursorPosCallback(m_window, onMouseMoveEvent);

		int w, h;
		glfwGetWindowSize(m_window, &w, &h);

		m_resizedCallback(static_cast<unsigned>(w), static_cast<unsigned>(h));
	}

	void destroy()
	{
		if (m_window)
		{
			glfwDestroyWindow(m_window);

			m_window = nullptr;
		}
	}

	void* getHandle() const
	{
#ifdef ATEMA_SYSTEM_WINDOWS
		HWND handle = glfwGetWin32Window(m_window);
		
		return handle;
#endif

		return nullptr;
	}

	void setTitle(const std::string& title)
	{
		glfwSetWindowTitle(m_window, title.c_str());
	}

	void setCursorEnabled(bool enable)
	{
		m_isCursorEnabled = enable;

		const auto mode = enable ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;

		glfwSetInputMode(m_window, GLFW_CURSOR, mode);
	}

	bool isCursorEnabled() const noexcept
	{
		return m_isCursorEnabled;
	}

	bool shouldClose() const noexcept
	{
		return glfwWindowShouldClose(m_window);
	}

	void processEvents()
	{
		glfwPollEvents();
	}

	void swapBuffers()
	{
		glfwSwapBuffers(m_window);
	}

	void framebufferResized(int width, int height)
	{
		m_resizedCallback(static_cast<unsigned>(width), static_cast<unsigned>(height));
	}

	void keyEvent(int key, int scancode, int action, int mods)
	{
		KeyEvent event;

		event.key = getKey(key);
		event.state = getKeyState(action);
		event.modifiers = getKeyModifier(mods);

		m_eventCallback(event);
	}

	void mouseMoveEvent(double x, double y)
	{
		MouseMoveEvent event;

		event.position.x = static_cast<float>(x);
		event.position.y = static_cast<float>(y);

		m_eventCallback(event);
	}

	void mouseButtonEvent(int button, int action, int mods)
	{
		double x, y;
		glfwGetCursorPos(m_window, &x, &y);

		MouseButtonEvent event;

		event.button = getMouseButton(button);
		event.state = getMouseButtonState(action);
		event.modifiers = getKeyModifier(mods);
		event.position.x = static_cast<float>(x);
		event.position.y = static_cast<float>(y);
		
		m_eventCallback(event);
	}

	static void onFramebufferResized(GLFWwindow* window, int width, int height)
	{
		auto w = static_cast<Window::Implementation*>(glfwGetWindowUserPointer(window));

		w->framebufferResized(width, height);
	}

	static void onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		auto w = static_cast<Window::Implementation*>(glfwGetWindowUserPointer(window));

		w->keyEvent(key, scancode, action, mods);
	}

	static void onMouseMoveEvent(GLFWwindow* window, double x, double y)
	{
		auto w = static_cast<Window::Implementation*>(glfwGetWindowUserPointer(window));

		w->mouseMoveEvent(x, y);
	}

	static void onMouseButtonEvent(GLFWwindow* window, int button, int action, int mods)
	{
		auto w = static_cast<Window::Implementation*>(glfwGetWindowUserPointer(window));

		w->mouseButtonEvent(button, action, mods);
	}

private:
	GLFWwindow* m_window;
	std::function<void(unsigned, unsigned)> m_resizedCallback;
	std::function<void(Event&)> m_eventCallback;
	bool m_isCursorEnabled;
};

// Window
Window::Window()
{
	m_implementation.reset(new Window::Implementation());
	m_implementation->setResizedCallback([this](unsigned int w, unsigned int h)
		{
			resizedCallback(w, h);
		});

	m_implementation->seteventCallback([this](Event& event)
		{
			m_eventDispatcher.execute(event);
		});
}

Window::~Window()
{
}

Ptr<Window> Window::create(const Window::Settings& description)
{
	Ptr<Window> window(new Window());

	window->initialize(description);
	
	return window;
}

void Window::setTitle(const std::string& title)
{
	m_implementation->setTitle(title);
}

void Window::setCursorEnabled(bool enable)
{
	m_implementation->setCursorEnabled(enable);
}

bool Window::isCursorEnabled() const noexcept
{
	return m_implementation->isCursorEnabled();
}

void Window::initialize(const Settings& description)
{
	m_implementation->create(description);
}

void Window::resizedCallback(unsigned width, unsigned height)
{
	m_size.x = width;
	m_size.y = height;
}

bool Window::shouldClose() const noexcept
{
	return m_implementation->shouldClose();
}

void Window::processEvents()
{
	m_implementation->processEvents();
}

void Window::swapBuffers()
{
	m_implementation->swapBuffers();
}

Vector2u Window::getSize() const noexcept
{
	return m_size;
}

EventDispatcher& Window::getEventDispatcher() noexcept
{
	return m_eventDispatcher;
}

const EventDispatcher& Window::getEventDispatcher() const noexcept
{
	return m_eventDispatcher;
}

void* Window::getHandle() const
{
	return m_implementation->getHandle();
}

const std::vector<const char*>& Window::getVulkanExtensions()
{
	static std::vector<const char*> requiredExtensions;

	if (requiredExtensions.empty())
	{
		uint32_t requiredExtensionCount = 0;
		auto requiredExtensionNames = glfwGetRequiredInstanceExtensions(&requiredExtensionCount);

		for (uint32_t i = 0; i < requiredExtensionCount; i++)
			requiredExtensions.push_back(requiredExtensionNames[i]);
	}

	return requiredExtensions;
}
