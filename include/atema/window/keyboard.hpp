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

#ifndef ATEMA_KEYBOARD_HEADER
#define ATEMA_KEYBOARD_HEADER

#include <atema/window/config.hpp>
#include <atema/window/window.hpp>

#if defined(ATEMA_WINDOW_IMPL_GLFW)
	
	#define ATEMA_KEYBOARD_IMPL_GLFW
	
#endif

namespace at
{
	class ATEMA_WINDOW_API Keyboard
	{
		public:
			enum class key
			{
				unknown = GLFW_KEY_UNKNOWN,
				space = GLFW_KEY_SPACE,
				apostrophe = GLFW_KEY_APOSTROPHE,
				comma = GLFW_KEY_COMMA,
				minus = GLFW_KEY_MINUS,
				period = GLFW_KEY_PERIOD,
				slash = GLFW_KEY_SLASH,
				num_0 = GLFW_KEY_0,
				num_1 = GLFW_KEY_1,
				num_2 = GLFW_KEY_2,
				num_3 = GLFW_KEY_3,
				num_4 = GLFW_KEY_4,
				num_5 = GLFW_KEY_5,
				num_6 = GLFW_KEY_6,
				num_7 = GLFW_KEY_7,
				num_8 = GLFW_KEY_8,
				num_9 = GLFW_KEY_9,
				semicolon = GLFW_KEY_SEMICOLON,
				equal = GLFW_KEY_EQUAL,
				a = GLFW_KEY_A,
				b = GLFW_KEY_B,
				c = GLFW_KEY_C,
				d = GLFW_KEY_D,
				e = GLFW_KEY_E,
				f = GLFW_KEY_F,
				g = GLFW_KEY_G,
				h = GLFW_KEY_H,
				i = GLFW_KEY_I,
				j = GLFW_KEY_J,
				k = GLFW_KEY_K,
				l = GLFW_KEY_L,
				m = GLFW_KEY_M,
				n = GLFW_KEY_N,
				o = GLFW_KEY_O,
				p = GLFW_KEY_P,
				q = GLFW_KEY_Q,
				r = GLFW_KEY_R,
				s = GLFW_KEY_S,
				t = GLFW_KEY_T,
				u = GLFW_KEY_U,
				v = GLFW_KEY_V,
				w = GLFW_KEY_W,
				x = GLFW_KEY_X,
				y = GLFW_KEY_Y,
				z = GLFW_KEY_Z,
				bracket_left = GLFW_KEY_LEFT_BRACKET,
				backslash = GLFW_KEY_BACKSLASH,
				bracket_right = GLFW_KEY_RIGHT_BRACKET,
				grave_accent = GLFW_KEY_GRAVE_ACCENT,
				world_1 = GLFW_KEY_WORLD_1,
				world_2 = GLFW_KEY_WORLD_2,
				escape = GLFW_KEY_ESCAPE,
				enter = GLFW_KEY_ENTER,
				tab = GLFW_KEY_TAB,
				backspace = GLFW_KEY_BACKSPACE,
				insert = GLFW_KEY_INSERT,
				del = GLFW_KEY_DELETE,
				right = GLFW_KEY_RIGHT,
				left = GLFW_KEY_LEFT,
				down = GLFW_KEY_DOWN,
				up = GLFW_KEY_UP,
				page_up = GLFW_KEY_PAGE_UP,
				page_down = GLFW_KEY_PAGE_DOWN,
				home = GLFW_KEY_HOME,
				end = GLFW_KEY_END,
				caps_lock = GLFW_KEY_CAPS_LOCK,
				scroll_lock = GLFW_KEY_SCROLL_LOCK,
				num_lock = GLFW_KEY_NUM_LOCK,
				print_screen = GLFW_KEY_PRINT_SCREEN,
				pause = GLFW_KEY_PAUSE,
				f1 = GLFW_KEY_F1,
				f2 = GLFW_KEY_F2,
				f3 = GLFW_KEY_F3,
				f4 = GLFW_KEY_F4,
				f5 = GLFW_KEY_F5,
				f6 = GLFW_KEY_F6,
				f7 = GLFW_KEY_F7,
				f8 = GLFW_KEY_F8,
				f9 = GLFW_KEY_F9,
				f10 = GLFW_KEY_F10,
				f11 = GLFW_KEY_F11,
				f12 = GLFW_KEY_F12,
				f13 = GLFW_KEY_F13,
				f14 = GLFW_KEY_F14,
				f15 = GLFW_KEY_F15,
				f16 = GLFW_KEY_F16,
				f17 = GLFW_KEY_F17,
				f18 = GLFW_KEY_F18,
				f19 = GLFW_KEY_F19,
				f20 = GLFW_KEY_F20,
				f21 = GLFW_KEY_F21,
				f22 = GLFW_KEY_F22,
				f23 = GLFW_KEY_F23,
				f24 = GLFW_KEY_F24,
				f25 = GLFW_KEY_F25,
				kp_0 = GLFW_KEY_KP_0,
				kp_1 = GLFW_KEY_KP_1,
				kp_2 = GLFW_KEY_KP_2,
				kp_3 = GLFW_KEY_KP_3,
				kp_4 = GLFW_KEY_KP_4,
				kp_5 = GLFW_KEY_KP_5,
				kp_6 = GLFW_KEY_KP_6,
				kp_7 = GLFW_KEY_KP_7,
				kp_8 = GLFW_KEY_KP_8,
				kp_9 = GLFW_KEY_KP_9,
				kp_decimal = GLFW_KEY_KP_DECIMAL,
				kp_divide = GLFW_KEY_KP_DIVIDE,
				kp_multiply = GLFW_KEY_KP_MULTIPLY,
				kp_subtract = GLFW_KEY_KP_SUBTRACT,
				kp_add = GLFW_KEY_KP_ADD,
				kp_enter = GLFW_KEY_KP_ENTER,
				kp_equal = GLFW_KEY_KP_EQUAL,
				shift_left = GLFW_KEY_LEFT_SHIFT,
				control_left = GLFW_KEY_LEFT_CONTROL,
				alt_left = GLFW_KEY_LEFT_ALT,
				super_left = GLFW_KEY_LEFT_SUPER,
				shift_right = GLFW_KEY_RIGHT_SHIFT,
				control_right = GLFW_KEY_RIGHT_CONTROL,
				alt_right = GLFW_KEY_RIGHT_ALT,
				super_right = GLFW_KEY_RIGHT_SUPER,
				menu = GLFW_KEY_MENU,
				last = GLFW_KEY_LAST
			};
		
		//To implement for each OS
		public:
			Keyboard();
			Keyboard(const Window& window);
			~Keyboard();
			
			void set_window(const Window& window);
			
			bool is_pressed(key k) const;
			
		//OS specific
		#if defined(ATEMA_KEYBOARD_IMPL_GLFW)
		private:
			GLFWwindow *m_window;
		#endif
	};
}

#endif