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

#ifndef ATEMA_OPENGL_CONTEXT_HEADER
#define ATEMA_OPENGL_CONTEXT_HEADER

#include <Atema/OpenGL/Config.hpp>
#include <Atema/Renderer/Context.hpp>

namespace at
{	
	class OpenGLRenderer;
	
	class ATEMA_OPENGL_API OpenGLContext : public Context
	{
		friend class OpenGLRenderer;
		
		public:
			~OpenGLContext();
			
			void reset(unsigned int w, unsigned int h, const Context::Settings& settings = Context::Settings());
			void reset(WindowHandle handle, const Context::Settings& settings = Context::Settings());
			
			void set_current(bool current = true);
			
			void swap_buffers();
			
		private:
			OpenGLContext();
			
			class Impl;
			OpenGLContext::Impl *m_impl;
	};
}

#endif