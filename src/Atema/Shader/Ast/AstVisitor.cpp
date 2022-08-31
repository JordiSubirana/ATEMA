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

#include <Atema/Shader/Ast/AstVisitor.hpp>

using namespace at;

// AstVisitor
AstVisitor::AstVisitor()
{
}

AstVisitor::~AstVisitor()
{
}

void AstVisitor::visit(Statement& statement)
{
	switch (statement.getType())
	{
#define ATEMA_MACROLIST_SHADERASTSTATEMENT(at_statement) case Statement::Type::at_statement: \
	{ \
		visit(static_cast<at_statement ## Statement&>(statement)); \
		break; \
	}
#include <Atema/Shader/Ast/StatementMacroList.hpp>
		default:
		{
			ATEMA_ERROR("Invalid statement type");
		}
	}
}

void AstVisitor::visit(Expression& expression)
{
	switch (expression.getType())
	{
#define ATEMA_MACROLIST_SHADERASTEXPRESSION(at_expression) case Expression::Type::at_expression: \
	{ \
		visit(static_cast<at_expression ## Expression&>(expression)); \
		break; \
	}
#include <Atema/Shader/Ast/ExpressionMacroList.hpp>
		default:
		{
			ATEMA_ERROR("Invalid expression type");
		}
	}
}

#define ATEMA_MACROLIST_SHADERASTSTATEMENT(at_statement) void AstVisitor::visit(at_statement ## Statement& statement) {}
#include <Atema/Shader/Ast/StatementMacroList.hpp>

#define ATEMA_MACROLIST_SHADERASTEXPRESSION(at_expression) void AstVisitor::visit(at_expression ## Expression& expression) {}
#include <Atema/Shader/Ast/ExpressionMacroList.hpp>

// AstConstVisitor
AstConstVisitor::AstConstVisitor()
{
}

AstConstVisitor::~AstConstVisitor()
{
}

void AstConstVisitor::visit(const Statement& statement) const
{
	switch (statement.getType())
	{
#define ATEMA_MACROLIST_SHADERASTSTATEMENT(at_statement) case Statement::Type::at_statement: \
	{ \
		visit(static_cast<const at_statement ## Statement&>(statement)); \
		break; \
	}
#include <Atema/Shader/Ast/StatementMacroList.hpp>
		default:
		{
			ATEMA_ERROR("Invalid statement type");
		}
	}
}

void AstConstVisitor::visit(const Expression& expression) const
{
	switch (expression.getType())
	{
#define ATEMA_MACROLIST_SHADERASTEXPRESSION(at_expression) case Expression::Type::at_expression: \
	{ \
		visit(static_cast<const at_expression ## Expression&>(expression)); \
		break; \
	}
#include <Atema/Shader/Ast/ExpressionMacroList.hpp>
		default:
		{
			ATEMA_ERROR("Invalid expression type");
		}
	}
}

#define ATEMA_MACROLIST_SHADERASTSTATEMENT(at_statement) void AstConstVisitor::visit(const at_statement ## Statement& statement) {}
#include <Atema/Shader/Ast/StatementMacroList.hpp>

#define ATEMA_MACROLIST_SHADERASTEXPRESSION(at_expression) void AstConstVisitor::visit(const at_expression ## Expression& expression) {}
#include <Atema/Shader/Ast/ExpressionMacroList.hpp>
