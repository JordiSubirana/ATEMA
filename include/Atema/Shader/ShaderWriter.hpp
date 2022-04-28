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

#ifndef ATEMA_SHADER_SHADERWRITER_HPP
#define ATEMA_SHADER_SHADERWRITER_HPP

#include <Atema/Shader/Config.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Shader/Ast/Expression.hpp>
#include <Atema/Shader/Ast/Statement.hpp>


namespace at
{
	class ATEMA_SHADER_API ShaderWriter
	{
	public:
		ShaderWriter();
		virtual ~ShaderWriter();

		void write(const UPtr<Statement>& statement);
		void write(const Statement& statement);

		void write(const UPtr<Expression>& expression);
		void write(const Expression& expression);

#define ATEMA_MACROLIST_SHADERASTSTATEMENT(at_statement) virtual void write(const at_statement ## Statement& statement) = 0;
#include <Atema/Shader/Ast/StatementMacroList.hpp>

#define ATEMA_MACROLIST_SHADERASTEXPRESSION(at_expression) virtual void write(const at_expression ## Expression& expression) = 0;
#include <Atema/Shader/Ast/ExpressionMacroList.hpp>
	};
}

#endif