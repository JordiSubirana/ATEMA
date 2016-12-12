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

#include <Atema/Misc.hpp>

#endif