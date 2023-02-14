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

#include <Atema/Shader/Ast/AstCloner.hpp>

using namespace at;

AstCloner::AstCloner()
{
}

AstCloner::~AstCloner()
{
}

UPtr<Statement> AstCloner::clone(const UPtr<Statement>& statement)
{
	return clone(statement.get());
}

UPtr<Expression> AstCloner::clone(const UPtr<Expression>& expression)
{
	return clone(expression.get());
}

UPtr<Statement> AstCloner::clone(const Statement* statementPtr)
{
	if (!statementPtr)
		return {};

	switch (statementPtr->getType())
	{
#define ATEMA_MACROLIST_SHADERASTSTATEMENT(at_statement) case Statement::Type::at_statement: \
	{ \
		return std::move(clone(static_cast<const at_statement ## Statement&>(*statementPtr))); \
	}
#include <Atema/Shader/Ast/StatementMacroList.hpp>

		default:
		{
			ATEMA_ERROR("Invalid Statement type");
		}
	}

	return {};
}

UPtr<Expression> AstCloner::clone(const Expression* expressionPtr)
{
	if (!expressionPtr)
		return {};

	switch (expressionPtr->getType())
	{
#define ATEMA_MACROLIST_SHADERASTEXPRESSION(at_expression) case Expression::Type::at_expression: \
	{ \
		return std::move(clone(static_cast<const at_expression ## Expression&>(*expressionPtr))); \
	}
#include <Atema/Shader/Ast/ExpressionMacroList.hpp>

		default:
		{
			ATEMA_ERROR("Invalid Expression type");
		}
	}

	return {};
}

UPtr<ConditionalStatement> AstCloner::clone(const ConditionalStatement& statement)
{
	auto cloneStatement = std::make_unique<ConditionalStatement>();

	cloneStatement->elseStatement = clone(statement.elseStatement);
	cloneStatement->branches.reserve(statement.branches.size());

	for (auto& branch : statement.branches)
	{
		ConditionalStatement::Branch cloneBranch;
		cloneBranch.condition = clone(branch.condition);
		cloneBranch.statement = clone(branch.statement);

		cloneStatement->branches.push_back(std::move(cloneBranch));
	}

	return std::move(cloneStatement);
}

UPtr<ForLoopStatement> AstCloner::clone(const ForLoopStatement& statement)
{
	auto cloneStatement = std::make_unique<ForLoopStatement>();

	cloneStatement->initialization = clone(statement.initialization);
	cloneStatement->condition = clone(statement.condition);
	cloneStatement->increase = clone(statement.increase);
	cloneStatement->statement = clone(statement.statement);

	return std::move(cloneStatement);
}

UPtr<WhileLoopStatement> AstCloner::clone(const WhileLoopStatement& statement)
{
	auto cloneStatement = std::make_unique<WhileLoopStatement>();

	cloneStatement->condition = clone(statement.condition);
	cloneStatement->statement = clone(statement.statement);

	return std::move(cloneStatement);
}

UPtr<DoWhileLoopStatement> AstCloner::clone(const DoWhileLoopStatement& statement)
{
	auto cloneStatement = std::make_unique<DoWhileLoopStatement>();

	cloneStatement->condition = clone(statement.condition);
	cloneStatement->statement = clone(statement.statement);

	return std::move(cloneStatement);
}

UPtr<VariableDeclarationStatement> AstCloner::clone(const VariableDeclarationStatement& statement)
{
	auto cloneStatement = std::make_unique<VariableDeclarationStatement>();

	cloneStatement->name = statement.name;
	cloneStatement->qualifiers = statement.qualifiers;
	cloneStatement->type = statement.type;
	cloneStatement->value = clone(statement.value);

	return std::move(cloneStatement);
}

UPtr<StructDeclarationStatement> AstCloner::clone(const StructDeclarationStatement& statement)
{
	auto cloneStatement = std::make_unique<StructDeclarationStatement>();

	cloneStatement->name = statement.name;
	cloneStatement->members = statement.members;

	return std::move(cloneStatement);
}

UPtr<InputDeclarationStatement> AstCloner::clone(const InputDeclarationStatement& statement)
{
	auto cloneStatement = std::make_unique<InputDeclarationStatement>();

	cloneStatement->stage = statement.stage;
	cloneStatement->variables = statement.variables;

	return std::move(cloneStatement);
}

UPtr<OutputDeclarationStatement> AstCloner::clone(const OutputDeclarationStatement& statement)
{
	auto cloneStatement = std::make_unique<OutputDeclarationStatement>();

	cloneStatement->stage = statement.stage;
	cloneStatement->variables = statement.variables;

	return std::move(cloneStatement);
}

UPtr<ExternalDeclarationStatement> AstCloner::clone(const ExternalDeclarationStatement& statement)
{
	auto cloneStatement = std::make_unique<ExternalDeclarationStatement>();

	cloneStatement->variables = statement.variables;

	return std::move(cloneStatement);
}

UPtr<OptionDeclarationStatement> AstCloner::clone(const OptionDeclarationStatement& statement)
{
	auto cloneStatement = std::make_unique<OptionDeclarationStatement>();

	cloneStatement->variables.reserve(statement.variables.size());

	for (auto& variable : statement.variables)
	{
		OptionDeclarationStatement::Variable cloneVariable;
		cloneVariable.name = variable.name;
		cloneVariable.type = variable.type;
		cloneVariable.value = clone(variable.value);

		cloneStatement->variables.push_back(std::move(cloneVariable));
	}

	return std::move(cloneStatement);
}

UPtr<FunctionDeclarationStatement> AstCloner::clone(const FunctionDeclarationStatement& statement)
{
	auto cloneStatement = std::make_unique<FunctionDeclarationStatement>();

	cloneStatement->name = statement.name;
	cloneStatement->returnType = statement.returnType;
	cloneStatement->arguments = statement.arguments;
	cloneStatement->sequence = clone(statement.sequence);

	return std::move(cloneStatement);
}

UPtr<EntryFunctionDeclarationStatement> AstCloner::clone(const EntryFunctionDeclarationStatement& statement)
{
	auto cloneStatement = std::make_unique<EntryFunctionDeclarationStatement>();

	cloneStatement->stage = statement.stage;
	cloneStatement->name = statement.name;
	cloneStatement->returnType = statement.returnType;
	cloneStatement->arguments = statement.arguments;
	cloneStatement->sequence = clone(statement.sequence);

	return std::move(cloneStatement);
}

UPtr<ExpressionStatement> AstCloner::clone(const ExpressionStatement& statement)
{
	auto cloneStatement = std::make_unique<ExpressionStatement>();

	cloneStatement->expression = clone(statement.expression);

	return std::move(cloneStatement);
}

UPtr<BreakStatement> AstCloner::clone(const BreakStatement& statement)
{
	auto cloneStatement = std::make_unique<BreakStatement>();

	return std::move(cloneStatement);
}

UPtr<ContinueStatement> AstCloner::clone(const ContinueStatement& statement)
{
	auto cloneStatement = std::make_unique<ContinueStatement>();

	return std::move(cloneStatement);
}

UPtr<ReturnStatement> AstCloner::clone(const ReturnStatement& statement)
{
	auto cloneStatement = std::make_unique<ReturnStatement>();

	cloneStatement->expression = clone(statement.expression);

	return std::move(cloneStatement);
}

UPtr<DiscardStatement> AstCloner::clone(const DiscardStatement& statement)
{
	auto cloneStatement = std::make_unique<DiscardStatement>();

	return std::move(cloneStatement);
}

UPtr<SequenceStatement> AstCloner::clone(const SequenceStatement& statement)
{
	auto cloneStatement = std::make_unique<SequenceStatement>();

	cloneStatement->statements.reserve(statement.statements.size());

	for (auto& subStatement : statement.statements)
		cloneStatement->statements.push_back(clone(subStatement));

	return std::move(cloneStatement);
}

UPtr<OptionalStatement> AstCloner::clone(const OptionalStatement& statement)
{
	auto cloneStatement = std::make_unique<OptionalStatement>();

	cloneStatement->condition = clone(statement.condition);
	cloneStatement->statement = clone(statement.statement);

	return std::move(cloneStatement);
}

UPtr<IncludeStatement> AstCloner::clone(const IncludeStatement& statement)
{
	auto cloneStatement = std::make_unique<IncludeStatement>();

	cloneStatement->libraries = statement.libraries;

	return std::move(cloneStatement);
}

UPtr<ConstantExpression> AstCloner::clone(const ConstantExpression& expression)
{
	auto cloneExpression = std::make_unique<ConstantExpression>();

	cloneExpression->value = expression.value;

	return std::move(cloneExpression);
}

UPtr<VariableExpression> AstCloner::clone(const VariableExpression& expression)
{
	auto cloneExpression = std::make_unique<VariableExpression>();

	cloneExpression->identifier = expression.identifier;

	return std::move(cloneExpression);
}

UPtr<AccessIndexExpression> AstCloner::clone(const AccessIndexExpression& expression)
{
	auto cloneExpression = std::make_unique<AccessIndexExpression>();

	cloneExpression->expression = clone(expression.expression);
	cloneExpression->index = clone(expression.index);

	return std::move(cloneExpression);
}

UPtr<AccessIdentifierExpression> AstCloner::clone(const AccessIdentifierExpression& expression)
{
	auto cloneExpression = std::make_unique<AccessIdentifierExpression>();

	cloneExpression->expression = clone(expression.expression);
	cloneExpression->identifier = expression.identifier;

	return std::move(cloneExpression);
}

UPtr<AssignmentExpression> AstCloner::clone(const AssignmentExpression& expression)
{
	auto cloneExpression = std::make_unique<AssignmentExpression>();

	cloneExpression->left = clone(expression.left);
	cloneExpression->right = clone(expression.right);

	return std::move(cloneExpression);
}

UPtr<UnaryExpression> AstCloner::clone(const UnaryExpression& expression)
{
	auto cloneExpression = std::make_unique<UnaryExpression>();

	cloneExpression->op = expression.op;
	cloneExpression->operand = clone(expression.operand);

	return std::move(cloneExpression);
}

UPtr<BinaryExpression> AstCloner::clone(const BinaryExpression& expression)
{
	auto cloneExpression = std::make_unique<BinaryExpression>();

	cloneExpression->op = expression.op;
	cloneExpression->left = clone(expression.left);
	cloneExpression->right = clone(expression.right);

	return std::move(cloneExpression);
}

UPtr<FunctionCallExpression> AstCloner::clone(const FunctionCallExpression& expression)
{
	auto cloneExpression = std::make_unique<FunctionCallExpression>();

	cloneExpression->identifier = expression.identifier;
	cloneExpression->arguments.reserve(expression.arguments.size());

	for (auto& argument : expression.arguments)
		cloneExpression->arguments.push_back(clone(argument));

	return std::move(cloneExpression);
}

UPtr<BuiltInFunctionCallExpression> AstCloner::clone(const BuiltInFunctionCallExpression& expression)
{
	auto cloneExpression = std::make_unique<BuiltInFunctionCallExpression>();

	cloneExpression->function = expression.function;
	cloneExpression->arguments.reserve(expression.arguments.size());

	for (auto& argument : expression.arguments)
		cloneExpression->arguments.push_back(clone(argument));

	return std::move(cloneExpression);
}

UPtr<CastExpression> AstCloner::clone(const CastExpression& expression)
{
	auto cloneExpression = std::make_unique<CastExpression>();

	cloneExpression->type = expression.type;
	cloneExpression->components.reserve(expression.components.size());

	for (auto& component : expression.components)
		cloneExpression->components.push_back(clone(component));

	return std::move(cloneExpression);
}

UPtr<SwizzleExpression> AstCloner::clone(const SwizzleExpression& expression)
{
	auto cloneExpression = std::make_unique<SwizzleExpression>();

	cloneExpression->expression = clone(expression.expression);
	cloneExpression->components = expression.components;

	return std::move(cloneExpression);
}

UPtr<TernaryExpression> AstCloner::clone(const TernaryExpression& expression)
{
	auto cloneExpression = std::make_unique<TernaryExpression>();

	cloneExpression->condition = clone(expression.condition);
	cloneExpression->trueValue = clone(expression.trueValue);
	cloneExpression->falseValue = clone(expression.falseValue);

	return std::move(cloneExpression);
}
