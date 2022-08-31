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

#ifndef ATEMA_SHADER_AST_ASTRECURSIVEVISITOR_HPP
#define ATEMA_SHADER_AST_ASTRECURSIVEVISITOR_HPP

#include <Atema/Shader/Config.hpp>
#include <Atema/Shader/Ast/AstVisitor.hpp>

namespace at
{
	class ATEMA_SHADER_API AstRecursiveVisitor : public AstVisitor
	{
	public:
		AstRecursiveVisitor();
		~AstRecursiveVisitor();

		void visit(ConditionalStatement& statement) override;
		void visit(ForLoopStatement& statement) override;
		void visit(WhileLoopStatement& statement) override;
		void visit(DoWhileLoopStatement& statement) override;
		void visit(VariableDeclarationStatement& statement) override;
		void visit(StructDeclarationStatement& statement) override;
		void visit(InputDeclarationStatement& statement) override;
		void visit(OutputDeclarationStatement& statement) override;
		void visit(ExternalDeclarationStatement& statement) override;
		void visit(OptionDeclarationStatement& statement) override;
		void visit(FunctionDeclarationStatement& statement) override;
		void visit(EntryFunctionDeclarationStatement& statement) override;
		void visit(ExpressionStatement& statement) override;
		void visit(ReturnStatement& statement) override;
		void visit(SequenceStatement& statement) override;
		void visit(OptionalStatement& statement) override;

		void visit(AccessIndexExpression& expression) override;
		void visit(AccessIdentifierExpression& expression) override;
		void visit(AssignmentExpression& expression) override;
		void visit(UnaryExpression& expression) override;
		void visit(BinaryExpression& expression) override;
		void visit(FunctionCallExpression& expression) override;
		void visit(BuiltInFunctionCallExpression& expression) override;
		void visit(CastExpression& expression) override;
		void visit(SwizzleExpression& expression) override;
		void visit(TernaryExpression& expression) override;
		
	};

	class ATEMA_SHADER_API AstConstRecursiveVisitor : public AstConstVisitor
	{
	public:
		AstConstRecursiveVisitor();
		~AstConstRecursiveVisitor();

		void visit(const ConditionalStatement& statement) override;
		void visit(const ForLoopStatement& statement) override;
		void visit(const WhileLoopStatement& statement) override;
		void visit(const DoWhileLoopStatement& statement) override;
		void visit(const VariableDeclarationStatement& statement) override;
		void visit(const StructDeclarationStatement& statement) override;
		void visit(const InputDeclarationStatement& statement) override;
		void visit(const OutputDeclarationStatement& statement) override;
		void visit(const ExternalDeclarationStatement& statement) override;
		void visit(const OptionDeclarationStatement& statement) override;
		void visit(const FunctionDeclarationStatement& statement) override;
		void visit(const EntryFunctionDeclarationStatement& statement) override;
		void visit(const ExpressionStatement& statement) override;
		void visit(const ReturnStatement& statement) override;
		void visit(const SequenceStatement& statement) override;
		void visit(const OptionalStatement& statement) override;

		void visit(const AccessIndexExpression& expression) override;
		void visit(const AccessIdentifierExpression& expression) override;
		void visit(const AssignmentExpression& expression) override;
		void visit(const UnaryExpression& expression) override;
		void visit(const BinaryExpression& expression) override;
		void visit(const FunctionCallExpression& expression) override;
		void visit(const BuiltInFunctionCallExpression& expression) override;
		void visit(const CastExpression& expression) override;
		void visit(const SwizzleExpression& expression) override;
		void visit(const TernaryExpression& expression) override;

	};
}

#endif