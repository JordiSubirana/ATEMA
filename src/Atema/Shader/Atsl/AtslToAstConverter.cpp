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

	bool isInOutType(const std::string& typeStr)
	{
		static const std::unordered_set<std::string> s_types =
		{
			"bool", "int", "uint", "float",
			"vec2i", "vec2u", "vec2f",
			"vec3i", "vec3u", "vec3f",
			"vec4i", "vec4u", "vec4f",
			"mat2i", "mat2u", "mat2f",
			"mat3i", "mat3u", "mat3f",
			"mat4i", "mat4u", "mat4f",
		};

		return s_types.find(typeStr) != s_types.end();
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
}

AtslToAstConverter::AtslToAstConverter()
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
						createStruct();
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
				auto& identifier = token.value.get<AtslIdentifier>();

				createFunction(identifier);
				
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
	return m_currentIndex + offset < m_tokens->size();
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

bool AtslToAstConverter::hasAttribute(const AtslIdentifier& identifier)
{
	return m_attributes.find(identifier) != m_attributes.end();
}

const AtslToAstConverter::Attribute& AtslToAstConverter::getAttribute(const AtslIdentifier& identifier)
{
	ATEMA_ASSERT(hasAttribute(identifier), "Attribute does not exist");

	return m_attributes[identifier];
}

const AtslIdentifier& AtslToAstConverter::expectAttributeIdentifier(const AtslIdentifier& name)
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

bool AtslToAstConverter::expectAttributeBool(const AtslIdentifier& name)
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

uint32_t AtslToAstConverter::expectAttributeInt(const AtslIdentifier& name)
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

float AtslToAstConverter::expectAttributeFloat(const AtslIdentifier& name)
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
			}
			default:
			{
				ATEMA_ERROR("Unexpected token");
			}
		}
	}
}

UPtr<Expression> AtslToAstConverter::parseExpression()
{
	auto& token = get();

	UPtr<Expression> expression;

	switch (token.type)
	{
		// - variable (constant, option or variable)
		// - function call
		// - cast
		case AtslTokenType::Identifier:
		{
			auto& identifier = token.value.get<AtslIdentifier>();

			iterate();

			// Cast
			if (atsl::isType(identifier))
			{
				auto tmp = std::make_unique<CastExpression>();
				tmp->type = atsl::getType(identifier);
				tmp->components = parseArguments();

				expression = std::move(tmp);
			}
			else
			{
				// Function call
				if (get().is(AtslSymbol::LeftParenthesis))
				{
					if (atsl::isBuiltInFunction(identifier))
					{
						auto tmp = std::make_unique<BuiltInFunctionCallExpression>();
						tmp->function = atsl::getBuiltInFunction(identifier);
						tmp->arguments = parseArguments();

						expression = std::move(tmp);
					}
					else
					{
						auto tmp = std::make_unique<FunctionCallExpression>();
						tmp->identifier = identifier;
						tmp->arguments = parseArguments();

						expression = std::move(tmp);
					}
				}
				// Variable
				else
				{
					
				}
			}
			
			break;
		}
		// - +value
		// - -value
		// - !value
		// - (expression)
		case AtslTokenType::Symbol:
		{
			switch (token.value.get<AtslSymbol>())
			{
				case AtslSymbol::Plus:
				{
					break;
				}
				case AtslSymbol::Minus:
				{
					break;
				}
				case AtslSymbol::Not:
				{
					break;
				}
				case AtslSymbol::LeftParenthesis:
				{
					break;
				}
				default:
				{
					ATEMA_ERROR("Unexpected symbol");
				}
			}

			break;
		}
		// - constant value
		case AtslTokenType::Value:
		{
			auto& value = token.value.get<AtslBasicValue>();

			iterate();

			auto tmp = std::make_unique<ConstantExpression>();

			if (value.is<bool>())
			{
				tmp->value = value.get<bool>();
			}
			else if (value.is<uint32_t>())
			{
				tmp->value = value.get<uint32_t>();
			}
			else if (value.is<float>())
			{
				tmp->value = value.get<float>();
			}
			else
			{
				ATEMA_ERROR("Invalid value type");
			}

			expression = std::move(tmp);

			break;
		}
		default:
		{
			ATEMA_ERROR("Unexpected token");
		}
	}

	return std::move(expression);
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

AtslToAstConverter::VariableData AtslToAstConverter::createVariable()
{
	VariableData variable;

	// Get type
	{
		const auto& token = expectType<AtslIdentifier>(iterate());

		variable.type = atsl::getType(token);
	}

	// Get name
	{
		variable.name = expectType<AtslIdentifier>(iterate());
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

			addVariable = [this, &tmp](const VariableData& variableData)
			{
				InputDeclarationStatement::Variable variable;
				variable.name = variableData.name;
				variable.type = variableData.type;
				variable.location = expectAttributeInt("location");

				if (!variable.type.isOneOf<PrimitiveType, VectorType, MatrixType>())
				{
					ATEMA_ERROR("Input must be a primitive, a vector or a matrix");
				}
				
				tmp->variables.push_back(variable);
			};

			statement = std::move(tmp);
			
			break;
		}
		case AtslKeyword::Output:
		{
			auto tmp = std::make_unique<OutputDeclarationStatement>();
			tmp->stage = getShaderStage(expectAttributeIdentifier("stage"));

			addVariable = [this, &tmp](const VariableData& variableData)
			{
				OutputDeclarationStatement::Variable variable;
				variable.name = variableData.name;
				variable.type = variableData.type;
				variable.location = expectAttributeInt("location");

				if (!variable.type.isOneOf<PrimitiveType, VectorType, MatrixType>())
				{
					ATEMA_ERROR("Output must be a primitive, a vector or a matrix");
				}

				tmp->variables.push_back(variable);
			};

			statement = std::move(tmp);
			
			break;
		}
		case AtslKeyword::External:
		{
			auto tmp = std::make_unique<ExternalDeclarationStatement>();

			addVariable = [this, &tmp](const VariableData& variableData)
			{
				ExternalDeclarationStatement::Variable variable;
				variable.name = variableData.name;
				variable.type = variableData.type;
				variable.setIndex = expectAttributeInt("set");
				variable.bindingIndex = expectAttributeInt("binding");

				tmp->variables.push_back(variable);
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
			addVariable(createVariable());

			variableCount++;

			clearAttributes();
		}
		else
		{
			ATEMA_ERROR("Unexpected token");
		}
	}

	m_currentSequence->statements.push_back(std::move(statement));
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
		auto variableData = createVariable();
	}
}

void AtslToAstConverter::createConsts()
{
}

void AtslToAstConverter::createStruct()
{
}

void AtslToAstConverter::createFunction(const AtslIdentifier& identifier)
{
}

void AtslToAstConverter::clearAttributes()
{
	m_attributes.clear();
}
