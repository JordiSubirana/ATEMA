/*
	Copyright 2021 Jordi SUBIRANA

	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
	Software, and to permit persons to whom the Software is furnished to do so, subject
	to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
	PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
	CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
	OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef ATEMA_CONFIG_HPP
#define ATEMA_CONFIG_HPP

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
	
	#error Unsupported compiler
	
#endif

#if !ATEMA_COMPILER_SUPPORTS_CPP11
	
	#error A c++11 compatible compiler is required for ATEMA
	
#endif


// Operating System
#if defined(_WIN32)

	#define ATEMA_SYSTEM_WINDOWS
	
	#define ATEMA_EXPORT __declspec(dllexport)
	#define ATEMA_IMPORT __declspec(dllimport)
	
	#ifndef NOMINMAX
		
		#define NOMINMAX
		
	#endif

#elif defined(__APPLE__) && defined(__MACH__)

	#include "TargetConditionals.h"
	
	#if TARGET_OS_MAC
		
		#define ATEMA_SYSTEM_MACOS
		
		#define ATEMA_EXPORT
		#define ATEMA_IMPORT
		
	#else
		
		#define ATEMA_SYSTEM_UNKNOWN
		
		#define ATEMA_EXPORT
		#define ATEMA_IMPORT
		
		#error Apple operating system not supported by ATEMA
		
	#endif
	
#elif defined(__unix__)
	
	#if defined(__linux__)
		
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