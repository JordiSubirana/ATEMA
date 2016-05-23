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

#ifndef ATEMA_WINDOW_HEADER
#define ATEMA_WINDOW_HEADER

#include <atema/window/config.hpp>
#include <atema/context/context.hpp>

#include <atema/core/flags.hpp>

#include <string>

#if defined(ATEMA_CONTEXT_IMPL_GLFW)
	
	#define ATEMA_WINDOW_IMPL_GLFW
	
#endif

namespace at
{
	class Mouse;
	class Keyboard;
	
	class ATEMA_WINDOW_API Window : public Context
	{
		public:
			enum class options : Flags
			{
				fullscreen	= 0x0001,
				visible		= 0x0002,
				autoscale	= 0x0004,
				resizable	= 0x0008,
				frame		= 0x0010,
				vsync		= 0x0020,
				adapt		= 0x0040
			};
			
			using Context::create;
			
		//To implement for each OS
		void set_title(std::string title);

	public:
			Window();
			virtual ~Window() noexcept;
			
			void create(unsigned int w, unsigned int h, const char *name, Flags flag_list, const Context::gl_version& version);
			void create(int x, int y, unsigned int w, unsigned int h, const char *name, Flags flag_list, const Context::gl_version& version);
			
			void set_position(int x, int y);
			
			operator bool() const noexcept; //true if open, false otherwise
			
			void update();
			
		private:
			friend class at::Mouse;
			friend class at::Keyboard;
			
		//OS specific
		#if defined(ATEMA_WINDOW_IMPL_GLFW)
		private:
			GLFWwindow* get_glfw_window() const noexcept;
		#endif
	};
}

ATEMA_ENUM_FLAGS(at::Window::options)

#endif