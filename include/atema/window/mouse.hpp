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

#ifndef ATEMA_MOUSE_HEADER
#define ATEMA_MOUSE_HEADER

#include <atema/window/config.hpp>
#include <atema/window/window.hpp>

#if defined(ATEMA_WINDOW_IMPL_GLFW)
	
	#define ATEMA_MOUSE_IMPL_GLFW
	
#endif

namespace at
{
	class ATEMA_WINDOW_API Mouse
	{
		public:
			enum class button
			{
				left = GLFW_MOUSE_BUTTON_LEFT,
				middle = GLFW_MOUSE_BUTTON_MIDDLE,
				right = GLFW_MOUSE_BUTTON_RIGHT
			};
			
		//To implement for each OS
		public:
			Mouse();
			Mouse(const Window& Window);
			~Mouse();
			
			void set_window(const Window& Window);
			
			bool is_pressed(button b) const;
			
			bool is_enabled() const;
			void disable();
			void enable();
			
			double get_x() const;
			double get_y() const;
			
		//OS specific
		#if defined(ATEMA_MOUSE_IMPL_GLFW)
		private:
			GLFWwindow *m_window;
			bool m_enabled;
			
		#endif
	};
}

#endif