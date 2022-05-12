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

#ifndef ATEMA_SHADER_AST_ENUMS_HPP
#define ATEMA_SHADER_AST_ENUMS_HPP

#include <Atema/Core/Flags.hpp>

namespace at
{
	enum class UnaryOperator
	{
		IncrementPrefix,	// ++var
		IncrementPostfix,	// var++
		DecrementPrefix,	// --var
		DecrementPostfix,	// var--
		
		Positive,			// +var
		Negative,			// -var
		
		LogicalNot			// !var
	};

	enum class BinaryOperator
	{
		Add,		// a + b
		Subtract,	// a - b
		Multiply,	// a * b
		Divide,		// a / b
		Power,		// a ^ b
		Modulo,		// a % b

		And,			// a && b
		Or,				// a || b
		
		Less,			// a < b
		Greater,		// a > b
		Equal,			// a == b
		NotEqual,		// a != b
		LessOrEqual,	// a <= b
		GreaterOrEqual	// a >= b
	};

	enum class BuiltInFunction
	{
		Min,
		Max,
		
		Cross,
		Dot,
		Norm,
		
		Sample, // Arguments : sampler, index (primitive type or vector)

		SetVertexPosition // Arguments : position (vector4f)
	};

	enum class VariableQualifier
	{
		None = 0 << 0,
		Const = 0 << 1
	};

	ATEMA_DECLARE_FLAGS(VariableQualifier);

	enum class AstShaderStage
	{
		Vertex,
		Fragment
	};
}

#endif
