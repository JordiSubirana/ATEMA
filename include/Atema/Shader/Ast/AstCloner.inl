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

#ifndef ATEMA_SHADER_AST_ASTCLONER_INL
#define ATEMA_SHADER_AST_ASTCLONER_INL

#include <Atema/Shader/Ast/AstCloner.hpp>

#include <type_traits>

namespace at
{
	template <typename T>
	UPtr<T> AstCloner::clone(const UPtr<T>& astNode)
	{
		if (!astNode)
			return {};

		if constexpr (std::is_base_of<Statement, T>::value)
		{
			return clone(*astNode);
		}
		else if constexpr (std::is_base_of<Expression, T>::value)
		{
			return clone(*astNode);
		}
		else
		{
			static_assert(false, "Invalid type to clone : must be a derived from Statement or Expression");
		}

		return {};
	}
}

#endif