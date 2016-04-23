#ifndef ATEMA_WINDOW_INTERNAL_CONFIG_HEADER
#define ATEMA_WINDOW_INTERNAL_CONFIG_HEADER

#include <atema/config.hpp>

#if defined(ATEMA_SYSTEM_WINDOWS)
	
	#define ATEMA_WINDOW_IMPL_GLFW
	
#elif defined(ATEMA_SYSTEM_LINUX)
	
	#define ATEMA_WINDOW_IMPL_GLFW
	
#elif defined(ATEMA_SYSTEM_MACOS)
	
	#define ATEMA_WINDOW_IMPL_GLFW

#endif

#endif