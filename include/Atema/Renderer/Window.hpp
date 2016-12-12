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

#ifndef ATEMA_RENDERER_WINDOW_HEADER
#define ATEMA_RENDERER_WINDOW_HEADER

#include <Atema/Renderer/Config.hpp>
#include <Atema/Renderer/Context.hpp>
#include <Atema/Core/WindowHandle.hpp>

#include <string>

namespace at
{	
	class ATEMA_RENDERER_API Window : public RendererDependent, public NonCopyable
	{
		public:
			class Style
			{
				public:
					bool resizable;
					bool closable;
					bool frame;
					bool fullscreen;
					
				public:
					Style() : resizable(true), closable(true), frame(true), fullscreen(false) {}
					virtual ~Style() = default;
			};
			
			static Ref<Window> create(unsigned int w, unsigned int h, const std::string& name, const Window::Style& style = Window::Style(), const Context::Settings& settings = Context::Settings(), Renderer *renderer = nullptr);
			static Ref<Window> create(int x, int y, unsigned int w, unsigned int h, const std::string& name, const Window::Style& style = Window::Style(), const Context::Settings& settings = Context::Settings(), Renderer *renderer = nullptr);
			
		public:
			virtual ~Window();
			
			void reset(unsigned int w, unsigned int h, const std::string& name, const Window::Style& style = Window::Style());
			void reset(int x, int y, unsigned int w, unsigned int h, const std::string& name, const Window::Style& style = Window::Style());
			
			void set_position(int x, int y);
			void set_title(const std::string& title);
			
			void close();
			bool should_close() const noexcept;
			operator bool() const noexcept; //true if open, false otherwise
			
			void process_events();
			void update();
			
			WindowHandle get_handle();
			const WindowHandle get_handle() const;
			
			Ref<Context> get_context();
			
		private:
			Window();
			
			class Impl;
			
			Window::Impl *m_impl;
			Ref<Context> m_context;
	};
}

#endif