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

#ifndef ATEMA_MACROLIST_SHADERASTEXPRESSION
#error ATEMA_MACROLIST_SHADERASTEXPRESSION must be defined before including this file
#endif

ATEMA_MACROLIST_SHADERASTEXPRESSION(Constant)
ATEMA_MACROLIST_SHADERASTEXPRESSION(Variable)
ATEMA_MACROLIST_SHADERASTEXPRESSION(AccessIndex)
ATEMA_MACROLIST_SHADERASTEXPRESSION(AccessIdentifier)
ATEMA_MACROLIST_SHADERASTEXPRESSION(Assignment)
ATEMA_MACROLIST_SHADERASTEXPRESSION(Unary)
ATEMA_MACROLIST_SHADERASTEXPRESSION(Binary)
ATEMA_MACROLIST_SHADERASTEXPRESSION(FunctionCall)
ATEMA_MACROLIST_SHADERASTEXPRESSION(BuiltInFunctionCall)
ATEMA_MACROLIST_SHADERASTEXPRESSION(Cast)
ATEMA_MACROLIST_SHADERASTEXPRESSION(Swizzle)
ATEMA_MACROLIST_SHADERASTEXPRESSION(Ternary)

#undef ATEMA_MACROLIST_SHADERASTEXPRESSION