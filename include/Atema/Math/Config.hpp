/*
	Copyright 2022 Jordi SUBIRANA

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

#ifndef ATEMA_MATH_CONFIG_HPP
#define ATEMA_MATH_CONFIG_HPP

#include <Atema/Config.hpp>

#if defined(ATEMA_STATIC)
	
	#define ATEMA_MATH_API
	
#else
	
	#if defined(ATEMA_MATH_EXPORT)
		
		#define ATEMA_MATH_API ATEMA_EXPORT
		
	#else
		
		#define ATEMA_MATH_API ATEMA_IMPORT
		
	#endif

#endif

// Clip space definition : by default clip space's Z is between [0,1]
#define ATEMA_CLIPSPACE_Z_ZERO_TO_ONE 1
#define ATEMA_CLIPSPACE_Z_MINUS_ONE_TO_ONE 2

#ifndef ATEMA_CLIPSPACE_Z
#define ATEMA_CLIPSPACE_Z ATEMA_CLIPSPACE_Z_ZERO_TO_ONE
#endif

#if ATEMA_CLIPSPACE_Z != ATEMA_CLIPSPACE_Z_ZERO_TO_ONE && ATEMA_CLIPSPACE_Z != ATEMA_CLIPSPACE_Z_MINUS_ONE_TO_ONE
#error Invalid clipspace Z range
#endif

#endif