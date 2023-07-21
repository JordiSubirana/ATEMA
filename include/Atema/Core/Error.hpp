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

#ifndef ATEMA_CORE_ERROR_HPP
#define ATEMA_CORE_ERROR_HPP

#include <Atema/Core/Config.hpp>

#include <stdexcept>

#define ATEMA_EXCEPTION(atDescription) throw std::runtime_error(atDescription)

#define ATEMA_MESSAGE(atMsgType, atDescription) (std::string(ATEMA_FUNCTION) + "\n" + (atMsgType) + " : " + (atDescription)).c_str()

#define ATEMA_ERROR(atDescription) ATEMA_EXCEPTION(ATEMA_MESSAGE("Error", atDescription))

#ifdef ATEMA_DISABLE_ASSERT
#define ATEMA_ASSERT(atCondition, atDescription) (void)(atCondition)
#else
#define ATEMA_ASSERT(atCondition, atDescription) do { if (!(atCondition)) { ATEMA_EXCEPTION(ATEMA_MESSAGE("Assertion failed", atDescription)); } } while (false)
#endif

#endif