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
	
	class ATEMA_CONTEXT_API Context : public NonCopyable
	{
		public:
			using gl_version = struct
			{
				int major;
				int minor;
			};
		
		//Already implemented
		public:
			bool is_active() const noexcept;
			
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
			virtual void create(const gl_version& version);
			
			//Must call check_activity() in the top of this method : it throws if activated in another thread
			//Then activate and set_current with this (true) or nullptr (false)
			virtual void activate(bool value);
			
		private:
			friend class at::Window;
			
			//Init to false in the constructor, and don't care after
			bool m_thread_active;
			
		//OS specific
		#if defined(ATEMA_CONTEXT_IMPL_GLFW)
		private:
			void create(unsigned int w, unsigned int h, const char *name, Flags flag_list, const gl_version& version);
			void destroy_window() noexcept;
			
			using rect = struct
			{
				int x;
				int y;
				int w;
				int h;
			};
			
			bool m_active;
			
			Flags m_flags;
			rect m_viewport;
			rect m_infos;
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