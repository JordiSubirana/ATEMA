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

#ifndef ATEMA_WINDOW_ENUMS_HPP
#define ATEMA_WINDOW_ENUMS_HPP

#include <Atema/Core/Flags.hpp>

namespace at
{
	enum class Key
	{
		Unknown,
		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,
		Key0,
		Key1,
		Key2,
		Key3,
		Key4,
		Key5,
		Key6,
		Key7,
		Key8,
		Key9,
		KeyPad0,
		KeyPad1,
		KeyPad2,
		KeyPad3,
		KeyPad4,
		KeyPad5,
		KeyPad6,
		KeyPad7,
		KeyPad8,
		KeyPad9,
		KeyPadDecimal,
		KeyPadDivide,
		KeyPadMultiply,
		KeyPadSubtract,
		KeyPadAdd,
		KeyPadEnter,
		KeyPadEqual,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		F13,
		F14,
		F15,
		F16,
		F17,
		F18,
		F19,
		F20,
		F21,
		F22,
		F23,
		F24,
		F25,
		Right,
		Left,
		Down,
		Up,
		Space,
		Escape,
		Enter,
		Tab,
		Backspace,
		Insert,
		Delete,
		ShiftLeft,
		ShiftRight,
		ControlLeft,
		ControlRight,
		AltLeft,
		AltRight,
		SuperLeft,
		Apostrophe,
		Comma,
		Minus,
		Period,
		Slash,
		Semicolon,
		Equal,
		LeftBracket,
		Backslash,
		RightBracket,
		GraveAccent,
		World1,
		World2,
		PageUp,
		PageDown,
		Home,
		End,
		CapsLock,
		ScrollLock,
		NumLock,
		PrintScreen,
		Pause,
		SuperRight,
		Menu
	};

	enum class KeyModifier
	{
		Shift = 0x0001,
		Control = 0x0002,
		Alt = 0x0004,
		Super = 0x0008,
		CapsLock = 0x0010,
		NumLock = 0x0020
	};

	ATEMA_DECLARE_FLAGS(KeyModifier);

	enum class KeyState
	{
		Press,
		Repeat,
		Release
	};

	enum class MouseButton
	{
		Left,
		Middle,
		Right,
		Button1,
		Button2,
		Button3,
		Button4,
		Button5
	};

	enum class MouseButtonState
	{
		Press,
		Release
	};
}

#endif
