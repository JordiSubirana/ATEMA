#ifndef ATEMA_CONTEXT_HEADER
#define ATEMA_CONTEXT_HEADER

#include <atema/config.hpp>

#if defined(ATEMA_SYSTEM_WINDOWS)
	
	#define ATEMA_CONTEXT_IMPL_GLFW
	
#elif defined(ATEMA_SYSTEM_LINUX)
	
	#define ATEMA_CONTEXT_IMPL_GLFW
	
#elif defined(ATEMA_SYSTEM_MACOS)
	
	#define ATEMA_CONTEXT_IMPL_GLFW

#endif

#include <atema/context/config.hpp>
#include <atema/utility/non_copyable.hpp>

#if defined(ATEMA_CONTEXT_IMPL_GLFW)
	#include <glad/glad.h>
	#include <GLFW/glfw3.h>
	
	#include <atema/utility/flags.hpp>
#endif

namespace at
{
	class window;
	
	class ATEMA_CONTEXT_API context : public non_copyable
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
			static void set_current(context *ptr) noexcept;
			void check_activity() const noexcept;
			
		//To implement for each OS
		public:
			context();
			virtual ~context() noexcept;
			
			virtual bool is_valid() const noexcept;
			
			virtual void create(const gl_version& version);
			
			//Must call check_activity() in the top of this method : it throws if activated in another thread
			//Then activate and set_current with this (true) or nullptr (false)
			virtual void activate(bool value);
			
		private:
			friend class at::window;
			
			//Init to false in the constructor, and don't care after
			bool m_thread_active;
			
			//OS specific
			#if defined(ATEMA_CONTEXT_IMPL_GLFW)
				void create(unsigned int w, unsigned int h, const char *name, flags flag_list, const gl_version& version);
				void destroy_window() noexcept;
				
				using rect = struct
				{
					int x;
					int y;
					int w;
					int h;
				};
				
				bool m_valid;
				bool m_active;
				
				flags m_flags;
				rect m_viewport;
				rect m_infos;
				std::string m_name;
				
				GLFWwindow *m_window;
			#endif
	};
}

#endif