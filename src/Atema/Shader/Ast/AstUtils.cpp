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

#include <Atema/Shader/Ast/AstUtils.hpp>

using namespace at;

// Precedence
// 10	- Postfix
// 9	- Prefix / Sign / Not
// 8	- Power
// 7	- Multiply / Divide / Modulo
// 6	- Add / Subtract
// 5	- Comparison (Less or Greater)
// 4	- Equality
// 3	- Logical AND
// 2	- Logical OR

int at::getOperatorPrecedence(UnaryOperator op)
{
	switch (op)
	{
		case UnaryOperator::IncrementPostfix:
		case UnaryOperator::DecrementPostfix: return 10;

		case UnaryOperator::Positive:
		case UnaryOperator::Negative:
		case UnaryOperator::LogicalNot:
		case UnaryOperator::IncrementPrefix:
		case UnaryOperator::DecrementPrefix: return 9;

		default: break;
	}
	
	return -1;
}

int at::getOperatorPrecedence(BinaryOperator op)
{
	switch (op)
	{
		case BinaryOperator::Power: return 8;

		case BinaryOperator::Multiply:
		case BinaryOperator::Divide:
		case BinaryOperator::Modulo: return 7;

		case BinaryOperator::Add:
		case BinaryOperator::Subtract: return 6;

		case BinaryOperator::Less:
		case BinaryOperator::Greater:
		case BinaryOperator::LessOrEqual:
		case BinaryOperator::GreaterOrEqual: return 5;

		case BinaryOperator::Equal:
		case BinaryOperator::NotEqual: return 4;

		case BinaryOperator::And: return 3;

		case BinaryOperator::Or: return 2;

		default: break;
	}

	return -1;
}
