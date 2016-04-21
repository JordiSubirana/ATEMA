#ifndef ATEMA_CONFIGURATION_HEADER
#define ATEMA_CONFIGURATION_HEADER

#ifdef _WIN32

	#define ATEMA_SYSTEM_WINDOWS
	
	#define ATEMA_EXPORT __declspec(dllexport)
	#define ATEMA_IMPORT __declspec(dllimport)
	
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif

#elif __APPLE__

	#include "TargetConditionals.h"
	
	#if TARGET_OS_MAC
		#define ATEMA_SYSTEM_MACOS
		
		#define ATEMA_EXPORT
		#define ATEMA_IMPORT
	#else
		#define ATEMA_SYSTEM_UNKNOWN
		
		#define ATEMA_EXPORT __attribute__((visibility ("default")))
		#define ATEMA_IMPORT __attribute__((visibility ("default")))
		
		#error Apple operating system not supported by ATEMA
	#endif
	
#elif __unix__
	
	#if __linux__
		#define ATEMA_SYSTEM_LINUX
		
		#define ATEMA_EXPORT __attribute__((visibility ("default")))
		#define ATEMA_IMPORT __attribute__((visibility ("default")))
	#else
		#define ATEMA_SYSTEM_UNKNOWN
		
		#define ATEMA_EXPORT
		#define ATEMA_IMPORT
		
		#error Unix operating system not supported by ATEMA
	#endif
	
#else
	
	#define ATEMA_SYSTEM_UNKNOWN
	
	#define ATEMA_EXPORT
	#define ATEMA_IMPORT
	
	#error Operating system not supported by ATEMA

#endif

#endif