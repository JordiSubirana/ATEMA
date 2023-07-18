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

#include <Atema/Shader/Ast/AstUtils.hpp>
#include <Atema/Shader/Atsl/AtslShaderWriter.hpp>
#include <Atema/Shader/Atsl/AtslUtils.hpp>

using namespace at;

AtslShaderWriter::AtslShaderWriter(std::ostream& ostream) : m_ostream(ostream), m_indent(0)
{
}

AtslShaderWriter::~AtslShaderWriter()
{
}

void AtslShaderWriter::visit(const ConditionalStatement& statement)
{
	bool isFirst = true;

	for (const auto& branch : statement.branches)
	{
		if (!isFirst)
		{
			newLine();
			m_ostream << "else ";
		}

		m_ostream << "if (";

		branch.condition->accept(*this);

		m_ostream << ")";

		beginBlock();

		branch.statement->accept(*this);

		endBlock();

		isFirst = false;
	}

	if (statement.elseStatement)
	{
		newLine();
		m_ostream << "else";

		beginBlock();

		statement.elseStatement->accept(*this);

		endBlock();
	}
}

void AtslShaderWriter::visit(const ForLoopStatement& statement)
{
	m_ostream << "for (";

	if (statement.initialization)
		statement.initialization->accept(*this);
	else
		m_ostream << ";";

	m_ostream << " ";

	if (statement.condition)
		statement.condition->accept(*this);

	m_ostream << "; ";

	if (statement.increase)
		statement.increase->accept(*this);

	m_ostream << ")";

	beginBlock();

	statement.statement->accept(*this);

	endBlock();
}

void AtslShaderWriter::visit(const WhileLoopStatement& statement)
{
	m_ostream << "while (";

	statement.condition->accept(*this);

	m_ostream << ")";

	beginBlock();

	statement.statement->accept(*this);

	endBlock();
}

void AtslShaderWriter::visit(const DoWhileLoopStatement& statement)
{
	m_ostream << "do";

	beginBlock();

	statement.statement->accept(*this);

	endBlock();

	m_ostream << " while (";

	statement.condition->accept(*this);

	m_ostream << ")";

	addDelimiter();
}

void AtslShaderWriter::visit(const VariableDeclarationStatement& statement)
{
	if (statement.qualifiers & VariableQualifier::Const)
		m_ostream << "const ";

	writeVariableDeclaration(statement.type, statement.name, statement.value.get());
}

void AtslShaderWriter::visit(const StructDeclarationStatement& statement)
{
	m_ostream << "struct " << statement.name;

	beginBlock();

	std::vector<Attribute> attributes =
	{
		{"conditional", AtslBasicValue(0)}
	};

	for (auto& member : statement.members)
	{
		if (member.condition)
		{
			attributes[0].value = member.condition;

			writeAttributes(attributes);
		}

		writeVariableDeclaration(member.type, member.name);

		// Add line except for last element
		if (&member != &statement.members.back())
			newLine();
	}

	endBlock();
}

void AtslShaderWriter::visit(const InputDeclarationStatement& statement)
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
	for (auto& variable : statement.variables)
	{
		std::vector<Attribute> variableAttributes =
		{
			{ "location", variable.location }
		};

		if (variable.condition)
		{
			Attribute layoutAttribute;
			layoutAttribute.name = "conditional";
			layoutAttribute.value = variable.condition;

			variableAttributes.emplace_back(std::move(layoutAttribute));
		}

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

void AtslShaderWriter::visit(const OutputDeclarationStatement& statement)
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
	for (auto& variable : statement.variables)
	{
		std::vector<Attribute> variableAttributes =
		{
			{ "location", variable.location }
		};

		if (variable.condition)
		{
			Attribute layoutAttribute;
			layoutAttribute.name = "conditional";
			layoutAttribute.value = variable.condition;

			variableAttributes.emplace_back(std::move(layoutAttribute));
		}

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

void AtslShaderWriter::visit(const ExternalDeclarationStatement& statement)
{
	// Begin block
	m_ostream << "external";

	if (statement.variables.size() != 1)
		beginBlock();
	else
		m_ostream << " ";

	// Write variables
	for (auto& variable : statement.variables)
	{
		std::vector<Attribute> variableAttributes =
		{
			{ "set", variable.setIndex },
			{ "binding", variable.bindingIndex }
		};

		if (variable.type.is<StructType>())
		{
			Attribute layoutAttribute;
			layoutAttribute.name = "layout";
			layoutAttribute.value = atsl::getStructLayoutStr(variable.structLayout);

			variableAttributes.emplace_back(std::move(layoutAttribute));
		}

		if (variable.condition)
		{
			Attribute layoutAttribute;
			layoutAttribute.name = "conditional";
			layoutAttribute.value = variable.condition;

			variableAttributes.emplace_back(std::move(layoutAttribute));
		}

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

void AtslShaderWriter::visit(const OptionDeclarationStatement& statement)
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

void AtslShaderWriter::visit(const FunctionDeclarationStatement& statement)
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

	// Function definition
	if (statement.sequence)
	{
		beginBlock();

		statement.sequence->accept(*this);

		endBlock();
	}
	// Declaration only
	else
	{
		addDelimiter();

		newLine();
	}
}

void AtslShaderWriter::visit(const EntryFunctionDeclarationStatement& statement)
{
	const std::vector<Attribute> attributes =
	{
		{"entry", AtslIdentifier(atsl::getShaderStageStr(statement.stage))}
	};

	writeAttributes(attributes);

	newLine();

	visit(static_cast<const FunctionDeclarationStatement&>(statement));
}

void AtslShaderWriter::visit(const ExpressionStatement& statement)
{
	statement.expression->accept(*this);

	addDelimiter();
}

void AtslShaderWriter::visit(const BreakStatement& statement)
{
	m_ostream << "break";

	addDelimiter();
}

void AtslShaderWriter::visit(const ContinueStatement& statement)
{
	m_ostream << "continue";

	addDelimiter();
}

void AtslShaderWriter::visit(const ReturnStatement& statement)
{
	m_ostream << "return";

	if (statement.expression)
	{
		m_ostream << " ";

		statement.expression->accept(*this);
	}

	addDelimiter();
}

void AtslShaderWriter::visit(const DiscardStatement& statement)
{
	m_ostream << "discard";

	addDelimiter();
}

void AtslShaderWriter::visit(const SequenceStatement& statement)
{
	for (auto& subStatement : statement.statements)
	{
		subStatement->accept(*this);

		// Add line except for last element
		if (&subStatement != &statement.statements.back())
		{
			newLine();
			newLine(); // Add 2 lines to make code better to read
		}
	}
}

void AtslShaderWriter::visit(const OptionalStatement& statement)
{
	m_ostream << "optional (";

	statement.condition->accept(*this);

	m_ostream << ")";

	if (statement.statement->getType() == Statement::Type::Sequence)
	{
		beginBlock();

		statement.statement->accept(*this);

		endBlock();
	}
	else
	{
		indent();

		newLine();

		statement.statement->accept(*this);

		unindent();
	}
}

void AtslShaderWriter::visit(const IncludeStatement& statement)
{
	m_ostream << "include";

	if (statement.libraries.size() == 1)
	{
		m_ostream << " " << statement.libraries[0];

		addDelimiter();
	}
	else
	{
		beginBlock();

		for (size_t i = 0; i < statement.libraries.size(); i++)
		{
			m_ostream << statement.libraries[i];

			addDelimiter();

			if (i != statement.libraries.size() - 1)
				newLine();
		}

		endBlock();
	}
}

void AtslShaderWriter::visit(const ConstantExpression& expression)
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

		m_ostream << "vec4u(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
	}
	else if (value.is<Vector4f>())
	{
		auto& vec = value.get<Vector4f>();

		m_ostream << "vec4f(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
	}
	else
	{
		ATEMA_ERROR("Invalid constant type");
	}
}

void AtslShaderWriter::visit(const VariableExpression& expression)
{
	m_ostream << expression.identifier;
}

void AtslShaderWriter::visit(const AccessIndexExpression& expression)
{
	expression.expression->accept(*this);

	m_ostream << "[";

	expression.index->accept(*this);

	m_ostream << "]";
}

void AtslShaderWriter::visit(const AccessIdentifierExpression& expression)
{
	// Check if we need to protect the expression with parenthesis
	if (expression.expression->getType() == Expression::Type::Binary)
	{
		m_ostream << "(";

		expression.expression->accept(*this);

		m_ostream << ")";
	}
	else
	{
		expression.expression->accept(*this);
	}

	m_ostream << "." << expression.identifier;
}

void AtslShaderWriter::visit(const AssignmentExpression& expression)
{
	expression.left->accept(*this);

	m_ostream << " = ";

	expression.right->accept(*this);
}

void AtslShaderWriter::visit(const UnaryExpression& expression)
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

	expression.operand->accept(*this);

	if (!suffix.empty())
		m_ostream << suffix;
}

void AtslShaderWriter::visit(const BinaryExpression& expression)
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

		expression.left->accept(*this);

		if (precedence < currentPrecedence)
			m_ostream << ")";
	}
	else
	{
		expression.left->accept(*this);
	}

	m_ostream << " " << operatorStr << " ";

	// Check if we need to protect the operand with parenthesis
	if (expression.right->getType() == Expression::Type::Binary)
	{
		const auto op = static_cast<BinaryExpression&>(*expression.right).op;
		auto precedence = getOperatorPrecedence(op);

		if (expression.op == BinaryOperator::Subtract && precedence == currentPrecedence)
			precedence--;

		if (precedence < currentPrecedence)
			m_ostream << "(";

		expression.right->accept(*this);

		if (precedence < currentPrecedence)
			m_ostream << ")";
	}
	else
	{
		expression.right->accept(*this);
	}
}

void AtslShaderWriter::visit(const FunctionCallExpression& expression)
{
	writeFunctionCall(expression.identifier, expression.arguments);
}

void AtslShaderWriter::visit(const BuiltInFunctionCallExpression& expression)
{
	writeFunctionCall(atsl::getBuiltInFunctionStr(expression.function), expression.arguments);
}

void AtslShaderWriter::visit(const CastExpression& expression)
{
	writeType(expression.type);

	m_ostream << "(";

	for (auto& component : expression.components)
	{
		component->accept(*this);

		// Add comma except for last element
		if (&component != &expression.components.back())
			m_ostream << ", ";
	}

	m_ostream << ")";
}

void AtslShaderWriter::visit(const SwizzleExpression& expression)
{
	expression.expression->accept(*this);

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

void AtslShaderWriter::visit(const TernaryExpression& expression)
{
	m_ostream << "(";

	expression.condition->accept(*this);

	m_ostream << " ? ";

	expression.trueValue->accept(*this);

	m_ostream << " : ";

	expression.falseValue->accept(*this);

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

void AtslShaderWriter::writeType(ArrayType::ComponentType type)
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
		else if (attribute.value.is<Ptr<Expression>>())
		{
			attribute.value.get<Ptr<Expression>>()->accept(*this);
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

void AtslShaderWriter::writeVariableDeclaration(Type type, std::string name, Expression* value)
{
	if (type.is<ArrayType>())
		writeType(type.get<ArrayType>().componentType);
	else
		writeType(type);

	m_ostream << " " << name;

	if (type.is<ArrayType>())
	{
		const auto& arrayType = type.get<ArrayType>();

		if (arrayType.sizeType == ArrayType::SizeType::Implicit)
			ATEMA_ERROR("Array size must be specified");

		m_ostream << "[" << atsl::getArraySizeStr(arrayType) << "]";
	}

	if (value)
	{
		m_ostream << " = ";

		value->accept(*this);
	}

	addDelimiter();
}

void AtslShaderWriter::writeFunctionCall(const std::string& functionName, const std::vector<UPtr<Expression>>& arguments)
{
	m_ostream << functionName << "(";

	// Add comma except for last element
	for (auto& argument : arguments)
	{
		argument->accept(*this);

		if (&argument != &arguments.back())
			m_ostream << ", ";
	}

	m_ostream << ")";
}
