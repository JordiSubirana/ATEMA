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

#include <Atema/Shader/Ast/AstRecursiveVisitor.hpp>

using namespace at;

AstRecursiveVisitor::AstRecursiveVisitor() : AstVisitor()
{
}

AstRecursiveVisitor::~AstRecursiveVisitor()
{
}

void AstRecursiveVisitor::visit(ConditionalStatement& statement)
{
	for (auto& branch : statement.branches)
	{
		branch.condition->accept(*this);
		branch.statement->accept(*this);
	}

	if (statement.elseStatement)
		statement.elseStatement->accept(*this);
}

void AstRecursiveVisitor::visit(ForLoopStatement& statement)
{
	if (statement.initialization)
		statement.initialization->accept(*this);

	if (statement.condition)
		statement.condition->accept(*this);

	if (statement.increase)
		statement.increase->accept(*this);

	if (statement.statement)
		statement.statement->accept(*this);
}

void AstRecursiveVisitor::visit(WhileLoopStatement& statement)
{
	statement.condition->accept(*this);
	statement.statement->accept(*this);
}

void AstRecursiveVisitor::visit(DoWhileLoopStatement& statement)
{
	statement.condition->accept(*this);
	statement.statement->accept(*this);
}

void AstRecursiveVisitor::visit(VariableDeclarationStatement& statement)
{
	if (statement.value)
		statement.value->accept(*this);
}

void AstRecursiveVisitor::visit(OptionDeclarationStatement& statement)
{
	for (auto& variable : statement.variables)
	{
		if (variable.value)
			variable.value->accept(*this);
	}
}

void AstRecursiveVisitor::visit(FunctionDeclarationStatement& statement)
{
	statement.sequence->accept(*this);
}

void AstRecursiveVisitor::visit(EntryFunctionDeclarationStatement& statement)
{
	statement.sequence->accept(*this);
}

void AstRecursiveVisitor::visit(ExpressionStatement& statement)
{
	statement.expression->accept(*this);
}

void AstRecursiveVisitor::visit(ReturnStatement& statement)
{
	statement.expression->accept(*this);
}

void AstRecursiveVisitor::visit(SequenceStatement& statement)
{
	for (auto& subStatement : statement.statements)
		subStatement->accept(*this);
}

void AstRecursiveVisitor::visit(AccessIndexExpression& expression)
{
	expression.expression->accept(*this);
	expression.index->accept(*this);
}

void AstRecursiveVisitor::visit(AccessIdentifierExpression& expression)
{
	expression.expression->accept(*this);
}

void AstRecursiveVisitor::visit(AssignmentExpression& expression)
{
	expression.left->accept(*this);
	expression.right->accept(*this);
}

void AstRecursiveVisitor::visit(UnaryExpression& expression)
{
	expression.operand->accept(*this);
}

void AstRecursiveVisitor::visit(BinaryExpression& expression)
{
	expression.left->accept(*this);
	expression.right->accept(*this);
}

void AstRecursiveVisitor::visit(FunctionCallExpression& expression)
{
	for (auto& argument : expression.arguments)
		argument->accept(*this);
}

void AstRecursiveVisitor::visit(BuiltInFunctionCallExpression& expression)
{
	for (auto& argument : expression.arguments)
		argument->accept(*this);
}

void AstRecursiveVisitor::visit(CastExpression& expression)
{
	for (auto& component : expression.components)
		component->accept(*this);
}

void AstRecursiveVisitor::visit(SwizzleExpression& expression)
{
	expression.expression->accept(*this);
}

void AstRecursiveVisitor::visit(TernaryExpression& expression)
{
	expression.condition->accept(*this);
	expression.trueValue->accept(*this);
	expression.falseValue->accept(*this);
}
