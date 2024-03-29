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

#include <Atema/Shader/Atsl/AtslToAstConverter.hpp>
#include <Atema/Shader/Atsl/AtslToken.hpp>
#include <Atema/Shader/Ast/Statement.hpp>
#include <Atema/Shader/Ast/Expression.hpp>
#include <Atema/Shader/Atsl/AtslUtils.hpp>
#include <Atema/Shader/Ast/AstUtils.hpp>

#include <unordered_set>
#include <functional>
#include <stack>

#define ATEMA_ATSL_TOKEN_ERROR(at_str) error(token, at_str)

using namespace at;

namespace
{
	void error(const AtslToken& token, const std::string& error)
	{
		std::string str = "ATSL error (line: " + std::to_string(token.line) + ", col: " + std::to_string(token.column) + ") - ";
		str += error;
		str += " (token : " + token.toString() + " )";

		ATEMA_ERROR(str);
	}

	template <typename T>
	const T& expectType(const AtslToken& token)
	{
		if (!token.value.is<T>())
		{
			ATEMA_ATSL_TOKEN_ERROR("Unexpected token type");
		}

		return token.value.get<T>();
	}
}

AtslToAstConverter::AtslToAstConverter():
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

	bool waitConditionalRightBrace = false;

	using AddStatementFunction = std::function<void(UPtr<Statement>&&)>;

	std::stack<AddStatementFunction> addStatementFunctions;

	{
		AddStatementFunction defaultAddStatement = [&](UPtr<Statement>&& statement)
		{
			m_root->statements.emplace_back(std::move(statement));
		};

		addStatementFunctions.emplace(std::move(defaultAddStatement));
	}

	const AddStatementFunction addStatement = [&](UPtr<Statement>&& statement)
	{
		addStatementFunctions.top()(std::move(statement));
	};

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
						parseAttributes();
						break;
					}
					case AtslSymbol::RightBrace:
					{
						if (waitConditionalRightBrace)
						{
							waitConditionalRightBrace = false;

							addStatementFunctions.pop();

							break;
						}
					}
					default:
					{
						ATEMA_ATSL_TOKEN_ERROR("Unexpected symbol");
						return nullptr;
					}
				}
				
				break;
			}
			// Keyword
			// - input / output / external / option blocks
			// - include blocks
			// - global constants
			// - struct definition
			case AtslTokenType::Keyword:
			{
				const auto keyword = token.value.get<AtslKeyword>();

				switch (keyword)
				{
					case AtslKeyword::Optional:
					{
						expect(iterate(), AtslKeyword::Optional);

						auto conditional = std::make_unique<OptionalStatement>();

						conditional->condition = parseParenthesisExpression();

						AddStatementFunction callback;

						// Wait for a full sequence
						if (get().is(AtslSymbol::LeftBrace))
						{
							iterate();

							waitConditionalRightBrace = true;

							conditional->statement = std::make_unique<SequenceStatement>();

							auto sequence = static_cast<SequenceStatement*>(conditional->statement.get());

							// Wait for one or more statements and pop when we'll find the corresponding right brace
							callback = [sequence](UPtr<Statement>&& statement)
							{
								sequence->statements.emplace_back(std::move(statement));
							};
						}
						// Only wait for the next statement
						else
						{
							auto conditionalPtr = conditional.get();

							// Wait for one statement and pop the add function
							callback = [&addStatementFunctions, conditionalPtr](UPtr<Statement>&& statement)
							{
								conditionalPtr->statement = std::move(statement);

								// We got the statement we needed, for now we'll add statements as before
								addStatementFunctions.pop();
							};
						}

						// Append current optional statement
						addStatement(std::move(conditional));

						// Append the next statement(s) to the optional statement
						addStatementFunctions.emplace(std::move(callback));

						break;
					}
					case AtslKeyword::Include:
					{
						expect(iterate(), AtslKeyword::Include);

						auto includeStatement = std::make_unique<IncludeStatement>();

						AddStatementFunction callback;

						bool multipleLibraries = false;

						// Wait for a full sequence
						if (get().is(AtslSymbol::LeftBrace))
						{
							iterate();

							multipleLibraries = true;
						}

						std::string library;

						while (true)
						{
							auto& includeToken = get();
								
							if (includeToken.type == AtslTokenType::Identifier)
							{
								library += expectType<AtslIdentifier>(iterate());
							}
							else if (includeToken.is(AtslSymbol::Dot))
							{
								iterate();

								library += '.';
							}
							else if (includeToken.is(AtslSymbol::Semicolon))
							{
								iterate();

								if (!library.empty())
								{
									includeStatement->libraries.emplace_back(library);
									library.clear();
								}

								if (!multipleLibraries)
									break;
							}
							else if (includeToken.is(AtslSymbol::RightBrace))
							{
								break;
							}
							else
							{
								ATEMA_ERROR("Invalid token in include statement");
							}
						}

						if (multipleLibraries)
							expect(iterate(), AtslSymbol::RightBrace);

						if (!includeStatement->libraries.empty())
							addStatement(std::move(includeStatement));

						break;
					}
					case AtslKeyword::Input:
					case AtslKeyword::Output:
					case AtslKeyword::External:
					{
						addStatement(parseVariableBlock());

						break;
					}
					case AtslKeyword::Option:
					{
						addStatement(parseOptionDeclaration());

						break;
					}
					case AtslKeyword::Const:
					{
						addStatement(parseVariableDeclaration());

						break;
					}
					case AtslKeyword::Struct:
					{
						addStatement(parseStructDeclaration());

						break;
					}
					default:
					{
						ATEMA_ATSL_TOKEN_ERROR("Unexpected keyword");
						return nullptr;
					}
				}
				
				break;
			}
			// Identifier
			// - function definition
			case AtslTokenType::Identifier:
			{
				// typeIdentifier functionName(...) => function declaration
				if (get(2).is(AtslSymbol::LeftParenthesis))
					addStatement(parseFunctionDeclaration());
				else
					addStatement(parseVariableDeclaration());
				
				break;
			}
			// All other tokens are invalid here
			default :
			{
				ATEMA_ATSL_TOKEN_ERROR("Unexpected token");

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
		ATEMA_ATSL_TOKEN_ERROR(std::string("Expected '") + atsl::getSymbol(symbol) + "'");
	}
}

void AtslToAstConverter::expect(const AtslToken& token, AtslKeyword keyword) const
{
	if (!token.is(keyword))
	{
		ATEMA_ATSL_TOKEN_ERROR(std::string("Expected '") + atsl::getKeyword(keyword) + "'");
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

	if (attribute->getType() != Expression::Type::Variable)
	{
		ATEMA_ERROR("Invalid '" + name + "' attribute : expected identifier");
	}

	return static_cast<VariableExpression&>(*attribute).identifier;
}

void AtslToAstConverter::parseAttributes()
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
			case AtslTokenType::Keyword:
			{
				if (!expectAttribute)
				{
					ATEMA_ATSL_TOKEN_ERROR("Expected attribute delimiter");
				}

				// Only optional keyword is valid here
				expect(token, AtslKeyword::Optional);

				auto expression = parseParenthesisExpression();

				m_attributes["optional"] = std::move(expression);

				expectAttribute = false;

				break;
			}
			case AtslTokenType::Identifier:
			{
				if (!expectAttribute)
				{
					ATEMA_ATSL_TOKEN_ERROR("Expected attribute delimiter");
				}

				auto& attributeName = token.value.get<AtslIdentifier>();

				auto expression = parseParenthesisExpression();

				m_attributes[attributeName] = std::move(expression);

				expectAttribute = false;

				break;
			}
			case AtslTokenType::Symbol:
			{
				if (expectAttribute)
				{
					ATEMA_ATSL_TOKEN_ERROR("Expected attribute identifier");
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
						// Another batch of attributes !
						if (get().is(AtslSymbol::LeftBracket))
						{
							iterate();

							expectAttribute = true;

							continue;
						}

						// We got all attributes
						return;
					}
					default:
					{
						ATEMA_ATSL_TOKEN_ERROR("Expected ',' or ']'");
					}
				}

				break;
			}
			default:
			{
				ATEMA_ATSL_TOKEN_ERROR("Unexpected token");
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
				else if (value.is<int32_t>())
				{
					constantExpression->value = value.get<int32_t>();
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
					ATEMA_ATSL_TOKEN_ERROR("Invalid value type");
				}

				// A value is a full primary expression, don't need to check anything else
				return std::move(constantExpression);
			}
			// - variable (constant, option or variable)
			// - function call
			// - cast
			case AtslTokenType::Identifier:
			{
				auto& identifier = token.value.get<AtslIdentifier>();

				// The identifier must be the first thing we get in this loop
				if (expression)
				{
					ATEMA_ATSL_TOKEN_ERROR("Unexpected identifier '" + identifier + "'");
				}

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
							ATEMA_ATSL_TOKEN_ERROR("Unexpected symbol");
						}
					}
				}
				// Currently parsing expression (variable name resolution)
				else
				{
					switch (token.value.get<AtslSymbol>())
					{
						//TODO: Manage Swizzle
						// AccessIdentifier
						case AtslSymbol::Dot:
						// AccessIndex
						case AtslSymbol::LeftBracket:
						{
							expression = parseAccess(std::move(expression));

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
				ATEMA_ATSL_TOKEN_ERROR("Unexpected token");
			}
		}
	}

	return std::move(expression);
}

UPtr<Expression> AtslToAstConverter::parseOperation(int precedence, UPtr<Expression> lhs)
{
	while (remains())
	{
		const auto& token = get();
		const auto& symbol = expectType<AtslSymbol>(token);

		// If we get a delimiter, just return the current operand
		if (atsl::isExpressionDelimiter(symbol))
			return std::move(lhs);

		// Save index in case we need to reset it later
		const auto currentIndex = m_currentIndex;

		// High precedence resolution (array or member access, ternary, assignment)
		switch (symbol)
		{
			// Assignment
			case AtslSymbol::Equal:
			{
				// Check whether this is an assignment or an equals binary expression
				if (get(1).is(AtslSymbol::Equal))
					break;

				iterate();

				auto expression = std::make_unique<AssignmentExpression>();

				expression->left = std::move(lhs);
				expression->right = parseExpression();

				return std::move(expression);
			}
			// Ternary
			case AtslSymbol::QuestionMark:
			{
				iterate();

				auto expression = std::make_unique<TernaryExpression>();

				expression->condition = std::move(lhs);

				expression->trueValue = parseExpression();

				expect(iterate(), AtslSymbol::Colon);

				expression->falseValue = parseExpression();

				lhs = std::move(expression);

				continue;
			}
			//TODO: Manage Swizzle
			// AccessIdentifier
			case AtslSymbol::Dot:
			// AccessIndex
			case AtslSymbol::LeftBracket:
			{
				lhs = parseAccess(std::move(lhs));

				continue;
			}
			default:
			{
				break;
			}
		}

		iterate();

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
			case AtslSymbol::Caret:
			{
				operation = BinaryOperator::BitwiseXor;

				break;
			}
			case AtslSymbol::Modulo:
			{
				operation = BinaryOperator::Modulo;

				break;
			}
			case AtslSymbol::And:
			{
				if (get().is(AtslSymbol::And))
				{
					iterate();

					operation = BinaryOperator::And;
				}
				else
				{
					operation = BinaryOperator::BitwiseAnd;
				}

				break;
			}
			case AtslSymbol::Or:
			{
				if (get().is(AtslSymbol::Or))
				{
					iterate();

					operation = BinaryOperator::Or;
				}
				else
				{
					operation = BinaryOperator::BitwiseOr;
				}

				break;
			}
			case AtslSymbol::Less:
			{
				if (get().is(AtslSymbol::Equal))
				{
					iterate();

					operation = BinaryOperator::LessOrEqual;
				}
				else if (get().is(AtslSymbol::Less))
				{
					iterate();

					operation = BinaryOperator::BitwiseLeftShift;
				}
				else
				{
					operation = BinaryOperator::Less;
				}

				break;
			}
			case AtslSymbol::Greater:
			{
				if (get().is(AtslSymbol::Equal))
				{
					iterate();

					operation = BinaryOperator::GreaterOrEqual;
				}
				else if (get().is(AtslSymbol::Greater))
				{
					iterate();

					operation = BinaryOperator::BitwiseRightShift;
				}
				else
				{
					operation = BinaryOperator::Greater;
				}

				break;
			}
			case AtslSymbol::Not:
			{
				expect(iterate(), AtslSymbol::Equal);

				operation = BinaryOperator::NotEqual;

				break;
			}
			case AtslSymbol::Equal:
			{
				expect(iterate(), AtslSymbol::Equal);

				operation = BinaryOperator::Equal;

				break;
			}
			default:
			{
				ATEMA_ATSL_TOKEN_ERROR("Unexpected symbol");
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

		// Current operation has a higher (or same) priority : execute it first
		// a * b + c * d
		// lhs = b ; rhs = c ; precedence = * ; nextPrecedence = +
		// return lhs to execute the priority operation, and reset the parsing index
		if (precedence >= nextPrecedence)
		{
			// We reset the index because we didn't process this part : the parent loop will
			m_currentIndex = currentIndex;

			return std::move(lhs);
		}
		// Next operation has a higher priority : execute it first
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
	variable.type = parseType();

	// Get name
	ATEMA_ASSERT(get().type == AtslTokenType::Identifier, "Expected variable name");

	variable.name = iterate().value.get<AtslIdentifier>();

	// Array
	if (get().is(AtslSymbol::LeftBracket))
	{
		iterate();

		const auto& type = variable.type;

		AstArrayType arrayType;
		arrayType.sizeType = AstArrayType::SizeType::Implicit;

		if (get().value.is<AtslBasicValue>())
		{
			const auto& basicValue = expectType<AtslBasicValue>(iterate());

			arrayType.sizeType = AstArrayType::SizeType::Constant;

			if (basicValue.is<int32_t>())
				arrayType.size = static_cast<size_t>(basicValue.get<int32_t>());
			else if (basicValue.is<uint32_t>())
				arrayType.size = static_cast<size_t>(basicValue.get<uint32_t>());
		}
		else if (get().value.is<AtslIdentifier>())
		{
			arrayType.sizeType = AstArrayType::SizeType::Option;
			arrayType.optionName = expectType<AtslIdentifier>(iterate());
		}

		if (type.is<AstPrimitiveType>())
			arrayType.componentType = type.get<AstPrimitiveType>();
		else if (type.is<AstVectorType>())
			arrayType.componentType = type.get<AstVectorType>();
		else if (type.is<AstMatrixType>())
			arrayType.componentType = type.get<AstMatrixType>();
		else if (type.is<AstSamplerType>())
			arrayType.componentType = type.get<AstSamplerType>();
		else if (type.is<AstStructType>())
			arrayType.componentType = type.get<AstStructType>();
		else
			ATEMA_ERROR("Invalid array type");

		expect(iterate(), AtslSymbol::RightBracket);

		variable.type = arrayType;
	}

	// Optional expression for initial value
	if (get().is(AtslSymbol::Equal))
	{
		iterate();

		variable.value = parseExpression();
	}

	expect(iterate(), AtslSymbol::Semicolon);

	return variable;
}

UPtr<Statement> AtslToAstConverter::parseVariableBlock()
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
			tmp->stage = atsl::getShaderStage(expectAttributeIdentifier("stage"));

			auto statementPtr = tmp.get();

			addVariable = [this, statementPtr](const VariableData& variableData)
			{
				InputDeclarationStatement::Variable variable;
				variable.name = variableData.name;
				variable.type = variableData.type;
				variable.location = getAttribute("location");

				if (!variable.type.isOneOf<AstPrimitiveType, AstVectorType, AstMatrixType>())
				{
					ATEMA_ERROR("Input must be a primitive, a vector or a matrix");
				}

				if (hasAttribute("optional"))
					variable.condition = getAttribute("optional");
				
				statementPtr->variables.push_back(variable);
			};

			statement = std::move(tmp);
			
			break;
		}
		case AtslKeyword::Output:
		{
			auto tmp = std::make_unique<OutputDeclarationStatement>();
			tmp->stage = atsl::getShaderStage(expectAttributeIdentifier("stage"));

			auto statementPtr = tmp.get();

			addVariable = [this, statementPtr](const VariableData& variableData)
			{
				OutputDeclarationStatement::Variable variable;
				variable.name = variableData.name;
				variable.type = variableData.type;
				variable.location = getAttribute("location");

				if (hasAttribute("optional"))
					variable.condition = getAttribute("optional");

				if (!variable.type.isOneOf<AstPrimitiveType, AstVectorType, AstMatrixType>())
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
				variable.setIndex = getAttribute("set");
				variable.bindingIndex = getAttribute("binding");
				variable.structLayout = StructLayout::Default;

				if (hasAttribute("optional"))
					variable.condition = getAttribute("optional");

				if (hasAttribute("layout"))
					variable.structLayout = atsl::getStructLayout(expectAttributeIdentifier("layout"));

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

	bool loop = false;

	// List of variables
	if (get().is(AtslSymbol::LeftBrace))
	{
		iterate();

		loop = true;
	}

	// We got the attributes we needed
	clearAttributes();

	// Get at least one variable
	do
	{
		// We got all options
		if (get().is(AtslSymbol::RightBrace))
		{
			ATEMA_ASSERT(variableCount > 0, "No variable defined");

			iterate();

			break;
		}

		// Get attributes if any
		if (get().is(AtslSymbol::LeftBracket))
			parseAttributes();

		// Add variable
		addVariable(parseVariableDeclarationData());

		variableCount++;

		// Clear attributes
		clearAttributes();
	} while (loop && remains());

	return std::move(statement);
}

UPtr<OptionDeclarationStatement> AtslToAstConverter::parseOptionDeclaration()
{
	auto statement = std::make_unique<OptionDeclarationStatement>();

	expect(iterate(), AtslKeyword::Option);

	bool loop = false;

	// List of variables
	if (get().is(AtslSymbol::LeftBrace))
	{
		iterate();

		loop = true;
	}

	// Get at least one option
	do
	{
		// We got all options
		if (get().is(AtslSymbol::RightBrace))
		{
			ATEMA_ASSERT(!statement->variables.empty(), "No option defined");

			iterate();

			break;
		}

		// Parse another option
		auto variableData = parseVariableDeclarationData();

		OptionDeclarationStatement::Variable variable;

		variable.name = std::move(variableData.name);
		variable.type = variableData.type;
		variable.value = std::move(variableData.value);

		statement->variables.push_back(std::move(variable));
	} while (loop && remains());

	return statement;
}

UPtr<SequenceStatement> AtslToAstConverter::parseBlockSequence()
{
	auto sequenceStatement = std::make_unique<SequenceStatement>();

	expect(iterate(), AtslSymbol::LeftBrace);

	// Parse next token to find what is the meaning of the following statements
	while (remains())
	{
		// Right brace : we got to the end of the block sequence
		if (get().is(AtslSymbol::RightBrace))
			break;

		sequenceStatement->statements.push_back(parseBlockStatement());
	}

	expect(iterate(), AtslSymbol::RightBrace);
	
	return sequenceStatement;
}

UPtr<Statement> AtslToAstConverter::parseBlockStatement()
{
	const auto& token = get();

	switch (token.type)
	{
		// Keyword
		// - Const : const variable declaration / assignment
		// - If : conditional branch
		// - For / While / Do : loop statement
		// - Break / Continue / Return : corresponding statement
		case AtslTokenType::Keyword:
		{
			switch (token.value.get<AtslKeyword>())
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
				// - If : conditional branch
				case AtslKeyword::Optional:
				{
					return parseOptionalBranch();
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
				// - Discard : return statement
				case AtslKeyword::Discard:
				{
					return parseDiscard();
				}
				default:
				{
					ATEMA_ATSL_TOKEN_ERROR("Unexpected keyword in block sequence");
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
		// - AstType : variable declaration or cast
		// - Expression (function call, cast, etc)
		case AtslTokenType::Identifier:
		{
			// Variable declaration
			if (get(1).type == AtslTokenType::Identifier)
			{
				return parseVariableDeclaration();
			}
			// Function call (or cast ?)
			//TODO: Check if we allow casts here
			else if (get(1).is(AtslSymbol::LeftParenthesis))
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
			ATEMA_ATSL_TOKEN_ERROR("Unexpected token type");
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
				branch.statement = parseBlockSequence();
			else
				branch.statement = parseBlockStatement();

			statement->branches.push_back(std::move(branch));

			firstBranch = false;
		}
		// Else statement
		else
		{
			if (get().is(AtslSymbol::LeftBrace))
				statement->elseStatement = parseBlockSequence();
			else
				statement->elseStatement = parseBlockStatement();
		}

	} while (get().is(AtslKeyword::Else));

	return std::move(statement);
}

UPtr<OptionalStatement> AtslToAstConverter::parseOptionalBranch()
{
	expect(iterate(), AtslKeyword::Optional);

	auto statement = std::make_unique<OptionalStatement>();

	statement->condition = parseParenthesisExpression();

	if (get().is(AtslSymbol::LeftBrace))
		statement->statement = parseBlockSequence();
	else
		statement->statement = parseBlockStatement();

	return std::move(statement);
}

UPtr<ForLoopStatement> AtslToAstConverter::parseForLoop()
{
	expect(iterate(), AtslKeyword::For);

	auto statement = std::make_unique<ForLoopStatement>();

	expect(iterate(), AtslSymbol::LeftParenthesis);

	// Check if there is an initialization statement
	if (!get().is(AtslSymbol::Semicolon))
		statement->initialization = parseBlockStatement();
	else
		expect(iterate(), AtslSymbol::Semicolon);

	// Check if there is a condition
	if (!get().is(AtslSymbol::Semicolon))
		statement->condition = parseExpression();
	
	expect(iterate(), AtslSymbol::Semicolon);

	// Check if there is an iteration
	if (!get().is(AtslSymbol::RightParenthesis))
		statement->increase = parseExpression();

	expect(iterate(), AtslSymbol::RightParenthesis);

	// Get loop block
	if (get().is(AtslSymbol::LeftBrace))
		statement->statement = parseBlockSequence();
	else
		statement->statement = parseBlockStatement();

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
		statement->statement = parseBlockSequence();
	else
		statement->statement = parseBlockStatement();

	return std::move(statement);
}

UPtr<DoWhileLoopStatement> AtslToAstConverter::parseDoWhileLoop()
{
	expect(iterate(), AtslKeyword::Do);

	auto statement = std::make_unique<DoWhileLoopStatement>();

	// Get loop block
	statement->statement = parseBlockSequence();

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

	clearAttributes();

	while (remains())
	{
		if (get().is(AtslSymbol::RightBrace))
			break;

		// Get attributes if any
		if (get().is(AtslSymbol::LeftBracket))
			parseAttributes();

		const auto variableData = parseVariableDeclarationData();

		statement->members.push_back({ variableData.name, variableData.type });

		if (hasAttribute("optional"))
			statement->members.back().condition = getAttribute("optional");

		clearAttributes();
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

		entryFunctionStatement->stage = atsl::getShaderStage(expectAttributeIdentifier("entry"));

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

	statement->returnType = parseType();

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

		argument.type = parseType();

		ATEMA_ASSERT(get().type == AtslTokenType::Identifier, "Expected argument name");

		argument.name = iterate().value.get<AtslIdentifier>();

		// There is at least one more argument to get
		statement->arguments.push_back(std::move(argument));
	}

	expect(iterate(), AtslSymbol::RightParenthesis);

	// Function definition
	if (get().is(AtslSymbol::LeftBrace))
		statement->sequence = parseBlockSequence();
	// Declaration only
	else
		expect(iterate(), AtslSymbol::Semicolon);

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

UPtr<DiscardStatement> AtslToAstConverter::parseDiscard()
{
	expect(iterate(), AtslKeyword::Discard);

	auto statement = std::make_unique<DiscardStatement>();

	expect(iterate(), AtslSymbol::Semicolon);

	return std::move(statement);
}

AstType AtslToAstConverter::parseType()
{
	ATEMA_ASSERT(get().type == AtslTokenType::Identifier, "Expected variable type");

	const auto type = atsl::getType(expectType<AtslIdentifier>(iterate()));

	// Array type
	if (get().is(AtslSymbol::LeftBracket))
	{
		iterate();

		AstArrayType arrayType;
		arrayType.sizeType = AstArrayType::SizeType::Implicit;

		if (get().value.is<AtslBasicValue>())
		{
			const auto& basicValue = expectType<AtslBasicValue>(iterate());

			arrayType.sizeType = AstArrayType::SizeType::Constant;

			if (basicValue.is<int32_t>())
				arrayType.size = static_cast<size_t>(basicValue.get<int32_t>());
			else if (basicValue.is<uint32_t>())
				arrayType.size = static_cast<size_t>(basicValue.get<uint32_t>());
		}
		else if (get().value.is<AtslIdentifier>())
		{
			arrayType.sizeType = AstArrayType::SizeType::Option;
			arrayType.optionName = expectType<AtslIdentifier>(iterate());
		}

		if (type.is<AstPrimitiveType>())
			arrayType.componentType = type.get<AstPrimitiveType>();
		else if (type.is<AstVectorType>())
			arrayType.componentType = type.get<AstVectorType>();
		else if (type.is<AstMatrixType>())
			arrayType.componentType = type.get<AstMatrixType>();
		else if (type.is<AstSamplerType>())
			arrayType.componentType = type.get<AstSamplerType>();
		else if (type.is<AstStructType>())
			arrayType.componentType = type.get<AstStructType>();
		else
			ATEMA_ERROR("Invalid array type");

		expect(iterate(), AtslSymbol::RightBracket);

		return arrayType;
	}

	return type;
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

	cast->type = parseType();
	cast->components = parseArguments();

	return std::move(cast);
}

UPtr<Expression> AtslToAstConverter::parseAccess(UPtr<Expression>&& expression)
{
	const auto& token = iterate();
	const auto& symbol = expectType<AtslSymbol>(token);

	switch (symbol)
	{
		// AccessIdentifier
		//TODO: Manage Swizzle
		case AtslSymbol::Dot:
		{
			auto tmp = std::make_unique<AccessIdentifierExpression>();

			tmp->expression = std::move(expression);
			tmp->identifier = expectType<AtslIdentifier>(iterate());

			return std::move(tmp);
		}
		// AccessIndex
		case AtslSymbol::LeftBracket:
		{
			auto tmp = std::make_unique<AccessIndexExpression>();

			tmp->expression = std::move(expression);
			tmp->index = parseExpression();

			expect(iterate(), AtslSymbol::RightBracket);

			return std::move(tmp);
		}
		default:
		{
			ATEMA_ATSL_TOKEN_ERROR("Unexpected symbol");
		}
	}

	return {};
}

void AtslToAstConverter::clearAttributes()
{
	m_attributes.clear();
}
