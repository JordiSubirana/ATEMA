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

#ifndef ATEMA_SHADER_AST_STATEMENT_HPP
#define ATEMA_SHADER_AST_STATEMENT_HPP

#include <Atema/Shader/Config.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Shader/Ast/Enums.hpp>
#include <Atema/Shader/Ast/Type.hpp>

#include <vector>
#include <string>

namespace at
{
	struct Expression;
	struct SequenceStatement;
	class AstVisitor;
	class AstConstVisitor;

	struct ATEMA_SHADER_API Statement : public NonCopyable
	{
		enum class Type
		{
#define ATEMA_MACROLIST_SHADERASTSTATEMENT(at_statement) at_statement,
#include <Atema/Shader/Ast/StatementMacroList.hpp>
		};

		Statement();
		virtual ~Statement();

		virtual Statement::Statement::Type getType() const noexcept = 0;

		virtual void accept(AstVisitor& visitor) = 0;
		virtual void accept(AstConstVisitor& visitor) const = 0;
	};

	// Branches / conditions
	struct ATEMA_SHADER_API ConditionalStatement : public Statement
	{
		Statement::Type getType() const noexcept override;

		void accept(AstVisitor& visitor) override;
		void accept(AstConstVisitor& visitor) const override;
		
		struct Branch
		{
			UPtr<Expression> condition;
			UPtr<Statement> statement;
		};

		std::vector<Branch> branches;
		UPtr<Statement> elseStatement;
	};

	// Loops
	struct ATEMA_SHADER_API ForLoopStatement : public Statement
	{
		Statement::Type getType() const noexcept override;

		void accept(AstVisitor& visitor) override;
		void accept(AstConstVisitor& visitor) const override;

		UPtr<Statement> initialization;
		UPtr<Expression> condition;
		UPtr<Expression> increase;
		UPtr<Statement> statement;
	};

	struct ATEMA_SHADER_API WhileLoopStatement : public Statement
	{
		Statement::Type getType() const noexcept override;

		void accept(AstVisitor& visitor) override;
		void accept(AstConstVisitor& visitor) const override;

		UPtr<Expression> condition;
		UPtr<Statement> statement;
	};

	struct ATEMA_SHADER_API DoWhileLoopStatement : public Statement
	{
		Statement::Type getType() const noexcept override;

		void accept(AstVisitor& visitor) override;
		void accept(AstConstVisitor& visitor) const override;

		UPtr<Expression> condition;
		UPtr<Statement> statement;
	};

	// Declarations
	struct ATEMA_SHADER_API VariableDeclarationStatement : public Statement
	{
		Statement::Type getType() const noexcept override;

		void accept(AstVisitor& visitor) override;
		void accept(AstConstVisitor& visitor) const override;
		
		std::string name;
		Flags<VariableQualifier> qualifiers;
		at::Type type;
		UPtr<Expression> value;
	};

	struct ATEMA_SHADER_API StructDeclarationStatement : public Statement
	{
		Statement::Type getType() const noexcept override;

		void accept(AstVisitor& visitor) override;
		void accept(AstConstVisitor& visitor) const override;

		struct Member
		{
			std::string name;
			at::Type type;
			Ptr<Expression> condition;
		};

		std::string name;
		std::vector<Member> members;
	};

	// Input / Output
	struct ATEMA_SHADER_API InputDeclarationStatement : public Statement
	{
		Statement::Type getType() const noexcept override;

		void accept(AstVisitor& visitor) override;
		void accept(AstConstVisitor& visitor) const override;

		struct Variable
		{
			std::string name;
			at::Type type;
			Ptr<Expression> location;
			Ptr<Expression> condition;
		};

		AstShaderStage stage;
		std::vector<Variable> variables;
	};

	struct ATEMA_SHADER_API OutputDeclarationStatement : public Statement
	{
		Statement::Type getType() const noexcept override;

		void accept(AstVisitor& visitor) override;
		void accept(AstConstVisitor& visitor) const override;

		struct Variable
		{
			std::string name;
			at::Type type;
			Ptr<Expression> location;
			Ptr<Expression> condition;
		};

		AstShaderStage stage;
		std::vector<Variable> variables;
	};

	struct ATEMA_SHADER_API ExternalDeclarationStatement : public Statement
	{
		Statement::Type getType() const noexcept override;

		void accept(AstVisitor& visitor) override;
		void accept(AstConstVisitor& visitor) const override;

		struct Variable
		{
			std::string name;
			at::Type type;
			Ptr<Expression> setIndex;
			Ptr<Expression> bindingIndex;
			StructLayout structLayout;
			Ptr<Expression> condition;
		};
		
		std::vector<Variable> variables;
	};

	struct ATEMA_SHADER_API OptionDeclarationStatement : public Statement
	{
		Statement::Type getType() const noexcept override;

		void accept(AstVisitor& visitor) override;
		void accept(AstConstVisitor& visitor) const override;

		struct Variable
		{
			std::string name;
			at::Type type;
			UPtr<Expression> value;
		};

		std::vector<Variable> variables;
	};

	// Functions
	struct ATEMA_SHADER_API FunctionDeclarationStatement : public Statement
	{
		Statement::Type getType() const noexcept override;

		void accept(AstVisitor& visitor) override;
		void accept(AstConstVisitor& visitor) const override;

		struct Argument
		{
			std::string name;
			at::Type type;
		};

		std::string name;
		at::Type returnType;
		std::vector<Argument> arguments;
		UPtr<SequenceStatement> sequence;
	};

	struct ATEMA_SHADER_API EntryFunctionDeclarationStatement : public FunctionDeclarationStatement
	{
		Statement::Type getType() const noexcept override;

		void accept(AstVisitor& visitor) override;
		void accept(AstConstVisitor& visitor) const override;

		AstShaderStage stage;
	};

	// Misc
	struct ATEMA_SHADER_API ExpressionStatement : public Statement
	{
		Statement::Type getType() const noexcept override;

		void accept(AstVisitor& visitor) override;
		void accept(AstConstVisitor& visitor) const override;

		UPtr<Expression> expression;
	};

	struct ATEMA_SHADER_API BreakStatement : public Statement
	{
		Statement::Type getType() const noexcept override;

		void accept(AstVisitor& visitor) override;
		void accept(AstConstVisitor& visitor) const override;
	};

	struct ATEMA_SHADER_API ContinueStatement : public Statement
	{
		Statement::Type getType() const noexcept override;

		void accept(AstVisitor& visitor) override;
		void accept(AstConstVisitor& visitor) const override;
	};

	struct ATEMA_SHADER_API ReturnStatement : public Statement
	{
		Statement::Type getType() const noexcept override;

		void accept(AstVisitor& visitor) override;
		void accept(AstConstVisitor& visitor) const override;

		UPtr<Expression> expression;
	};

	struct ATEMA_SHADER_API SequenceStatement : public Statement
	{
		Statement::Statement::Type getType() const noexcept override;

		void accept(AstVisitor& visitor) override;
		void accept(AstConstVisitor& visitor) const override;

		std::vector<UPtr<Statement>> statements;
	};

	struct ATEMA_SHADER_API OptionalStatement : public Statement
	{
		Statement::Statement::Type getType() const noexcept override;

		void accept(AstVisitor& visitor) override;
		void accept(AstConstVisitor& visitor) const override;

		UPtr<Expression> condition;
		UPtr<Statement> statement;
	};
}

#endif
