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

#ifndef ATEMA_CONTEXT_HEADER
#define ATEMA_CONTEXT_HEADER

#include <atema/config.hpp>


#if defined(ATEMA_SYSTEM_WINDOWS)
#		define ATEMA_CONTEXT_IMPL_GLFW

#elif defined(ATEMA_SYSTEM_LINUX)
#		define ATEMA_CONTEXT_IMPL_GLFW

#elif defined(ATEMA_SYSTEM_MACOS)
#		define ATEMA_CONTEXT_IMPL_GLFW

#endif

#include <atema/context/config.hpp>
#include <atema/utility/non_copyable.hpp>
#include <atema/context/render_target.hpp>


#if defined(ATEMA_CONTEXT_IMPL_GLFW)
#		include <glad/glad.h>
#		include <GLFW/glfw3.h>

#		if defined(ATEMA_SYSTEM_WINDOWS)
#				define GLFW_EXPOSE_NATIVE_WGL // context API
#				define GLFW_EXPOSE_NATIVE_WIN32 // window API
#				include <GLFW/glfw3native.h>

#		elif defined(ATEMA_SYSTEM_LINUX)
				struct __GLXcontextRec;
				typedef struct __GLXcontextRec *GLXContext;

#		elif defined(ATEMA_SYSTEM_MACOS)
#				define GLFW_EXPOSE_NATIVE_NSGL // context API
#				define GLFW_EXPOSE_NATIVE_COCOA // window API
#				include <GLFW/glfw3native.h>

#		endif

#		include <string>
#		include <atema/utility/flags.hpp>

#endif




namespace at
{
	class Window;
	
	class ATEMA_CONTEXT_API Context : public NonCopyable, public RenderTarget
	{
		public:
			using gl_version = struct
			{
				int major;
				int minor;
			};
		
		//Already implemented
		public:
			bool is_current_context() const noexcept;
			
		private:
			static void set_current(Context *ptr) noexcept;
			void check_activity() const noexcept;
			void init_gl_states() const noexcept;
			
		//To implement for each OS
		public:
			Context();
			virtual ~Context() noexcept;
			
			virtual bool is_valid() const noexcept;
			
			//Must call init_gl_states() after creating Context
			//w & h refer to the size of the internal framebuffer
			virtual void create(unsigned int w, unsigned int h, const gl_version& version);
			
			//Must call check_activity() in the top of this method : it throws if activated in another thread
			//Then activate and set_current with this (true) or nullptr (false)
			virtual void make_current(bool value);
			
			//Default framebuffer dimensions
			virtual unsigned int get_width() const;
			virtual unsigned int get_height() const;
			virtual GLuint get_gl_framebuffer_id() const;
			
		protected:
			virtual void ensure_framebuffer_exists();
			
		private:
			friend class at::Window;
			
			//Init to false in the constructor, and don't care after
			bool m_thread_active;
			
		//OS specific
		#if defined(ATEMA_CONTEXT_IMPL_GLFW)
		private:
			void create(unsigned int w, unsigned int h, const char *name, Flags flag_list, const gl_version& version);
			void destroy_window() noexcept;
			
			bool m_active;
			
			Flags m_flags;
			Rect m_infos;
			std::string m_name;
			
			GLFWwindow *m_window;
		#endif
		
		#if defined(ATEMA_SYSTEM_WINDOWS)
		public:
			static HGLRC get_current_os_context() noexcept;
		#elif defined(ATEMA_SYSTEM_LINUX)
		public:
			static GLXContext get_current_os_context() noexcept;
		#endif
	};
}

#endif