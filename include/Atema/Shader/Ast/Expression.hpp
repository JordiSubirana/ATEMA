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

#ifndef ATEMA_SHADER_AST_EXPRESSION_HPP
#define ATEMA_SHADER_AST_EXPRESSION_HPP

#include <Atema/Shader/Config.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Shader/Ast/Constant.hpp>
#include <Atema/Shader/Ast/Enums.hpp>
#include <Atema/Shader/Ast/Type.hpp>

#include <vector>

namespace at
{
	// Base class
	struct ATEMA_SHADER_API Expression : public NonCopyable
	{
		enum class Type
		{
#define ATEMA_MACROLIST_SHADERASTEXPRESSION(at_expression) at_expression,
#include <Atema/Shader/Ast/ExpressionMacroList.hpp>
		};

		Expression();
		virtual ~Expression();

		virtual Expression::Type getType() const noexcept = 0;
	};

	// Direct value
	struct ATEMA_SHADER_API ConstantExpression : Expression
	{
		Expression::Type getType() const noexcept override;

		ConstantValue value;
	};

	struct ATEMA_SHADER_API VariableExpression : Expression
	{
		Expression::Type getType() const noexcept override;

		std::string identifier;
	};

	// Access 
	struct ATEMA_SHADER_API AccessIndexExpression : Expression
	{
		Expression::Type getType() const noexcept override;

		UPtr<Expression> expression;
		UPtr<Expression> index;
	};

	struct ATEMA_SHADER_API AccessIdentifierExpression : Expression
	{
		Expression::Type getType() const noexcept override;

		UPtr<Expression> expression;
		std::string identifier;
	};

	// Assignment
	struct ATEMA_SHADER_API AssignmentExpression : Expression
	{
		Expression::Type getType() const noexcept override;

		UPtr<Expression> left;
		UPtr<Expression> right;
	};

	// Operations
	struct ATEMA_SHADER_API UnaryExpression : Expression
	{
		Expression::Type getType() const noexcept override;

		UnaryOperator op;
		UPtr<Expression> operand;
	};

	struct ATEMA_SHADER_API BinaryExpression : Expression
	{
		Expression::Type getType() const noexcept override;

		BinaryOperator op;
		UPtr<Expression> left;
		UPtr<Expression> right;
	};

	// Function
	struct ATEMA_SHADER_API FunctionCallExpression : Expression
	{
		Expression::Type getType() const noexcept override;

		std::string identifier;
		std::vector<UPtr<Expression>> arguments;
	};

	struct ATEMA_SHADER_API BuiltInFunctionCallExpression : Expression
	{
		Expression::Type getType() const noexcept override;

		BuiltInFunction function;
		std::vector<UPtr<Expression>> arguments;
	};

	// Misc
	struct ATEMA_SHADER_API CastExpression : Expression
	{
		Expression::Type getType() const noexcept override;

		at::Type type;
		std::vector<UPtr<Expression>> components;
	};

	struct ATEMA_SHADER_API SwizzleExpression : Expression
	{
		Expression::Type getType() const noexcept override;

		UPtr<Expression> expression;
		std::vector<size_t> components;
	};

	struct ATEMA_SHADER_API TernaryExpression : Expression
	{
		Expression::Type getType() const noexcept override;

		UPtr<Expression> condition;
		UPtr<Expression> trueValue;
		UPtr<Expression> falseValue;
	};

	
}

#endif
