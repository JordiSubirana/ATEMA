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

#include <Atema/Shader/Ast/AstVisitor.hpp>
#include <Atema/Shader/Ast/Expression.hpp>

using namespace at;

Expression::Expression()
{
}

Expression::~Expression()
{
}

Expression::Type ConstantExpression::getType() const noexcept
{
	return Type::Constant;
}

void ConstantExpression::accept(AstVisitor& visitor)
{
	visitor.visit(*this);
}

Expression::Type VariableExpression::getType() const noexcept
{
	return Type::Variable;
}

void VariableExpression::accept(AstVisitor& visitor)
{
	visitor.visit(*this);
}

Expression::Type AccessIndexExpression::getType() const noexcept
{
	return Type::AccessIndex;
}

void AccessIndexExpression::accept(AstVisitor& visitor)
{
	visitor.visit(*this);
}

Expression::Type AccessIdentifierExpression::getType() const noexcept
{
	return Type::AccessIdentifier;
}

void AccessIdentifierExpression::accept(AstVisitor& visitor)
{
	visitor.visit(*this);
}

Expression::Type AssignmentExpression::getType() const noexcept
{
	return Type::Assignment;
}

void AssignmentExpression::accept(AstVisitor& visitor)
{
	visitor.visit(*this);
}

Expression::Type UnaryExpression::getType() const noexcept
{
	return Type::Unary;
}

void UnaryExpression::accept(AstVisitor& visitor)
{
	visitor.visit(*this);
}

Expression::Type BinaryExpression::getType() const noexcept
{
	return Type::Binary;
}

void BinaryExpression::accept(AstVisitor& visitor)
{
	visitor.visit(*this);
}

Expression::Type FunctionCallExpression::getType() const noexcept
{
	return Type::FunctionCall;
}

void FunctionCallExpression::accept(AstVisitor& visitor)
{
	visitor.visit(*this);
}

Expression::Type BuiltInFunctionCallExpression::getType() const noexcept
{
	return Type::BuiltInFunctionCall;
}

void BuiltInFunctionCallExpression::accept(AstVisitor& visitor)
{
	visitor.visit(*this);
}

Expression::Type CastExpression::getType() const noexcept
{
	return Type::Cast;
}

void CastExpression::accept(AstVisitor& visitor)
{
	visitor.visit(*this);
}

Expression::Type SwizzleExpression::getType() const noexcept
{
	return Type::Swizzle;
}

void SwizzleExpression::accept(AstVisitor& visitor)
{
	visitor.visit(*this);
}

Expression::Type TernaryExpression::getType() const noexcept
{
	return Type::Ternary;
}

void TernaryExpression::accept(AstVisitor& visitor)
{
	visitor.visit(*this);
}
