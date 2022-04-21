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

#include <Atema/Shader/Atsl/AtslToAstConverter.hpp>
#include <Atema/Shader/Atsl/AtslToken.hpp>
#include <Atema/Shader/Ast/Statement.hpp>
#include <Atema/Shader/Ast/Expression.hpp>
#include <Atema/Shader/Atsl/Utils.hpp>
#include <Atema/Shader/Ast/AstUtils.hpp>

#include <unordered_set>
#include <functional>


using namespace at;

namespace
{
	AstShaderStage getShaderStage(const std::string& stage)
	{
		if (stage == "vertex")
			return AstShaderStage::Vertex;
		else if (stage == "fragment")
			return AstShaderStage::Fragment;

		ATEMA_ERROR("Invalid shader stage '" + stage + "'");

		return AstShaderStage::Vertex;
	}

	struct Variable
	{
		Type type;
		std::string name;
	};

	template <typename T>
	const T& expectType(const AtslToken& token)
	{
		if (!token.value.is<T>())
		{
			ATEMA_ERROR("Unexpected token type");
		}

		return token.value.get<T>();
	}

	bool isTypeOrStruct(const std::string& identifier)
	{
		//TODO: Manage struct types
		return atsl::isType(identifier);
	}
}

AtslToAstConverter::AtslToAstConverter():
	m_currentSequence(nullptr),
	m_parentSequence(nullptr),
	m_tokens(nullptr),
	m_currentIndex(0)
{
}

AtslToAstConverter::~AtslToAstConverter()
{
}

UPtr<SequenceStatement> AtslToAstConverter::createAst(const std::vector<AtslToken>& tokens)
{
	// Initialize variables
	m_root = std::make_unique<SequenceStatement>();

	m_currentSequence = m_root.get();
	m_parentSequence = nullptr;

	m_attributes.clear();

	m_tokens = &tokens;
	
	m_currentIndex = 0;

	// Create AST
	
	// On the main sequence we can only find :
	// - attribute declaration for the next block
	// - input / output / external / option blocks
	// - global constants
	// - struct definition
	// - function definition
	while (remains())
	{
		const auto& token = get();

		switch (token.type)
		{
			// Symbol
			// - attribute declaration for the next block
			case AtslTokenType::Symbol:
			{
				const auto symbol = token.value.get<AtslSymbol>();

				// Attribute declaration for the next block
				switch (symbol)
				{
					case AtslSymbol::LeftBracket:
					{
						createAttributes();
						break;
					}
					default:
					{
						ATEMA_ERROR("Unexpected token");
						return nullptr;
					}
				}
				
				break;
			}
			// Keyword
			// - input / output / external / option blocks
			// - global constants
			// - struct definition
			case AtslTokenType::Keyword:
			{
				const auto keyword = token.value.get<AtslKeyword>();

				switch (keyword)
				{
					case AtslKeyword::Input:
					case AtslKeyword::Output:
					case AtslKeyword::External:
					{
						createVariableBlock();
						break;
					}
					case AtslKeyword::Option:
					{
						createOptions();
						break;
					}
					case AtslKeyword::Const:
					{
						createConsts();
						break;
					}
					case AtslKeyword::Struct:
					{
						m_currentSequence->statements.push_back(parseStructDeclaration());

						break;
					}
					default:
					{
						ATEMA_ERROR("Unexpected token");
						return nullptr;
					}
				}
				
				break;
			}
			// Identifier
			// - function definition
			case AtslTokenType::Identifier:
			{
				m_currentSequence->statements.push_back(parseFunctionDeclaration());
				
				break;
			}
			// All other tokens are invalid here
			default :
			{
				ATEMA_ERROR("Unexpected token");
				return nullptr;
			}
		}
	}

	return std::move(m_root);
}

bool AtslToAstConverter::remains(size_t offset) const
{
	return m_currentIndex + offset <= m_tokens->size();
}

const AtslToken& AtslToAstConverter::get(size_t offset) const
{
	ATEMA_ASSERT(remains(offset), "Invalid token index");

	return (*m_tokens)[m_currentIndex + offset];
}

const AtslToken& AtslToAstConverter::iterate(size_t offset)
{
	ATEMA_ASSERT(remains(offset), "Invalid token index");

	const auto& token = get();

	m_currentIndex += offset;

	return token;
}

void AtslToAstConverter::expect(const AtslToken& token, AtslSymbol symbol) const
{
	if (!token.is(symbol))
	{
		ATEMA_ERROR(std::string("Expected '") + atsl::getSymbol(symbol) + "'");
	}
}

void AtslToAstConverter::expect(const AtslToken& token, AtslKeyword keyword) const
{
	if (!token.is(keyword))
	{
		ATEMA_ERROR(std::string("Expected '") + atsl::getKeyword(keyword) + "'");
	}
}

bool AtslToAstConverter::hasAttribute(const AtslIdentifier& identifier) const
{
	return m_attributes.find(identifier) != m_attributes.end();
}

const AtslToAstConverter::Attribute& AtslToAstConverter::getAttribute(const AtslIdentifier& identifier) const
{
	ATEMA_ASSERT(hasAttribute(identifier), "Attribute does not exist");

	return m_attributes.at(identifier);
}

const AtslIdentifier& AtslToAstConverter::expectAttributeIdentifier(const AtslIdentifier& name) const
{
	if (!hasAttribute(name))
	{
		ATEMA_ERROR("Expected '" + name + "' attribute");
	}

	const auto& attribute = getAttribute(name);

	if (!attribute.is<AtslIdentifier>())
	{
		ATEMA_ERROR("Invalid '" + name + "' attribute : expected identifier");
	}

	return attribute.get<AtslIdentifier>();
}

bool AtslToAstConverter::expectAttributeBool(const AtslIdentifier& name) const
{
	if (!hasAttribute(name))
	{
		ATEMA_ERROR("Expected '" + name + "' attribute");
	}

	const auto& attribute = getAttribute(name);

	if (!attribute.is<AtslBasicValue>() || !attribute.get<AtslBasicValue>().is<bool>())
	{
		ATEMA_ERROR("Invalid '" + name + "' attribute : expected boolean");
	}

	return attribute.get<AtslBasicValue>().get<bool>();
}

uint32_t AtslToAstConverter::expectAttributeInt(const AtslIdentifier& name) const
{
	if (!hasAttribute(name))
	{
		ATEMA_ERROR("Expected '" + name + "' attribute");
	}

	const auto& attribute = getAttribute(name);

	if (!attribute.is<AtslBasicValue>() || !attribute.get<AtslBasicValue>().is<uint32_t>())
	{
		ATEMA_ERROR("Invalid '" + name + "' attribute : expected integer");
	}

	return attribute.get<AtslBasicValue>().get<uint32_t>();
}

float AtslToAstConverter::expectAttributeFloat(const AtslIdentifier& name) const
{
	if (!hasAttribute(name))
	{
		ATEMA_ERROR("Expected '" + name + "' attribute");
	}

	const auto& attribute = getAttribute(name);

	if (!attribute.is<AtslBasicValue>() || !attribute.get<AtslBasicValue>().is<float>())
	{
		ATEMA_ERROR("Invalid '" + name + "' attribute : expected float");
	}

	return attribute.get<AtslBasicValue>().get<float>();
}

void AtslToAstConverter::createAttributes()
{
	// Clear previous attributes
	clearAttributes();

	// Check if there are some attributes. If not, just return
	if (get().type != AtslTokenType::Symbol ||
		get().value.get<AtslSymbol>() != AtslSymbol::LeftBracket)
	{
		return;
	}

	// There are some attributes. Get them
	expect(iterate(), AtslSymbol::LeftBracket);

	bool expectAttribute = true;

	while (remains())
	{
		auto& token = iterate();

		switch (token.type)
		{
			case AtslTokenType::Identifier:
			{
				if (!expectAttribute)
				{
					ATEMA_ERROR("Expected attribute delimiter");
				}

				auto& attributeName = token.value.get<AtslIdentifier>();

				expect(iterate(), AtslSymbol::LeftParenthesis);

				if (!remains())
				{
					ATEMA_ERROR("Expected attribute value");
				}

				auto& attributeValue = iterate();

				if (attributeValue.type == AtslTokenType::Identifier)
				{
					const auto value = attributeValue.value.get<AtslIdentifier>();

					m_attributes[attributeName] = value;
				}
				else if (attributeValue.type == AtslTokenType::Value)
				{
					const auto value = attributeValue.value.get<AtslBasicValue>();

					m_attributes[attributeName] = value;
				}
				else
				{
					ATEMA_ERROR("Expected attribute value");
				}

				expect(iterate(), AtslSymbol::RightParenthesis);

				expectAttribute = false;

				break;
			}
			case AtslTokenType::Symbol:
			{
				if (expectAttribute)
				{
					ATEMA_ERROR("Expected attribute identifier");
				}

				switch (token.value.get<AtslSymbol>())
				{
					case AtslSymbol::Comma:
					{
						// Continue for next attribute
						expectAttribute = true;
						
						break;
					}
					case AtslSymbol::RightBracket:
					{
						// We got all attributes
						return;
					}
					default:
					{
						ATEMA_ERROR("Expected ',' or ']'");
					}
				}

				break;
			}
			default:
			{
				ATEMA_ERROR("Unexpected token");
			}
		}
	}
}

UPtr<Expression> AtslToAstConverter::parsePrimaryExpression()
{
	UPtr<Expression> expression;

	while (remains())
	{
		auto& token = get();

		switch (token.type)
		{
			// - constant value
			case AtslTokenType::Value:
			{
				auto& value = token.value.get<AtslBasicValue>();

				iterate();

				auto constantExpression = std::make_unique<ConstantExpression>();

				if (value.is<bool>())
				{
					constantExpression->value = value.get<bool>();
				}
				else if (value.is<uint32_t>())
				{
					constantExpression->value = value.get<uint32_t>();
				}
				else if (value.is<float>())
				{
					constantExpression->value = value.get<float>();
				}
				else
				{
					ATEMA_ERROR("Invalid value type");
				}

				// A value is a full primary expression, don't need to check anything else
				return std::move(constantExpression);
			}
			// - variable (constant, option or variable)
			// - function call
			// - cast
			case AtslTokenType::Identifier:
			{
				// The identifier must be the first thing we get in this loop
				ATEMA_ASSERT(!expression, "Unexpected identifier");

				auto& identifier = token.value.get<AtslIdentifier>();

				// Cast
				if (atsl::isType(identifier))
				{
					expression = parseCast();
				}
				// Function call
				else if (get(1).is(AtslSymbol::LeftParenthesis))
				{
					expression = parseFunctionCall();
				}
				// Variable operations, depends on the following symbol
				else
				{
					expression = parseVariable();
				}

				// We got the first
				continue;
			}
			// - access identifier
			// - access index
			// - parenthesis expression
			// - unary operation
			case AtslTokenType::Symbol:
			{
				// First token : check for parenthesis expression or unary operation
				if (!expression)
				{
					switch (token.value.get<AtslSymbol>())
					{
						// Parenthesis expression
						case AtslSymbol::LeftParenthesis:
						{
							return parseParenthesisExpression();
						}
						// Unary Operation (positive or increment prefix)
						case AtslSymbol::Plus:
						{
							iterate();

							auto tmp = std::make_unique<UnaryExpression>();

							tmp->op = UnaryOperator::Positive;

							// Another identical symbol : unary prefix operation
							if (get().is(AtslSymbol::Plus))
							{
								iterate();

								tmp->op = UnaryOperator::IncrementPrefix;
							}

							// Operator must be followed by a primary expression
							tmp->operand = parsePrimaryExpression();

							// No need to check anything else
							return std::move(tmp);
						}
						// Unary Operation (negative or decrement prefix)
						case AtslSymbol::Minus:
						{
							iterate();

							auto tmp = std::make_unique<UnaryExpression>();

							tmp->op = UnaryOperator::Negative;

							// Another identical symbol : unary prefix operation
							if (get().is(AtslSymbol::Minus))
							{
								iterate();

								tmp->op = UnaryOperator::DecrementPrefix;
							}

							// Operator must be followed by a primary expression
							tmp->operand = parsePrimaryExpression();

							// No need to check anything else
							return std::move(tmp);
						}
						// Unary Operation (not)
						case AtslSymbol::Not:
						{
							iterate();

							auto tmp = std::make_unique<UnaryExpression>();

							tmp->op = UnaryOperator::LogicalNot;
							tmp->operand = parsePrimaryExpression();

							// No need to check anything else
							return std::move(tmp);
						}
						default:
						{
							ATEMA_ERROR("Unexpected symbol");
						}
					}
				}
				// Currently parsing expression (variable name resolution)
				else
				{
					switch (token.value.get<AtslSymbol>())
					{
						// AccessIdentifier
						//TODO: Manage Swizzle
						case AtslSymbol::Dot:
						{
							iterate();

							auto tmp = std::make_unique<AccessIdentifierExpression>();

							tmp->expression = std::move(expression);
							tmp->identifier = expectType<AtslIdentifier>(iterate());

							expression = std::move(tmp);

							continue;
						}
						// AccessIndex
						case AtslSymbol::LeftBracket:
						{
							iterate();

							auto tmp = std::make_unique<AccessIndexExpression>();

							tmp->expression = std::move(expression);
							tmp->index = parseExpression();

							expect(iterate(), AtslSymbol::RightBracket);

							expression = std::move(tmp);

							continue;
						}
						default:
						{
							// Unexpected token, we got the entire expression
							return std::move(expression);
						}
					}
				}

				break;
			}
			default:
			{
				ATEMA_ERROR("Unexpected token");
			}
		}
	}

	return std::move(expression);
}

UPtr<Expression> AtslToAstConverter::parseOperation(int precedence, UPtr<Expression> lhs)
{
	while (remains())
	{
		const auto& symbol = expectType<AtslSymbol>(get());

		// If we get a delimiter, just return the current operand
		if (atsl::isExpressionDelimiter(symbol))
			return std::move(lhs);

		// Save index in case we need to reset it later
		const auto currentIndex = m_currentIndex;

		iterate();

		// High precedence resolution (array or member access, ternary, assignment)
		switch (symbol)
		{
			// Assignment
			case AtslSymbol::Equal:
			{
				// Check whether this is an assignment or an equals binary expression
				if (get().is(AtslSymbol::Equal))
					break;

				auto expression = std::make_unique<AssignmentExpression>();

				expression->left = std::move(lhs);
				expression->right = parseExpression();

				return std::move(expression);
			}
			// Ternary
			case AtslSymbol::QuestionMark:
			{
				auto expression = std::make_unique<TernaryExpression>();

				expression->condition = std::move(lhs);

				expression->trueValue = parseExpression();

				expect(iterate(), AtslSymbol::Colon);

				expression->falseValue = parseExpression();

				lhs = std::move(expression);

				continue;
			}
			default:
			{
				break;
			}
		}

		// Operations
		// Get next operation, compare its precedence with the current one
		// Get next operand, and check which operation needs to occur first
		Variant<UnaryOperator, BinaryOperator> operation;

		switch (symbol)
		{
			case AtslSymbol::Plus:
			{
				if (get().is(AtslSymbol::Plus))
				{
					iterate();

					operation = UnaryOperator::IncrementPostfix;

					break;
				}

				operation = BinaryOperator::Add;

				break;
			}
			case AtslSymbol::Minus:
			{
				if (get().is(AtslSymbol::Minus))
				{
					iterate();

					operation = UnaryOperator::DecrementPostfix;

					break;
				}

				operation = BinaryOperator::Subtract;

				break;
			}
			case AtslSymbol::Multiply:
			{
				operation = BinaryOperator::Multiply;

				break;
			}
			case AtslSymbol::Divide:
			{
				operation = BinaryOperator::Divide;

				break;
			}
			case AtslSymbol::Power:
			{
				operation = BinaryOperator::Power;

				break;
			}
			case AtslSymbol::Modulo:
			{
				operation = BinaryOperator::Modulo;

				break;
			}
			case AtslSymbol::And:
			{
				expect(iterate(), AtslSymbol::And);

				operation = BinaryOperator::And;

				break;
			}
			case AtslSymbol::Or:
			{
				expect(iterate(), AtslSymbol::Or);

				operation = BinaryOperator::Or;

				break;
			}
			case AtslSymbol::Less:
			{
				if (get().is(AtslSymbol::Equal))
				{
					iterate();

					operation = BinaryOperator::LessOrEqual;

					break;
				}

				operation = BinaryOperator::Less;

				break;
			}
			case AtslSymbol::Greater:
			{
				if (get().is(AtslSymbol::Equal))
				{
					iterate();

					operation = BinaryOperator::GreaterOrEqual;

					break;
				}

				operation = BinaryOperator::Greater;

				break;
			}
			case AtslSymbol::Not:
			{
				expect(iterate(), AtslSymbol::Equal);

				operation = BinaryOperator::NotEqual;

				break;
			}
			default:
			{
				ATEMA_ERROR("Unexpected symbol");
			}
		}

		// Unary operators are always executed first
		// Create the expression and iterate
		if (operation.is<UnaryOperator>())
		{
			auto expression = std::make_unique<UnaryExpression>();

			expression->op = operation.get<UnaryOperator>();
			expression->operand = std::move(lhs);

			lhs = std::move(expression);

			continue;
		}

		// Binary operation : we expected a right operand
		auto rhs = parsePrimaryExpression();

		ATEMA_ASSERT(rhs, "Expected right operand for binary operation");

		// Check for next operation precedence
		auto nextPrecedence = getOperatorPrecedence(operation.get<BinaryOperator>());

		// Current operation has a higher priority : execute it first
		// a * b + c * d
		// lhs = b ; rhs = c ; precedence = * ; nextPrecedence = +
		// return lhs to execute the priority operation, and reset the parsing index
		if (precedence > nextPrecedence)
		{
			// We reset the index because we didn't process this part : the parent loop will
			m_currentIndex = currentIndex;

			return std::move(lhs);
		}
		// Next operation has a higher (or same) priority : execute it first
		// a + b * c + d
		// lhs = b ; rhs = c ; precedence = + ; nextPrecedence = *
		// 
		else
		{
			auto expression = std::make_unique<BinaryExpression>();

			expression->op = operation.get<BinaryOperator>();
			expression->left = std::move(lhs);
			expression->right = parseOperation(nextPrecedence, std::move(rhs));

			lhs = std::move(expression);

			continue;
		}
	}

	return std::move(lhs);
}

UPtr<Expression> AtslToAstConverter::parseExpression()
{
	// An expression must be :
	// - A primary expression
	//		- constant value
	//		- variable (constant, option or variable)
	//		- function call
	//		- cast
	//		- access identifier
	//		- access index
	//		- parenthesis expression
	//		- unary operation (prefix)
	// - An operation on that primary expression
	//		- assignment
	//		- ternary
	//		- unary operation (suffix)
	//		- binary operation with another primary expression
	return parseOperation(-1, parsePrimaryExpression());
}

std::vector<UPtr<Expression>> AtslToAstConverter::parseArguments()
{
	std::vector<UPtr<Expression>> arguments;

	expect(iterate(), AtslSymbol::LeftParenthesis);

	while (remains())
	{
		// We got all arguments (possibly none)
		if (get().is(AtslSymbol::RightParenthesis))
			break;

		// If we have at least one argument, we expect a comma
		if (!arguments.empty())
			expect(iterate(), AtslSymbol::Comma);

		// There is at least one more argument to get
		arguments.push_back(parseExpression());
	}
	
	expect(iterate(), AtslSymbol::RightParenthesis);

	// Copy ellision
	return arguments;
}

UPtr<Expression> AtslToAstConverter::parseParenthesisExpression()
{
	expect(iterate(), AtslSymbol::LeftParenthesis);

	auto expression = parseExpression();

	expect(iterate(), AtslSymbol::RightParenthesis);

	return std::move(expression);
}

UPtr<Expression> AtslToAstConverter::createFunctionCall()
{
	return {};
}

AtslToAstConverter::VariableData AtslToAstConverter::parseVariableDeclarationData()
{
	VariableData variable;

	// Check if this is a const variable
	if (get().is(AtslKeyword::Const))
	{
		variable.qualifiers |= VariableQualifier::Const;

		iterate();
	}

	// Get type
	ATEMA_ASSERT(get().type == AtslTokenType::Identifier, "Expected variable type");

	auto& typeIdentifier = iterate().value.get<AtslIdentifier>();

	variable.type = atsl::getType(typeIdentifier);

	// Get name
	ATEMA_ASSERT(get().type == AtslTokenType::Identifier, "Expected variable name");

	variable.name = iterate().value.get<AtslIdentifier>();

	// Optional expression for initial value
	if (get().is(AtslSymbol::Equal))
	{
		iterate();

		variable.value = parseExpression();
	}

	expect(iterate(), AtslSymbol::Semicolon);

	return variable;
}

void AtslToAstConverter::createVariableBlock()
{
	UPtr<Statement> statement;

	// Attributes must have been set before

	// We expect the block type (input / output / external / ...)
	std::function<void(const VariableData&)> addVariable = [](const VariableData&){};
	size_t variableCount = 0;
	
	switch (expectType<AtslKeyword>(iterate()))
	{
		case AtslKeyword::Input:
		{
			auto tmp = std::make_unique<InputDeclarationStatement>();
			tmp->stage = getShaderStage(expectAttributeIdentifier("stage"));

			auto statementPtr = tmp.get();

			addVariable = [this, statementPtr](const VariableData& variableData)
			{
				InputDeclarationStatement::Variable variable;
				variable.name = variableData.name;
				variable.type = variableData.type;
				variable.location = expectAttributeInt("location");

				if (!variable.type.isOneOf<PrimitiveType, VectorType, MatrixType>())
				{
					ATEMA_ERROR("Input must be a primitive, a vector or a matrix");
				}
				
				statementPtr->variables.push_back(variable);
			};

			statement = std::move(tmp);
			
			break;
		}
		case AtslKeyword::Output:
		{
			auto tmp = std::make_unique<OutputDeclarationStatement>();
			tmp->stage = getShaderStage(expectAttributeIdentifier("stage"));

			auto statementPtr = tmp.get();

			addVariable = [this, statementPtr](const VariableData& variableData)
			{
				OutputDeclarationStatement::Variable variable;
				variable.name = variableData.name;
				variable.type = variableData.type;
				variable.location = expectAttributeInt("location");

				if (!variable.type.isOneOf<PrimitiveType, VectorType, MatrixType>())
				{
					ATEMA_ERROR("Output must be a primitive, a vector or a matrix");
				}

				statementPtr->variables.push_back(variable);
			};

			statement = std::move(tmp);
			
			break;
		}
		case AtslKeyword::External:
		{
			auto tmp = std::make_unique<ExternalDeclarationStatement>();

			auto statementPtr = tmp.get();

			addVariable = [this, statementPtr](const VariableData& variableData)
			{
				ExternalDeclarationStatement::Variable variable;
				variable.name = variableData.name;
				variable.type = variableData.type;
				variable.setIndex = expectAttributeInt("set");
				variable.bindingIndex = expectAttributeInt("binding");

				statementPtr->variables.push_back(variable);
			};
			
			statement = std::move(tmp);
			
			break;
		}
		default:
		{
			ATEMA_ERROR("Invalid variable block type, expected one of : 'input', 'output', 'external'");
		}
	}

	// We got the attributes we needed
	clearAttributes();

	// Begin block
	expect(iterate(), AtslSymbol::LeftBrace);

	// Fill block
	while (remains())
	{
		const auto& token = get();

		if (token.type == AtslTokenType::Symbol)
		{
			const auto symbol = token.value.get<AtslSymbol>();

			// Attributes
			if (symbol == AtslSymbol::LeftBracket)
			{
				createAttributes();
			}
			// End of block
			else if (symbol == AtslSymbol::RightBrace)
			{
				if (!variableCount)
				{
					ATEMA_ERROR("No variable defined");
					return;
				}

				// Iterate to the next token and add statement to sequence
				iterate();

				m_currentSequence->statements.push_back(std::move(statement));

				break;
			}
			else
			{
				ATEMA_ERROR("Unexpected token");
			}
		}
		else if (token.type == AtslTokenType::Identifier)
		{
			addVariable(parseVariableDeclarationData());

			variableCount++;

			clearAttributes();
		}
		else
		{
			ATEMA_ERROR("Unexpected token");
		}
	}
}

void AtslToAstConverter::createOptions()
{
	expect(iterate(), AtslKeyword::Option);

	// List of variables
	if (get().is(AtslSymbol::LeftBrace))
	{
		
	}
	// Only one variable
	else
	{
		auto variableData = parseVariableDeclarationData();
	}
}

void AtslToAstConverter::createConsts()
{
}

UPtr<SequenceStatement> AtslToAstConverter::createBlockSequence()
{
	auto sequenceStatement = std::make_unique<SequenceStatement>();

	expect(iterate(), AtslSymbol::LeftBrace);

	// Parse next token to find what is the meaning of the following statements
	while (remains())
	{
		// Right brace : we got to the end of the block sequence
		if (get().is(AtslSymbol::RightBrace))
			break;

		sequenceStatement->statements.push_back(createBlockStatement());
	}

	expect(iterate(), AtslSymbol::RightBrace);
	
	return sequenceStatement;
}

UPtr<Statement> AtslToAstConverter::createBlockStatement()
{
	switch (get().type)
	{
		// Keyword
		// - Const : const variable declaration / assignment
		// - If : conditional branch
		// - For / While / Do : loop statement
		// - Break / Continue / Return : corresponding statement
		case AtslTokenType::Keyword:
		{
			switch (get().value.get<AtslKeyword>())
			{
				// - Const : const variable declaration / assignment
				case AtslKeyword::Const:
				{
					return parseVariableDeclaration();
				}
				// - If : conditional branch
				case AtslKeyword::If:
				{
					return parseConditionalBranch();
				}
				// - For : loop statement
				case AtslKeyword::For:
				{
					return parseForLoop();
				}
				// - While : loop statement
				case AtslKeyword::While:
				{
					return parseWhileLoop();
				}
				// - Do : loop statement
				case AtslKeyword::Do:
				{
					return parseDoWhileLoop();
				}
				// - Break : break statement
				case AtslKeyword::Break:
				{
					return parseBreak();
				}
				// - Continue : continue statement
				case AtslKeyword::Continue:
				{
					return parseContinue();
				}
				// - Return : return statement
				case AtslKeyword::Return:
				{
					return parseReturn();
				}
				default:
				{
					ATEMA_ERROR("Unexpected keyword in block sequence");
				}
			}

			break;
		}
		// Symbol
		// - ++ / -- / () expressions
		case AtslTokenType::Symbol:
		{
			auto statement = std::make_unique<ExpressionStatement>();

			statement->expression = parseExpression();

			return std::move(statement);
		}
		// Identifier
		// - Type : variable declaration or cast
		// - Expression (function call, cast, etc)
		case AtslTokenType::Identifier:
		{
			auto& identifier = get().value.get<AtslIdentifier>();

			if (isTypeOrStruct(identifier))
			{
				// Type(expression) : cast
				if (get(1).is(AtslSymbol::LeftParenthesis))
				{
					auto statement = std::make_unique<ExpressionStatement>();

					statement->expression = parseCast();

					// Parse semicolon after expression statement
					expect(iterate(), AtslSymbol::Semicolon);

					return std::move(statement);
				}
				// Variable declaration
				else if (get(1).type == AtslTokenType::Identifier)
				{
					return parseVariableDeclaration();
				}
				else
				{
					ATEMA_ERROR("Unexpected token");
				}
			}
			// Function call
			else if (get().is(AtslSymbol::LeftParenthesis))
			{
				auto statement = std::make_unique<ExpressionStatement>();

				statement->expression = parseFunctionCall();

				// Parse semicolon after expression statement
				expect(iterate(), AtslSymbol::Semicolon);

				return std::move(statement);
			}
			// Expression
			else
			{
				auto statement = std::make_unique<ExpressionStatement>();

				statement->expression = parseExpression();

				// Parse semicolon after expression statement
				expect(iterate(), AtslSymbol::Semicolon);

				return std::move(statement);
			}

			break;
		}
		default:
		{
			ATEMA_ERROR("Unexpected token type");
		}
	}
	
	return UPtr<Statement>();
}

UPtr<ConditionalStatement> AtslToAstConverter::parseConditionalBranch()
{
	expect(get(), AtslKeyword::If);
	
	auto statement = std::make_unique<ConditionalStatement>();

	bool firstBranch = true;

	do
	{
		iterate(); // Get 'else' of first 'if'

		// If/else if branches
		if (firstBranch || get().is(AtslKeyword::If))
		{
			// Get 'if' after 'else'
			if (!firstBranch)
				iterate();

			ConditionalStatement::Branch branch;

			branch.condition = parseParenthesisExpression();

			if (get().is(AtslSymbol::LeftBrace))
				branch.statement = createBlockSequence();
			else
				branch.statement = createBlockStatement();

			statement->branches.push_back(std::move(branch));

			firstBranch = false;
		}
		// Else statement
		else
		{
			if (get().is(AtslSymbol::LeftBrace))
				statement->elseStatement = createBlockSequence();
			else
				statement->elseStatement = createBlockStatement();
		}

	} while (get().is(AtslKeyword::Else));

	return std::move(statement);
}

UPtr<ForLoopStatement> AtslToAstConverter::parseForLoop()
{
	expect(iterate(), AtslKeyword::For);

	auto statement = std::make_unique<ForLoopStatement>();

	expect(iterate(), AtslSymbol::LeftParenthesis);

	// Check if there is an initialization statement
	if (!get().is(AtslSymbol::Semicolon))
		statement->initialization = createBlockStatement();
	else
		iterate(); // Get the semicolon

	// Check if there is a condition
	if (!get().is(AtslSymbol::Semicolon))
		statement->condition = parseExpression();
	else
		iterate(); // Get the semicolon

	// Check if there is an iteration
	if (!get().is(AtslSymbol::RightParenthesis))
		statement->increase = createBlockStatement();

	expect(iterate(), AtslSymbol::RightParenthesis);

	// Get loop block
	if (get().is(AtslSymbol::LeftBrace))
		statement->statement = createBlockSequence();
	else
		statement->statement = createBlockStatement();

	return std::move(statement);
}

UPtr<WhileLoopStatement> AtslToAstConverter::parseWhileLoop()
{
	expect(iterate(), AtslKeyword::While);

	auto statement = std::make_unique<WhileLoopStatement>();

	expect(iterate(), AtslSymbol::LeftParenthesis);

	// Check if there is a condition
	if (!get().is(AtslSymbol::RightParenthesis))
		statement->condition = parseExpression();

	expect(iterate(), AtslSymbol::RightParenthesis);

	// Get loop block
	if (get().is(AtslSymbol::LeftBrace))
		statement->statement = createBlockSequence();
	else
		statement->statement = createBlockStatement();

	return std::move(statement);
}

UPtr<DoWhileLoopStatement> AtslToAstConverter::parseDoWhileLoop()
{
	expect(iterate(), AtslKeyword::Do);

	auto statement = std::make_unique<DoWhileLoopStatement>();

	// Get loop block
	statement->statement = createBlockSequence();

	// Get condition
	expect(iterate(), AtslKeyword::While);
	expect(iterate(), AtslSymbol::LeftParenthesis);

	// Check if there is a condition
	if (!get().is(AtslSymbol::RightParenthesis))
		statement->condition = parseExpression();

	expect(iterate(), AtslSymbol::RightParenthesis);

	return std::move(statement);
}

UPtr<VariableDeclarationStatement> AtslToAstConverter::parseVariableDeclaration()
{
	auto statement = std::make_unique<VariableDeclarationStatement>();

	auto variableData = parseVariableDeclarationData();

	statement->qualifiers = variableData.qualifiers;
	statement->type = variableData.type;
	statement->name = variableData.name;
	statement->value = std::move(variableData.value);

	return std::move(statement);
}

UPtr<StructDeclarationStatement> AtslToAstConverter::parseStructDeclaration()
{
	auto statement = std::make_unique<StructDeclarationStatement>();

	expect(iterate(), AtslKeyword::Struct);

	statement->name = expectType<AtslIdentifier>(iterate());

	expect(iterate(), AtslSymbol::LeftBrace);

	while (remains())
	{
		if (get().is(AtslSymbol::RightBrace))
			break;

		const auto variableData = parseVariableDeclarationData();

		statement->members.push_back({ variableData.name, variableData.type });
	}

	expect(iterate(), AtslSymbol::RightBrace);

	return statement;
}

UPtr<FunctionDeclarationStatement> AtslToAstConverter::parseFunctionDeclaration()
{
	UPtr<FunctionDeclarationStatement> statement;

	// Entry function in one of the shader stages
	if (hasAttribute("entry"))
	{
		auto entryFunctionStatement = std::make_unique<EntryFunctionDeclarationStatement>();

		entryFunctionStatement->stage = getShaderStage(expectAttributeIdentifier("entry"));

		statement = std::move(entryFunctionStatement);
	}
	// Generic function
	else
	{
		statement = std::make_unique<FunctionDeclarationStatement>();
	}

	// Return type
	//TODO: Handle qualifiers if needed
	ATEMA_ASSERT(get().type == AtslTokenType::Identifier, "Expected function return type identifier");

	statement->returnType = atsl::getType(iterate().value.get<AtslIdentifier>());

	ATEMA_ASSERT(isReturnType(statement->returnType), "Invalid function return type");

	// Function name
	ATEMA_ASSERT(get().type == AtslTokenType::Identifier, "Expected function name");

	statement->name = iterate().value.get<AtslIdentifier>();

	// Arguments
	//TODO: Handle qualifiers if needed
	expect(iterate(), AtslSymbol::LeftParenthesis);

	while (remains())
	{
		// We got all arguments (possibly none)
		if (get().is(AtslSymbol::RightParenthesis))
			break;

		// If we have at least one argument, we expect a comma
		if (!statement->arguments.empty())
			expect(iterate(), AtslSymbol::Comma);

		FunctionDeclarationStatement::Argument argument;

		ATEMA_ASSERT(get().type == AtslTokenType::Identifier, "Expected argument type identifier");

		argument.type = atsl::getType(iterate().value.get<AtslIdentifier>());

		ATEMA_ASSERT(get().type == AtslTokenType::Identifier, "Expected argument name");

		argument.name = iterate().value.get<AtslIdentifier>();

		// There is at least one more argument to get
		statement->arguments.push_back(std::move(argument));
	}

	expect(iterate(), AtslSymbol::RightParenthesis);

	statement->sequence = createBlockSequence();

	return std::move(statement);
}

UPtr<BreakStatement> AtslToAstConverter::parseBreak()
{
	expect(iterate(), AtslKeyword::Break);

	auto statement = std::make_unique<BreakStatement>();

	expect(iterate(), AtslSymbol::Semicolon);

	return std::move(statement);
}

UPtr<ContinueStatement> AtslToAstConverter::parseContinue()
{
	expect(iterate(), AtslKeyword::Continue);

	auto statement = std::make_unique<ContinueStatement>();

	expect(iterate(), AtslSymbol::Semicolon);

	return std::move(statement);
}

UPtr<ReturnStatement> AtslToAstConverter::parseReturn()
{
	expect(iterate(), AtslKeyword::Return);

	auto statement = std::make_unique<ReturnStatement>();

	if (get().is(AtslSymbol::Semicolon))
	{
		iterate();
	}
	else
	{
		statement->expression = parseExpression();

		expect(iterate(), AtslSymbol::Semicolon);
	}

	return std::move(statement);
}

UPtr<VariableExpression> AtslToAstConverter::parseVariable()
{
	auto expression = std::make_unique<VariableExpression>();

	expression->identifier = expectType<AtslIdentifier>(iterate());

	return expression;
}

UPtr<Expression> AtslToAstConverter::parseFunctionCall()
{
	auto& identifier = iterate().value.get<AtslIdentifier>();

	if (atsl::isBuiltInFunction(identifier))
	{
		auto functionCall = std::make_unique<BuiltInFunctionCallExpression>();

		functionCall->function = atsl::getBuiltInFunction(identifier);

		functionCall->arguments = parseArguments();

		return std::move(functionCall);
	}

	auto functionCall = std::make_unique<FunctionCallExpression>();

	functionCall->identifier = identifier;

	functionCall->arguments = parseArguments();

	return std::move(functionCall);
}

UPtr<CastExpression> AtslToAstConverter::parseCast()
{
	auto cast = std::make_unique<CastExpression>();

	cast->type = atsl::getType(iterate().value.get<AtslIdentifier>());
	cast->components = parseArguments();

	return std::move(cast);
}

void AtslToAstConverter::clearAttributes()
{
	m_attributes.clear();
}
