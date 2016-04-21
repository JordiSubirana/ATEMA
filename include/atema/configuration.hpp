#ifndef ATEMA_CONFIGURATION_HEADER
#define ATEMA_CONFIGURATION_HEADER

#ifdef _WIN32

	#define ATEMA_SYSTEM_WINDOWS

#elif __APPLE__

	#include "TargetConditionals.h"
	
	#if TARGET_OS_MAC
		#define ATEMA_SYSTEM_MACOS
	#endif
	
#elif __unix__
	
	#if __linux__
		#define ATEMA_SYSTEM_LINUX
	#endif
	
#else
	
	#error "Operating system unsupported by ATEMA"

#endif

#endif