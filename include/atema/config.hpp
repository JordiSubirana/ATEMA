#ifndef ATEMA_CONFIG_HEADER
#define ATEMA_CONFIG_HEADER

// Compiler
#if defined(__BORLANDC__)
	
	#define ATEMA_COMPILER_BORDLAND
	#define ATEMA_COMPILER_SUPPORTS_CPP11 (defined(__cplusplus) && __cplusplus >= 201103L)
	#define ATEMA_FUNCTION __FUNC__
	
#elif defined(__clang__)
	
	#define ATEMA_COMPILER_CLANG
	#define ATEMA_COMPILER_SUPPORTS_CPP11 (defined(__cplusplus) && __cplusplus >= 201103L)
	#define ATEMA_FUNCTION __PRETTY_FUNCTION__
	
#elif defined(__GNUC__)
	
	#define ATEMA_COMPILER_GCC
	#define ATEMA_COMPILER_SUPPORTS_CPP11 (defined(__cplusplus) && __cplusplus >= 201103L)
	#define ATEMA_FUNCTION __PRETTY_FUNCTION__
	
#elif defined(__MINGW32__)
	
	#define ATEMA_COMPILER_GCC
	#define ATEMA_COMPILER_MINGW
	#define ATEMA_COMPILER_SUPPORTS_CPP11 (defined(__cplusplus) && __cplusplus >= 201103L)
	#define ATEMA_FUNCTION __PRETTY_FUNCTION__
	
#elif defined(__INTEL_COMPILER) || defined(__ICL)
	
	#define ATEMA_COMPILER_INTEL
	#define ATEMA_COMPILER_SUPPORTS_CPP11 (defined(__cplusplus) && __cplusplus >= 201103L)
	#define ATEMA_FUNCTION __FUNCTION__
	
#elif defined(_MSC_VER)
	
	#define ATEMA_COMPILER_MSVC
	#define ATEMA_COMPILER_SUPPORTS_CPP11 (_MSC_VER >= 1900)
	#define ATEMA_FUNCTION __FUNCSIG__

	#pragma warning(disable: 4251)
#else
	
	#define ATEMA_COMPILER_UNKNOWN
	#define ATEMA_FUNCTION ""
	
	#error Unsupported compiler
	
#endif

#if !ATEMA_COMPILER_SUPPORTS_CPP11
	
	#error A c++11 compatible compiler is required for ATEMA
	
#endif


// Operating System
#ifdef _WIN32

	#define ATEMA_SYSTEM_WINDOWS
	
	#define ATEMA_EXPORT __declspec(dllexport)
	#define ATEMA_IMPORT __declspec(dllimport)
	// #define ATEMA_EXPORT
	// #define ATEMA_IMPORT
	
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