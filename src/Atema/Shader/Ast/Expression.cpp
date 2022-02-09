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

Expression::Type VariableExpression::getType() const noexcept
{
	return Type::Variable;
}

Expression::Type AccessIndexExpression::getType() const noexcept
{
	return Type::AccessIndex;
}

Expression::Type AccessIdentifierExpression::getType() const noexcept
{
	return Type::AccessIdentifier;
}

Expression::Type AssignmentExpression::getType() const noexcept
{
	return Type::Assignment;
}

Expression::Type UnaryExpression::getType() const noexcept
{
	return Type::Unary;
}

Expression::Type BinaryExpression::getType() const noexcept
{
	return Type::Binary;
}

Expression::Type FunctionCallExpression::getType() const noexcept
{
	return Type::FunctionCall;
}

Expression::Type BuiltInFunctionCallExpression::getType() const noexcept
{
	return Type::BuiltInFunctionCall;
}

Expression::Type CastExpression::getType() const noexcept
{
	return Type::Cast;
}

Expression::Type SwizzleExpression::getType() const noexcept
{
	return Type::Swizzle;
}

Expression::Type TernaryExpression::getType() const noexcept
{
	return Type::Ternary;
}
