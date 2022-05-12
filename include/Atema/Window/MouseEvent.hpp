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

#ifndef ATEMA_WINDOW_MOUSEEVENT_HPP
#define ATEMA_WINDOW_MOUSEEVENT_HPP

#include <Atema/Window/Config.hpp>
#include <Atema/Core/Event.hpp>
#include <Atema/Math/Vector.hpp>
#include <Atema/Window/Enums.hpp>

namespace at
{
	class ATEMA_WINDOW_API MouseButtonEvent : public Event
	{
	public:
		MouseButtonEvent();
		MouseButtonEvent(MouseButton button, MouseButtonState state, Flags<KeyModifier> modifiers = 0, Vector2f position = Vector2f(0.0f, 0.0f));
		virtual ~MouseButtonEvent();

		MouseButton button;
		MouseButtonState state;
		Flags<KeyModifier> modifiers;
		Vector2f position;
	};

	class ATEMA_WINDOW_API MouseMoveEvent : public Event
	{
	public:
		MouseMoveEvent();
		MouseMoveEvent(Vector2f position);
		virtual ~MouseMoveEvent();

		Vector2f position;
	};
}

#endif
