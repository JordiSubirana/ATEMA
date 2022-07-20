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

#include <Atema/Shader/Ast/AstPreprocessor.hpp>
#include <Atema/Shader/Ast/AstEvaluator.hpp>

#include <optional>

using namespace at;

AstPreprocessor::AstPreprocessor() :
	AstRecursiveVisitor()
{
}

AstPreprocessor::~AstPreprocessor()
{
}

void AstPreprocessor::setOption(const std::string& optionName, const ConstantValue& value)
{
	m_options[optionName] = value;
}

void AstPreprocessor::clear()
{
	m_options.clear();
}

void AstPreprocessor::visit(OptionDeclarationStatement& statement)
{
	for (auto& option : statement.variables)
	{
		auto optionalValue = evaluate(*option.value);

		if (optionalValue)
			m_defaultOptions[option.name] = optionalValue.value();
	}
}

UPtr<Statement> AstPreprocessor::process(const Statement& statement)
{
	switch (statement.getType())
	{
#define ATEMA_MACROLIST_SHADERASTSTATEMENT(at_statement) case Statement::Type::at_statement: \
	{ \
		return process(static_cast<const at_statement ## Statement&>(statement)); \
		break; \
	}
#include <Atema/Shader/Ast/StatementMacroList.hpp>
		default:
		{
			ATEMA_ERROR("Invalid statement type");
		}
	}

	return nullptr;
}

UPtr<Expression> AstPreprocessor::process(const Expression& expression)
{
	switch (expression.getType())
	{
#define ATEMA_MACROLIST_SHADERASTEXPRESSION(at_expression) case Expression::Type::at_expression: \
	{ \
		return process(static_cast<const at_expression ## Expression&>(expression)); \
		break; \
	}
#include <Atema/Shader/Ast/ExpressionMacroList.hpp>
		default:
		{
			ATEMA_ERROR("Invalid expression type");
		}
	}

	return nullptr;
}

UPtr<Statement> AstPreprocessor::process(const ConditionalStatement& statement)
{
	auto newStatement = std::make_unique<ConditionalStatement>();

	bool firstBranch = true;

	for (auto& branch : statement.branches)
	{
		auto branchCondition = process(*branch.condition);
		auto branchStatement = process(*branch.statement);

		auto optionalValue = evaluateCondition(*branchCondition);

		// Couldn't evaluate the condition : just keep it the way it is
		if (!optionalValue)
		{
			ConditionalStatement::Branch newBranch;
			newBranch.condition = std::move(branchCondition);
			newBranch.statement = std::move(branchStatement);

			newStatement->branches.emplace_back(std::move(newBranch));
		}
		// Always true : ignore the remaining branches
		else if (optionalValue.value())
		{
			// The first branch is always true : we get rid of the condition
			if (firstBranch)
				return std::move(branchStatement);

			// This is not the first branch but it is always true : this is the new else statement
			newStatement->elseStatement = std::move(branchStatement);
		}
		// Always false : ignore the branch
		else
		{
			continue;
		}

		firstBranch = false;
	}

	// If all the branches are never reached, use the else statement
	if (firstBranch)
	{
		if (statement.elseStatement)
			return process(*statement.elseStatement);

		return nullptr;
	}

	if (statement.elseStatement && !newStatement->elseStatement)
		newStatement->elseStatement = process(*statement.elseStatement);

	return std::move(newStatement);
}

UPtr<Statement> AstPreprocessor::process(const ForLoopStatement& statement)
{
	auto newStatement = std::make_unique<ForLoopStatement>();

	if (statement.initialization)
		newStatement->initialization = process(*statement.initialization);

	if (statement.condition)
		newStatement->condition = process(*statement.condition);

	if (statement.increase)
		newStatement->increase = process(*statement.increase);

	newStatement->statement = process(*statement.statement);

	return std::move(newStatement);
}

UPtr<Statement> AstPreprocessor::process(const WhileLoopStatement& statement)
{
	auto newStatement = std::make_unique<WhileLoopStatement>();

	if (statement.condition)
		newStatement->condition = process(*statement.condition);

	newStatement->statement = process(*statement.statement);

	return std::move(newStatement);
}

UPtr<Statement> AstPreprocessor::process(const DoWhileLoopStatement& statement)
{
	auto newStatement = std::make_unique<DoWhileLoopStatement>();

	if (statement.condition)
		newStatement->condition = process(*statement.condition);

	newStatement->statement = process(*statement.statement);

	return std::move(newStatement);
}

UPtr<Statement> AstPreprocessor::process(const VariableDeclarationStatement& statement)
{
	auto newStatement = std::make_unique<VariableDeclarationStatement>();

	newStatement->name = statement.name;
	newStatement->qualifiers = statement.qualifiers;
	newStatement->type = statement.type;
	newStatement->value = process(*statement.value);

	return std::move(newStatement);
}

UPtr<Statement> AstPreprocessor::process(const StructDeclarationStatement& statement)
{
	auto newStatement = std::make_unique<StructDeclarationStatement>();

	newStatement->name = statement.name;

	for (auto& member : statement.members)
	{
		StructDeclarationStatement::Member newMember;

		if (member.condition)
		{
			auto condition = process(*member.condition);

			auto optionalBool = evaluateCondition(*condition);

			if (optionalBool)
			{
				if (!optionalBool.value())
					continue;
			}
			else
			{
				newMember.condition = std::move(condition);
			}
		}

		newMember.name = member.name;
		newMember.type = member.type;

		newStatement->members.emplace_back(std::move(newMember));
	}

	if (newStatement->members.empty())
		return nullptr;

	return std::move(newStatement);
}

UPtr<Statement> AstPreprocessor::process(const InputDeclarationStatement& statement)
{
	auto newStatement = std::make_unique<InputDeclarationStatement>();

	newStatement->stage = statement.stage;

	for (auto& variable : statement.variables)
	{
		InputDeclarationStatement::Variable newVariable;

		if (newVariable.condition)
		{
			auto condition = process(*newVariable.condition);

			auto optionalBool = evaluateCondition(*condition);

			if (optionalBool)
			{
				if (!optionalBool.value())
					continue;
			}
			else
			{
				newVariable.condition = std::move(condition);
			}
		}

		newVariable.name = variable.name;
		newVariable.type = variable.type;
		newVariable.location = process(*variable.location);

		newStatement->variables.emplace_back(std::move(newVariable));
	}

	if (newStatement->variables.empty())
		return nullptr;

	return std::move(newStatement);
}

UPtr<Statement> AstPreprocessor::process(const OutputDeclarationStatement& statement)
{
	auto newStatement = std::make_unique<OutputDeclarationStatement>();

	newStatement->stage = statement.stage;

	for (auto& variable : statement.variables)
	{
		OutputDeclarationStatement::Variable newVariable;

		if (newVariable.condition)
		{
			auto condition = process(*newVariable.condition);

			auto optionalBool = evaluateCondition(*condition);

			if (optionalBool)
			{
				if (!optionalBool.value())
					continue;
			}
			else
			{
				newVariable.condition = std::move(condition);
			}
		}

		newVariable.name = variable.name;
		newVariable.type = variable.type;
		newVariable.location = process(*variable.location);

		newStatement->variables.emplace_back(std::move(newVariable));
	}

	if (newStatement->variables.empty())
		return nullptr;

	return std::move(newStatement);
}

UPtr<Statement> AstPreprocessor::process(const ExternalDeclarationStatement& statement)
{
	auto newStatement = std::make_unique<ExternalDeclarationStatement>();

	for (auto& variable : statement.variables)
	{
		ExternalDeclarationStatement::Variable newVariable;

		if (newVariable.condition)
		{
			auto condition = process(*newVariable.condition);

			auto optionalBool = evaluateCondition(*condition);

			if (optionalBool)
			{
				if (!optionalBool.value())
					continue;
			}
			else
			{
				newVariable.condition = std::move(condition);
			}
		}

		newVariable.name = variable.name;
		newVariable.type = variable.type;
		newVariable.setIndex = process(*variable.setIndex);
		newVariable.bindingIndex = process(*variable.bindingIndex);
		newVariable.structLayout = variable.structLayout;

		newStatement->variables.emplace_back(std::move(newVariable));
	}

	if (newStatement->variables.empty())
		return nullptr;

	return std::move(newStatement);
}

UPtr<Statement> AstPreprocessor::process(const OptionDeclarationStatement& statement)
{
	auto newStatement = std::make_unique<OptionDeclarationStatement>();

	newStatement->variables.reserve(statement.variables.size());

	for (auto& variable : statement.variables)
	{
		auto& newVariable = newStatement->variables.emplace_back();

		newVariable.name = variable.name;
		newVariable.type = variable.type;

		auto optionIt = m_options.find(variable.name);
		if (optionIt != m_options.end())
			newVariable.value = createConstantExpression(optionIt->second);
		else
			newVariable.value = process(*variable.value);
	}

	return std::move(newStatement);
}

UPtr<Statement> AstPreprocessor::process(const FunctionDeclarationStatement& statement)
{
	auto processedStatement = process(*statement.sequence);

	if (!processedStatement || processedStatement->getType() != Statement::Type::Sequence)
		return m_cloner.clone(statement);

	auto functionDeclaration = std::make_unique<FunctionDeclarationStatement>();

	functionDeclaration->name = statement.name;
	functionDeclaration->returnType = statement.returnType;
	functionDeclaration->arguments = statement.arguments;
	functionDeclaration->sequence = std::unique_ptr<SequenceStatement>(static_cast<SequenceStatement*>(processedStatement.release()));

	return std::move(functionDeclaration);
}

UPtr<Statement> AstPreprocessor::process(const EntryFunctionDeclarationStatement& statement)
{
	auto processedStatement = process(*statement.sequence);

	if (!processedStatement || processedStatement->getType() != Statement::Type::Sequence)
		return m_cloner.clone(statement);

	auto functionDeclaration = std::make_unique<EntryFunctionDeclarationStatement>();

	functionDeclaration->stage = statement.stage;
	functionDeclaration->name = statement.name;
	functionDeclaration->returnType = statement.returnType;
	functionDeclaration->arguments = statement.arguments;
	functionDeclaration->sequence = std::unique_ptr<SequenceStatement>(static_cast<SequenceStatement*>(processedStatement.release()));

	return std::move(functionDeclaration);
}

UPtr<Statement> AstPreprocessor::process(const ExpressionStatement& statement)
{
	return m_cloner.clone(statement);
}

UPtr<Statement> AstPreprocessor::process(const BreakStatement& statement)
{
	return m_cloner.clone(statement);
}

UPtr<Statement> AstPreprocessor::process(const ContinueStatement& statement)
{
	return m_cloner.clone(statement);
}

UPtr<Statement> AstPreprocessor::process(const ReturnStatement& statement)
{
	return m_cloner.clone(statement);
}

UPtr<Statement> AstPreprocessor::process(const SequenceStatement& statement)
{
	auto newStatement = std::make_unique<SequenceStatement>();

	for (auto& subStatement : statement.statements)
	{
		auto processedSubStatement = process(*subStatement);

		if (processedSubStatement)
			newStatement->statements.emplace_back(std::move(processedSubStatement));
	}

	if (newStatement->statements.empty())
		return nullptr;

	return std::move(newStatement);
}

UPtr<Statement> AstPreprocessor::process(const OptionalStatement& statement)
{
	auto condition = process(*statement.condition);
	const auto optionalValue = evaluateCondition(*condition);

	if (optionalValue)
	{
		// Always true : replace optional by the child statement
		if (optionalValue.value())
			return process(*statement.statement);
		// Always false : discard the statement
		else
			return nullptr;
	}

	auto optional = std::make_unique<OptionalStatement>();
	optional->condition = std::move(condition);
	optional->statement = process(*statement.statement);

	return std::move(optional);
}

UPtr<Expression> AstPreprocessor::process(const ConstantExpression& expression)
{
	return m_cloner.clone(expression);
}

UPtr<Expression> AstPreprocessor::process(const VariableExpression& expression)
{
	auto optionIt = m_options.find(expression.identifier);
	if (optionIt != m_options.end())
		return createConstantExpression(optionIt->second);

	optionIt = m_defaultOptions.find(expression.identifier);
	if (optionIt != m_defaultOptions.end())
		return createConstantExpression(optionIt->second);

	return m_cloner.clone(expression);
}

UPtr<Expression> AstPreprocessor::process(const AccessIndexExpression& expression)
{
	auto accessIndex = std::make_unique<AccessIndexExpression>();

	accessIndex->expression = process(*expression.expression);
	accessIndex->index = process(*expression.index);

	auto optionalConstant = evaluate(*accessIndex->index);

	if (optionalConstant)
		accessIndex->index = createConstantExpression(optionalConstant.value());

	return createConstantIfPossible(std::move(accessIndex));
}

UPtr<Expression> AstPreprocessor::process(const AccessIdentifierExpression& expression)
{
	return createConstantIfPossible(m_cloner.clone(expression));
}

UPtr<Expression> AstPreprocessor::process(const AssignmentExpression& expression)
{
	auto accessIndex = std::make_unique<AssignmentExpression>();

	accessIndex->left = process(*expression.left);
	accessIndex->right = process(*expression.right);

	auto optionalConstant = evaluate(*accessIndex->right);

	if (optionalConstant)
		accessIndex->right = createConstantExpression(optionalConstant.value());

	return createConstantIfPossible(std::move(accessIndex));
}

UPtr<Expression> AstPreprocessor::process(const UnaryExpression& expression)
{
	auto accessIndex = std::make_unique<UnaryExpression>();

	accessIndex->op = expression.op;
	accessIndex->operand = process(*expression.operand);

	return createConstantIfPossible(std::move(accessIndex));
}

UPtr<Expression> AstPreprocessor::process(const BinaryExpression& expression)
{
	auto binary = std::make_unique<BinaryExpression>();

	binary->op = expression.op;
	binary->left = process(*expression.left);
	binary->right = process(*expression.right);

	return createConstantIfPossible(std::move(binary));
}

UPtr<Expression> AstPreprocessor::process(const FunctionCallExpression& expression)
{
	auto functionCall = std::make_unique<FunctionCallExpression>();

	functionCall->identifier = expression.identifier;
	functionCall->arguments.reserve(expression.arguments.size());

	for (auto& arg : expression.arguments)
		functionCall->arguments.emplace_back(process(*arg));

	return createConstantIfPossible(std::move(functionCall));
}

UPtr<Expression> AstPreprocessor::process(const BuiltInFunctionCallExpression& expression)
{
	auto functionCall = std::make_unique<BuiltInFunctionCallExpression>();

	functionCall->function = expression.function;
	functionCall->arguments.reserve(expression.arguments.size());

	for (auto& arg : expression.arguments)
		functionCall->arguments.emplace_back(process(*arg));

	return createConstantIfPossible(std::move(functionCall));
}

UPtr<Expression> AstPreprocessor::process(const CastExpression& expression)
{
	auto cast = std::make_unique<CastExpression>();

	cast->type = expression.type;

	for (auto& component : expression.components)
		cast->components.emplace_back(process(*component));

	return createConstantIfPossible(std::move(cast));
}

UPtr<Expression> AstPreprocessor::process(const SwizzleExpression& expression)
{
	return createConstantIfPossible(m_cloner.clone(expression));
}

UPtr<Expression> AstPreprocessor::process(const TernaryExpression& expression)
{
	auto ternary = std::make_unique<TernaryExpression>();
	ternary->trueValue = process(*expression.trueValue);
	ternary->falseValue = process(*expression.falseValue);

	const auto condition = process(*expression.condition);

	auto optionalBool = evaluateCondition(*condition);

	if (optionalBool)
	{
		if (optionalBool.value())
			return createConstantIfPossible(std::move(ternary->trueValue));
		else
			return createConstantIfPossible(std::move(ternary->falseValue));
	}

	return createConstantIfPossible(std::move(ternary));
}

std::optional<ConstantValue> AstPreprocessor::evaluate(Expression& expression)
{
	AstEvaluator evaluator;

	return evaluator.evaluate(expression);
}

std::optional<bool> AstPreprocessor::evaluateCondition(Expression& expression)
{
	AstEvaluator evaluator;

	return evaluator.evaluateCondition(expression);
}

UPtr<Expression> AstPreprocessor::createConstantIfPossible(UPtr<Expression>&& value)
{
	auto optionalConstant = evaluate(*value);

	if (optionalConstant)
		return createConstantExpression(optionalConstant.value());

	return std::move(value);
}

UPtr<ConstantExpression> AstPreprocessor::createConstantExpression(const ConstantValue& value)
{
	auto constant = std::make_unique<ConstantExpression>();

	constant->value = value;

	return constant;
}
