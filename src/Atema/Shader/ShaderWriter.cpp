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

#include <Atema/Shader/ShaderWriter.hpp>

using namespace at;

ShaderWriter::ShaderWriter()
{
}

ShaderWriter::~ShaderWriter()
{
}

void ShaderWriter::write(const UPtr<Statement>& statement)
{
	ATEMA_ASSERT(statement, "Invalid statement");

	write(*statement);
}

void ShaderWriter::write(const Statement& statement)
{
	switch (statement.getType())
	{

#define ATEMA_MACROLIST_SHADERASTSTATEMENT(at_statement) case Statement::Type::at_statement: \
	{ \
		write(static_cast<const at_statement ## Statement&>(statement)); \
		break; \
	}
#include <Atema/Shader/Ast/StatementMacroList.hpp>

		default:
		{
			ATEMA_ERROR("Invalid statement type");
		}
	}
}

void ShaderWriter::write(const UPtr<Expression>& expression)
{
	ATEMA_ASSERT(expression, "Invalid expression");

	write(*expression);
}

void ShaderWriter::write(const Expression& expression)
{
	switch (expression.getType())
	{

#define ATEMA_MACROLIST_SHADERASTEXPRESSION(at_expression) case Expression::Type::at_expression: \
	{ \
		write(static_cast<const at_expression ## Expression&>(expression)); \
		break; \
	}
#include <Atema/Shader/Ast/ExpressionMacroList.hpp>

		default:
		{
			ATEMA_ERROR("Invalid expression type");
		}
	}
}
