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

#include <Atema/Window/MouseEvent.hpp>

using namespace at;

// MouseButtonEvent
MouseButtonEvent::MouseButtonEvent() :
	Event(TypeInfo<MouseButtonEvent>::id),
	button(MouseButton::Left),
	state(MouseButtonState::Press),
	modifiers(0)
{
}

MouseButtonEvent::MouseButtonEvent(MouseButton button, MouseButtonState state, Flags<KeyModifier> modifiers, Vector2f position) :
	Event(TypeInfo<MouseButtonEvent>::id),
	button(button),
	state(state),
	modifiers(modifiers),
	position(position)
{
}

MouseButtonEvent::~MouseButtonEvent()
{
}

// MouseMoveEvent
MouseMoveEvent::MouseMoveEvent() :
	Event(TypeInfo<MouseMoveEvent>::id)	
{
}

MouseMoveEvent::MouseMoveEvent(Vector2f position) :
	Event(TypeInfo<MouseMoveEvent>::id),
	position(position)
{
}

MouseMoveEvent::~MouseMoveEvent()
{
}
