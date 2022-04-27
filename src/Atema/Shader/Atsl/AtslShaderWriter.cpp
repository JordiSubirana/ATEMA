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

#include <Atema/Shader/Ast/AstUtils.hpp>
#include <Atema/Shader/Atsl/AtslShaderWriter.hpp>
#include <Atema/Shader/Atsl/Utils.hpp>

using namespace at;

AtslShaderWriter::AtslShaderWriter(std::ostream& ostream) : m_ostream(ostream), m_indent(0)
{
}

AtslShaderWriter::~AtslShaderWriter()
{
}

void AtslShaderWriter::write(const ConditionalStatement& statement)
{
	for (auto& branch : statement.branches)
	{
		m_ostream << "if (";

		ShaderWriter::write(branch.condition);

		m_ostream << ")";

		beginBlock();

		ShaderWriter::write(branch.statement);

		endBlock();
	}
}

void AtslShaderWriter::write(const ForLoopStatement& statement)
{
	m_ostream << "for (";

	if (statement.initialization)
		ShaderWriter::write(statement.initialization);

	m_ostream << "; ";

	if (statement.condition)
		ShaderWriter::write(statement.condition);

	m_ostream << "; ";

	if (statement.increase)
		ShaderWriter::write(statement.increase);

	beginBlock();

	ShaderWriter::write(statement.statement);

	endBlock();
}

void AtslShaderWriter::write(const WhileLoopStatement& statement)
{
	m_ostream << "while (";

	ShaderWriter::write(statement.condition);

	m_ostream << ")";

	beginBlock();

	ShaderWriter::write(statement.statement);

	endBlock();
}

void AtslShaderWriter::write(const DoWhileLoopStatement& statement)
{
	m_ostream << "do";

	beginBlock();

	ShaderWriter::write(statement.statement);

	endBlock();

	m_ostream << "while (";

	ShaderWriter::write(statement.condition);

	m_ostream << ")";

	addDelimiter();
}

void AtslShaderWriter::write(const VariableDeclarationStatement& statement)
{
	if (statement.qualifiers & VariableQualifier::Const)
		m_ostream << "const ";

	writeVariableDeclaration(statement.type, statement.name, statement.value.get());
}

void AtslShaderWriter::write(const StructDeclarationStatement& statement)
{
	m_ostream << "struct " << statement.name;

	beginBlock();

	for (auto& member : statement.members)
	{
		writeVariableDeclaration(member.type, member.name);

		// Add line except for last element
		if (&member != &statement.members.back())
			newLine();
	}

	endBlock();
}

void AtslShaderWriter::write(const InputDeclarationStatement& statement)
{
	// Write block declaration attributes
	const std::vector<Attribute> blockAttributes =
	{
		{"stage", AtslIdentifier(atsl::getShaderStageStr(statement.stage))}
	};

	writeAttributes(blockAttributes);

	newLine();

	// Begin block
	m_ostream << "input";

	if (statement.variables.size() != 1)
		beginBlock();
	else
		m_ostream << " ";

	// Write variables
	std::vector<Attribute> variableAttributes =
	{
		{"location", AtslBasicValue(0)}
	};

	for (auto& variable : statement.variables)
	{
		variableAttributes[0].value = static_cast<int32_t>(variable.location);

		writeAttributes(variableAttributes);

		m_ostream << " ";

		writeVariableDeclaration(variable.type, variable.name);

		// Add line except for last element
		if (&variable != &statement.variables.back())
			newLine();
	}

	// End block
	if (statement.variables.size() != 1)
		endBlock();
}

void AtslShaderWriter::write(const OutputDeclarationStatement& statement)
{
	// Write block declaration attributes
	const std::vector<Attribute> blockAttributes =
	{
		{"stage", AtslIdentifier(atsl::getShaderStageStr(statement.stage))}
	};

	writeAttributes(blockAttributes);

	newLine();

	// Begin block
	m_ostream << "output";

	if (statement.variables.size() != 1)
		beginBlock();
	else
		m_ostream << " ";

	// Write variables
	std::vector<Attribute> variableAttributes =
	{
		{"location", AtslBasicValue(0)}
	};

	for (auto& variable : statement.variables)
	{
		variableAttributes[0].value = static_cast<int32_t>(variable.location);

		writeAttributes(variableAttributes);

		m_ostream << " ";

		writeVariableDeclaration(variable.type, variable.name);

		// Add line except for last element
		if (&variable != &statement.variables.back())
			newLine();
	}

	// End block
	if (statement.variables.size() != 1)
		endBlock();
}

void AtslShaderWriter::write(const ExternalDeclarationStatement& statement)
{
	// Write block declaration attributes
	const std::vector<Attribute> blockAttributes =
	{
		{"stage", AtslIdentifier(atsl::getShaderStageStr(statement.stage))}
	};

	writeAttributes(blockAttributes);

	newLine();

	// Begin block
	m_ostream << "external";

	if (statement.variables.size() != 1)
		beginBlock();
	else
		m_ostream << " ";

	// Write variables
	std::vector<Attribute> variableAttributes =
	{
		{"set", AtslBasicValue(0)},
		{"binding", AtslBasicValue(0)}
	};

	for (auto& variable : statement.variables)
	{
		variableAttributes[0].value = static_cast<int32_t>(variable.setIndex);
		variableAttributes[1].value = static_cast<int32_t>(variable.bindingIndex);

		writeAttributes(variableAttributes);

		m_ostream << " ";

		writeVariableDeclaration(variable.type, variable.name);

		// Add line except for last element
		if (&variable != &statement.variables.back())
			newLine();
	}

	// End block
	if (statement.variables.size() != 1)
		endBlock();
}

void AtslShaderWriter::write(const OptionDeclarationStatement& statement)
{
	m_ostream << "option";

	if (statement.variables.size() > 1)
		beginBlock();
	else
		m_ostream << " ";

	for (auto& variable : statement.variables)
	{
		writeVariableDeclaration(variable.type, variable.name, variable.value.get());

		// Add line except for last element
		if (&variable != &statement.variables.back())
			newLine();
	}

	if (statement.variables.size() > 1)
		endBlock();
}

void AtslShaderWriter::write(const FunctionDeclarationStatement& statement)
{
	writeType(statement.returnType);

	m_ostream << " " << statement.name << "(";

	for (auto& argument : statement.arguments)
	{
		writeType(argument.type);

		m_ostream << " " << argument.name;

		// Add comma except for last element
		if (&argument != &statement.arguments.back())
			m_ostream << ", ";
	}

	m_ostream << ")";

	beginBlock();

	if (statement.sequence)
		write(*statement.sequence);

	endBlock();
}

void AtslShaderWriter::write(const EntryFunctionDeclarationStatement& statement)
{
	const std::vector<Attribute> attributes =
	{
		{"entry", AtslIdentifier(atsl::getShaderStageStr(statement.stage))}
	};

	writeAttributes(attributes);

	newLine();

	write(static_cast<const FunctionDeclarationStatement&>(statement));
}

void AtslShaderWriter::write(const ExpressionStatement& statement)
{
	ShaderWriter::write(statement.expression);

	addDelimiter();
}

void AtslShaderWriter::write(const BreakStatement& statement)
{
	m_ostream << "break";

	addDelimiter();
}

void AtslShaderWriter::write(const ContinueStatement& statement)
{
	m_ostream << "continue";

	addDelimiter();
}

void AtslShaderWriter::write(const ReturnStatement& statement)
{
	m_ostream << "return";

	if (statement.expression)
	{
		m_ostream << " ";

		ShaderWriter::write(statement.expression);
	}

	addDelimiter();
}

void AtslShaderWriter::write(const SequenceStatement& statement)
{
	for (auto& subStatement : statement.statements)
	{
		ShaderWriter::write(subStatement);

		// Add line except for last element
		if (&subStatement != &statement.statements.back())
		{
			newLine();
			newLine(); // Add 2 lines to make code better to read
		}
	}
}

void AtslShaderWriter::write(const ConstantExpression& expression)
{
	const auto& value = expression.value;

	if (value.is<bool>())
	{
		m_ostream << (value.get<bool>() ? "true" : "false");
	}
	else if (value.is<int32_t>())
	{
		m_ostream << value.get<int32_t>();
	}
	else if (value.is<uint32_t>())
	{
		m_ostream << value.get<uint32_t>();
	}
	else if (value.is<float>())
	{
		m_ostream << value.get<float>();
	}
	else if (value.is<Vector2i>())
	{
		auto& vec = value.get<Vector2i>();

		m_ostream << "vec2i(" << vec.x << ", " << vec.y << ")";
	}
	else if (value.is<Vector2u>())
	{
		auto& vec = value.get<Vector2u>();

		m_ostream << "vec2u(" << vec.x << ", " << vec.y << ")";
	}
	else if (value.is<Vector2f>())
	{
		auto& vec = value.get<Vector2f>();

		m_ostream << "vec2f(" << vec.x << ", " << vec.y << ")";
	}
	else if (value.is<Vector3i>())
	{
		auto& vec = value.get<Vector3i>();

		m_ostream << "vec3i(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
	}
	else if (value.is<Vector3u>())
	{
		auto& vec = value.get<Vector3u>();

		m_ostream << "vec3u(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
	}
	else if (value.is<Vector3f>())
	{
		auto& vec = value.get<Vector3f>();

		m_ostream << "vec3f(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
	}
	else if (value.is<Vector4i>())
	{
		auto& vec = value.get<Vector4i>();

		m_ostream << "vec4i(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
	}
	else if (value.is<Vector4u>())
	{
		auto& vec = value.get<Vector4u>();

		m_ostream << "vec4i(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
	}
	else if (value.is<Vector4f>())
	{
		auto& vec = value.get<Vector4f>();

		m_ostream << "vec4i(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
	}
	else
	{
		ATEMA_ERROR("Invalid constant type");
	}
}

void AtslShaderWriter::write(const VariableExpression& expression)
{
	m_ostream << expression.identifier;
}

void AtslShaderWriter::write(const AccessIndexExpression& expression)
{
	ShaderWriter::write(expression.expression);

	m_ostream << "[";

	ShaderWriter::write(expression.index);

	m_ostream << "]";
}

void AtslShaderWriter::write(const AccessIdentifierExpression& expression)
{
	ShaderWriter::write(expression.expression);

	m_ostream << "." << expression.identifier;
}

void AtslShaderWriter::write(const AssignmentExpression& expression)
{
	ShaderWriter::write(expression.left);

	m_ostream << " = ";

	ShaderWriter::write(expression.right);
}

void AtslShaderWriter::write(const UnaryExpression& expression)
{
	std::string prefix;
	std::string suffix;

	switch (expression.op)
	{
		case UnaryOperator::IncrementPrefix:
		{
			prefix = "++";
			break;
		}
		case UnaryOperator::IncrementPostfix:
		{
			suffix = "++";
			break;
		}
		case UnaryOperator::DecrementPrefix:
		{
			prefix = "--";
			break;
		}
		case UnaryOperator::DecrementPostfix:
		{
			suffix = "--";
			break;
		}
		case UnaryOperator::Positive:
		{
			prefix = "+";
			break;
		}
		case UnaryOperator::Negative:
		{
			prefix = "-";
			break;
		}
		case UnaryOperator::LogicalNot:
		{
			prefix = "!";
			break;
		}
		default:
		{
			ATEMA_ERROR("Invalid unary operation");
		}
	}

	if (!prefix.empty())
		m_ostream << prefix;

	ShaderWriter::write(*expression.operand);

	if (!suffix.empty())
		m_ostream << suffix;
}

void AtslShaderWriter::write(const BinaryExpression& expression)
{
	std::string operatorStr;

	switch (expression.op)
	{
		case BinaryOperator::Add:
		{
			operatorStr = "+";
			break;
		}
		case BinaryOperator::Subtract:
		{
			operatorStr = "-";
			break;
		}
		case BinaryOperator::Multiply:
		{
			operatorStr = "*";
			break;
		}
		case BinaryOperator::Divide:
		{
			operatorStr = "/";
			break;
		}
		case BinaryOperator::Power:
		{
			operatorStr = "^";
			break;
		}
		case BinaryOperator::Modulo:
		{
			operatorStr = "%";
			break;
		}
		case BinaryOperator::And:
		{
			operatorStr = "&&";
			break;
		};
		case BinaryOperator::Or:
		{
			operatorStr = "||";
			break;
		}
		case BinaryOperator::Less:
		{
			operatorStr = "<";
			break;
		}
		case BinaryOperator::Greater:
		{
			operatorStr = ">";
			break;
		}
		case BinaryOperator::Equal:
		{
			operatorStr = "==";
			break;
		}
		case BinaryOperator::NotEqual:
		{
			operatorStr = "!=";
			break;
		}
		case BinaryOperator::LessOrEqual:
		{
			operatorStr = "<=";
			break;
		}
		case BinaryOperator::GreaterOrEqual:
		{
			operatorStr = ">=";
			break;
		}
		default:
		{
			ATEMA_ERROR("Invalid binary operator");
		}
	}

	const auto currentPrecedence = getOperatorPrecedence(expression.op);

	// Check if we need to protect the operand with parenthesis
	if (expression.left->getType() == Expression::Type::Binary)
	{
		const auto op = static_cast<BinaryExpression&>(*expression.left).op;
		const auto precedence = getOperatorPrecedence(op);

		if (precedence < currentPrecedence)
			m_ostream << "(";

		ShaderWriter::write(expression.left);

		if (precedence < currentPrecedence)
			m_ostream << ")";
	}
	else
	{
		ShaderWriter::write(expression.left);
	}

	m_ostream << " " << operatorStr << " ";

	// Check if we need to protect the operand with parenthesis
	if (expression.right->getType() == Expression::Type::Binary)
	{
		const auto op = static_cast<BinaryExpression&>(*expression.right).op;
		const auto precedence = getOperatorPrecedence(op);

		if (precedence < currentPrecedence)
			m_ostream << "(";

		ShaderWriter::write(expression.right);

		if (precedence < currentPrecedence)
			m_ostream << ")";
	}
	else
	{
		ShaderWriter::write(expression.right);
	}
}

void AtslShaderWriter::write(const FunctionCallExpression& expression)
{
	writeFunctionCall(expression.identifier, expression.arguments);
}

void AtslShaderWriter::write(const BuiltInFunctionCallExpression& expression)
{
	writeFunctionCall(atsl::getBuiltInFunctionStr(expression.function), expression.arguments);
}

void AtslShaderWriter::write(const CastExpression& expression)
{
	writeType(expression.type);

	m_ostream << "(";

	for (auto& component : expression.components)
	{
		ShaderWriter::write(component);

		// Add comma except for last element
		if (&component != &expression.components.back())
			m_ostream << ", ";
	}

	m_ostream << ")";
}

void AtslShaderWriter::write(const SwizzleExpression& expression)
{
	ShaderWriter::write(expression.expression);

	m_ostream << ".";

	//TODO: Manage color components
	for (auto& component : expression.components)
	{
		switch (component)
		{
			case 0:
			{
				m_ostream << "x";
				break;
			}
			case 1:
			{
				m_ostream << "y";
				break;
			}
			case 2:
			{
				m_ostream << "z";
				break;
			}
			case 3:
			{
				m_ostream << "w";
				break;
			}
			default:
			{
				ATEMA_ERROR("Invalid component index");
			}
		}
	}
}

void AtslShaderWriter::write(const TernaryExpression& expression)
{
	m_ostream << "(";

	ShaderWriter::write(expression.condition);

	m_ostream << " ? ";

	ShaderWriter::write(expression.trueValue);

	m_ostream << " : ";

	ShaderWriter::write(expression.falseValue);

	m_ostream << ")";
}

void AtslShaderWriter::newLine()
{
	m_ostream << std::endl;

	for (int i = 0; i < m_indent; i++)
		m_ostream << "\t";
}

void AtslShaderWriter::indent()
{
	m_indent++;
}

void AtslShaderWriter::unindent()
{
	m_indent--;
}

void AtslShaderWriter::beginBlock()
{
	newLine();

	m_ostream << "{";

	indent();

	newLine();
}

void AtslShaderWriter::endBlock()
{
	unindent();

	newLine();

	m_ostream << "}";
}

void AtslShaderWriter::addDelimiter()
{
	m_ostream << ";";
}

void AtslShaderWriter::writeType(Type type)
{
	m_ostream << atsl::getTypeStr(type);
}

void AtslShaderWriter::writeAttributes(const std::vector<Attribute>& attributes)
{
	if (attributes.empty())
		return;

	m_ostream << "[";

	for (auto& attribute : attributes)
	{
		m_ostream << attribute.name << "(";

		if (attribute.value.is<AtslBasicValue>())
		{
			m_ostream << attribute.value.get<AtslBasicValue>();
		}
		else if (attribute.value.is<AtslIdentifier>())
		{
			m_ostream << attribute.value.get<AtslIdentifier>();
		}
		else
		{
			ATEMA_ERROR("Invalid attribute type");
		}

		m_ostream << ")";

		// Add comma except for last element
		if (&attribute != &attributes.back())
			m_ostream << ", ";
	}

	m_ostream << "]";
}

void AtslShaderWriter::writeVariableDeclaration(Type type, std::string name, const Expression* value)
{
	writeType(type);

	m_ostream << " " << name;

	if (value)
	{
		m_ostream << " = ";

		ShaderWriter::write(*value);
	}

	addDelimiter();
}

void AtslShaderWriter::writeFunctionCall(const std::string& functionName, const std::vector<UPtr<Expression>>& arguments)
{
	m_ostream << functionName << "(";

	// Add comma except for last element
	for (auto& argument : arguments)
	{
		ShaderWriter::write(argument);

		if (&argument != &arguments.back())
			m_ostream << ", ";
	}

	m_ostream << ")";
}
