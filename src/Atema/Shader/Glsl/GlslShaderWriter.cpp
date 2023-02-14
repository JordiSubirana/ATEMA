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

#include <Atema/Shader/Ast/AstReflector.hpp>
#include <Atema/Shader/Glsl/GlslShaderWriter.hpp>
#include <Atema/Shader/Ast/AstUtils.hpp>
#include <Atema/Shader/Glsl/GlslUtils.hpp>

using namespace at;

namespace
{
	std::string getInterfaceBlockSuffix(AstShaderStage stage)
	{
		switch (stage)
		{
			case AstShaderStage::Vertex: return "VS";
			case AstShaderStage::Fragment: return "FS";
			default:
			{
				ATEMA_ERROR("Invalid shader stage");
			}
		}

		return "";
	}
}

GlslShaderWriter::GlslShaderWriter(std::ostream& ostream, Settings settings) :
	m_isSequenceProcessed(false),
	m_stage(settings.stage),
	m_ostream(ostream),
	m_settings(settings),
	m_indent(0)
{
}

GlslShaderWriter::~GlslShaderWriter()
{
}

void GlslShaderWriter::visit(const ConditionalStatement& statement)
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

void GlslShaderWriter::visit(const ForLoopStatement& statement)
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

void GlslShaderWriter::visit(const WhileLoopStatement& statement)
{
	m_ostream << "while (";

	statement.condition->accept(*this);

	m_ostream << ")";

	beginBlock();

	statement.statement->accept(*this);

	endBlock();
}

void GlslShaderWriter::visit(const DoWhileLoopStatement& statement)
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

void GlslShaderWriter::visit(const VariableDeclarationStatement& statement)
{
	if (statement.qualifiers & VariableQualifier::Const)
		m_ostream << "const ";

	writeVariableDeclaration(statement.type, statement.name, statement.value.get());
}

void GlslShaderWriter::visit(const StructDeclarationStatement& statement)
{
	m_ostream << "struct " << statement.name;

	beginBlock();

	for (auto& member : statement.members)
	{
		if (member.condition)
		{
			preprocessorIf(*member.condition);

			newLine();
		}

		writeVariableDeclaration(member.type, member.name);

		if (member.condition)
		{
			newLine();

			preprocessorEndif();
		}

		// Add line except for last element
		if (&member != &statement.members.back())
			newLine();
	}

	endBlock();

	addDelimiter();

	// Save struct declaration for uniform interface block declaration
	{
		AstCloner cloner;
		m_structDeclarations[statement.name] = cloner.clone(statement);
	}
}

void GlslShaderWriter::visit(const InputDeclarationStatement& statement)
{
	for (auto& variable : statement.variables)
	{
		if (variable.condition)
		{
			preprocessorIf(*variable.condition);

			newLine();
		}

		writeLayout(variable.location);
		
		m_ostream << " in ";

		if (variable.type.is<StructType>())
		{
			const auto suffix = "I" + getInterfaceBlockSuffix(statement.stage);

			writeInterfaceBlock(variable.type.get<StructType>().name, variable.name, suffix);

			// Add line except for last element
			if (&variable != &statement.variables.back())
				newLine();
		}
		else
		{
			writeVariableDeclaration(variable.type, variable.name);
		}

		if (variable.condition)
		{
			newLine();

			preprocessorEndif();
		}

		// Add line except for last element
		if (&variable != &statement.variables.back())
			newLine();
	}
}

void GlslShaderWriter::visit(const OutputDeclarationStatement& statement)
{
	for (auto& variable : statement.variables)
	{
		if (variable.condition)
		{
			preprocessorIf(*variable.condition);

			newLine();
		}

		writeLayout(variable.location);

		m_ostream << " out ";

		if (variable.type.is<StructType>())
		{
			const auto suffix = "O" + getInterfaceBlockSuffix(statement.stage);

			writeInterfaceBlock(variable.type.get<StructType>().name, variable.name, suffix);

			// Add line except for last element
			if (&variable != &statement.variables.back())
				newLine();
		}
		else
		{
			writeVariableDeclaration(variable.type, variable.name);
		}

		if (variable.condition)
		{
			newLine();

			preprocessorEndif();
		}

		// Add line except for last element
		if (&variable != &statement.variables.back())
			newLine();
	}
}

void GlslShaderWriter::visit(const ExternalDeclarationStatement& statement)
{
	for (auto& variable : statement.variables)
	{
		if (variable.condition)
		{
			preprocessorIf(*variable.condition);

			newLine();
		}

		if (variable.type.is<StructType>())
		{
			writeLayout(variable.setIndex, variable.bindingIndex, variable.structLayout);

			m_ostream << " uniform ";

			writeInterfaceBlock(variable.type.get<StructType>().name, variable.name, "U");

			// Add line except for last element
			if (&variable != &statement.variables.back())
				newLine();
		}
		else
		{
			writeLayout(variable.setIndex, variable.bindingIndex);

			m_ostream << " uniform ";

			writeVariableDeclaration(variable.type, variable.name);
		}

		if (variable.condition)
		{
			newLine();

			preprocessorEndif();
		}

		// Add line except for last element
		if (&variable != &statement.variables.back())
			newLine();
	}
}

void GlslShaderWriter::visit(const OptionDeclarationStatement& statement)
{
	for (auto& variable : statement.variables)
	{
		m_ostream << "#define " << variable.name;

		if (variable.value)
		{
			m_ostream << " ";

			variable.value->accept(*this);
		}

		// Add line except for last element
		if (&variable != &statement.variables.back())
			newLine();
	}
}

void GlslShaderWriter::visit(const FunctionDeclarationStatement& statement)
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
		statement.sequence->accept(*this);

	endBlock();
}

void GlslShaderWriter::visit(const EntryFunctionDeclarationStatement& statement)
{
	m_ostream << "void main()";

	beginBlock();

	if (statement.sequence)
		statement.sequence->accept(*this);

	endBlock();
}

void GlslShaderWriter::visit(const ExpressionStatement& statement)
{
	statement.expression->accept(*this);

	addDelimiter();
}

void GlslShaderWriter::visit(const BreakStatement& statement)
{
	m_ostream << "break";

	addDelimiter();
}

void GlslShaderWriter::visit(const ContinueStatement& statement)
{
	m_ostream << "continue";

	addDelimiter();
}

void GlslShaderWriter::visit(const ReturnStatement& statement)
{
	m_ostream << "return";

	if (statement.expression)
	{
		m_ostream << " ";

		statement.expression->accept(*this);
	}

	addDelimiter();
}

void GlslShaderWriter::visit(const DiscardStatement& statement)
{
	m_ostream << "discard";

	addDelimiter();
}

void GlslShaderWriter::visit(const SequenceStatement& statement)
{
	if (!m_isSequenceProcessed)
	{
		// Write header
		writeHeader();

		newLine();

		// Then write the desired shader stage
		m_isSequenceProcessed = true;

		if (m_stage.has_value())
		{
			// Extract desired stage from current sequence (when first calling this method)
			AstReflector reflector;

			statement.accept(reflector);

			const auto processedStage = reflector.getAst(m_stage.value());

			processedStage->accept(*this);
		}
		else
		{
			statement.accept(*this);
		}

		m_isSequenceProcessed = false;
	}
	else
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
}

void GlslShaderWriter::visit(const OptionalStatement& statement)
{
	preprocessorIf(*statement.condition);

	newLine();

	statement.statement->accept(*this);

	newLine();

	preprocessorEndif();
}

void GlslShaderWriter::visit(const IncludeStatement& statement)
{
	ATEMA_ERROR("GLSL writer does not support include statements, AST should have been preprocessed");
}

void GlslShaderWriter::visit(const ConstantExpression& expression)
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

		m_ostream << "ivec2(" << vec.x << ", " << vec.y << ")";
	}
	else if (value.is<Vector2u>())
	{
		auto& vec = value.get<Vector2u>();

		m_ostream << "uvec2(" << vec.x << ", " << vec.y << ")";
	}
	else if (value.is<Vector2f>())
	{
		auto& vec = value.get<Vector2f>();

		m_ostream << "vec2(" << vec.x << ", " << vec.y << ")";
	}
	else if (value.is<Vector3i>())
	{
		auto& vec = value.get<Vector3i>();

		m_ostream << "ivec3(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
	}
	else if (value.is<Vector3u>())
	{
		auto& vec = value.get<Vector3u>();

		m_ostream << "uvec3(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
	}
	else if (value.is<Vector3f>())
	{
		auto& vec = value.get<Vector3f>();

		m_ostream << "vec3(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
	}
	else if (value.is<Vector4i>())
	{
		auto& vec = value.get<Vector4i>();

		m_ostream << "ivec4(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
	}
	else if (value.is<Vector4u>())
	{
		auto& vec = value.get<Vector4u>();

		m_ostream << "uvec4(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
	}
	else if (value.is<Vector4f>())
	{
		auto& vec = value.get<Vector4f>();

		m_ostream << "vec4(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
	}
	else
	{
		ATEMA_ERROR("Invalid constant type");
	}
}

void GlslShaderWriter::visit(const VariableExpression& expression)
{
	m_ostream << expression.identifier;
}

void GlslShaderWriter::visit(const AccessIndexExpression& expression)
{
	expression.expression->accept(*this);

	m_ostream << "[";

	expression.index->accept(*this);

	m_ostream << "]";
}

void GlslShaderWriter::visit(const AccessIdentifierExpression& expression)
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

void GlslShaderWriter::visit(const AssignmentExpression& expression)
{
	expression.left->accept(*this);

	m_ostream << " = ";

	expression.right->accept(*this);
}

void GlslShaderWriter::visit(const UnaryExpression& expression)
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

void GlslShaderWriter::visit(const BinaryExpression& expression)
{
	std::string operatorStr;

	switch (expression.op)
	{
		// Specific case
		case BinaryOperator::Power:
		{
			m_ostream << "pow(";

			expression.left->accept(*this);

			m_ostream << ", ";

			expression.right->accept(*this);

			m_ostream << ")";

			return;
		}
		case BinaryOperator::Modulo:
		{
			m_ostream << "mod(";

			expression.left->accept(*this);

			m_ostream << ", ";

			expression.right->accept(*this);

			m_ostream << ")";

			return;
		}
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

void GlslShaderWriter::visit(const FunctionCallExpression& expression)
{
	writeFunctionCall(expression.identifier, expression.arguments);
}

void GlslShaderWriter::visit(const BuiltInFunctionCallExpression& expression)
{
	//TODO: In the future some arguments positions may change

	// Specific cases
	if (expression.function == BuiltInFunction::SetVertexPosition)
	{
		m_ostream << "gl_Position = ";

		expression.arguments[0]->accept(*this);
	}
	// Classic function calls
	else
	{
		std::string functionName;

		switch (expression.function)
		{
			case BuiltInFunction::Min:
			{
				functionName = "min";
				break;
			}
			case BuiltInFunction::Max:
			{
				functionName = "max";
				break;
			}
			case BuiltInFunction::Cross:
			{
				functionName = "cross";
				break;
			}
			case BuiltInFunction::Dot:
			{
				functionName = "dot";
				break;
			}
			case BuiltInFunction::Norm:
			{
				functionName = "length";
				break;
			}
			case BuiltInFunction::Normalize:
			{
				functionName = "normalize";
				break;
			}
			case BuiltInFunction::Sample:
			{
				functionName = "texture";
				break;
			}
			default:
			{
				ATEMA_ERROR("Invalid built-in function");
			}
		}

		writeFunctionCall(functionName, expression.arguments);
	}
}

void GlslShaderWriter::visit(const CastExpression& expression)
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

void GlslShaderWriter::visit(const SwizzleExpression& expression)
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

void GlslShaderWriter::visit(const TernaryExpression& expression)
{
	m_ostream << "(";

	expression.condition->accept(*this);

	m_ostream << " ? ";

	expression.trueValue->accept(*this);

	m_ostream << " : ";

	expression.falseValue->accept(*this);

	m_ostream << ")";
}

void GlslShaderWriter::newLine()
{
	m_ostream << std::endl;

	for (int i = 0; i < m_indent; i++)
		m_ostream << "\t";
}

void GlslShaderWriter::indent()
{
	m_indent++;
}

void GlslShaderWriter::unindent()
{
	m_indent--;
}

void GlslShaderWriter::beginBlock()
{
	newLine();

	m_ostream << "{";

	indent();

	newLine();
}

void GlslShaderWriter::endBlock()
{
	unindent();

	newLine();

	m_ostream << "}";
}

void GlslShaderWriter::addDelimiter()
{
	m_ostream << ";";
}

void GlslShaderWriter::writeHeader()
{
	// Version
	const auto version = m_settings.versionMajor * 100 + m_settings.versionMinor * 10;

	m_ostream << "#version " << version;

	newLine();

	// Extensions
	std::vector<std::string> extensions;

	if (version < 420)
		extensions.push_back("GL_ARB_shading_language_420pack"); // layout(binding = XXX)

	if (version < 410)
		extensions.push_back("GL_ARB_separate_shader_objects"); // layout(location = XXX)

	for (auto& extension : extensions)
	{
		m_ostream << "#extension " << extension << " : require";

		// Add line except for last element
		if (&extension != &extensions.back())
			newLine();
	}

	newLine();
}

void GlslShaderWriter::writeLayout(const Ptr<Expression>& location)
{
	m_ostream << "layout(location = ";
	
	location->accept(*this);

	m_ostream << ")";
}

void GlslShaderWriter::writeLayout(const Ptr<Expression>& set, const Ptr<Expression>& binding)
{
	m_ostream << "layout(set = ";
	
	set->accept(*this);

	m_ostream << ", binding = ";
	
	binding->accept(*this);

	m_ostream << ")";
}

void GlslShaderWriter::writeLayout(const Ptr<Expression>& set, const Ptr<Expression>& binding, StructLayout structLayout)
{
	m_ostream << "layout(set = ";

	set->accept(*this);

	m_ostream << ", binding = ";

	binding->accept(*this);

	m_ostream << ", " << glsl::getStructLayoutStr(structLayout) << ")";
}

void GlslShaderWriter::writeType(Type type)
{
	m_ostream << glsl::getTypeStr(type);
}

void GlslShaderWriter::writeType(ArrayType::ComponentType type)
{
	m_ostream << glsl::getTypeStr(type);
}

void GlslShaderWriter::writeVariableDeclaration(Type type, std::string name, Expression* value)
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

		m_ostream << "[" << glsl::getArraySizeStr(arrayType) << "]";
	}

	if (value)
	{
		m_ostream << " = ";

		value->accept(*this);
	}

	addDelimiter();
}

void GlslShaderWriter::writeFunctionCall(const std::string& functionName, const std::vector<UPtr<Expression>>& arguments)
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

void GlslShaderWriter::writeInterfaceBlock(const std::string& blockName, const std::string& instanceName, const std::string& suffix)
{
	auto structIt = m_structDeclarations.find(blockName);

	if (structIt == m_structDeclarations.end())
	{
		ATEMA_ERROR("Struct '" + blockName + "' is not defined");
	}

	const auto interfaceName = blockName + "_" + suffix;

	auto& count = m_interfaceBlockCount.try_emplace(interfaceName, 0).first->second;

	m_ostream << interfaceName << count++;

	auto& statement = *(structIt->second);

	beginBlock();

	for (auto& member : statement.members)
	{
		if (member.condition)
		{
			preprocessorIf(*member.condition);

			newLine();
		}

		writeVariableDeclaration(member.type, member.name);

		if (member.condition)
		{
			newLine();

			preprocessorEndif();
		}

		// Add line except for last element
		if (&member != &statement.members.back())
			newLine();
	}

	endBlock();

	m_ostream << " " << instanceName;

	addDelimiter();
}

void GlslShaderWriter::preprocessorIf(Expression& expression)
{
	m_ostream << "#if (";
	
	expression.accept(*this);

	m_ostream << ")";
}

void GlslShaderWriter::preprocessorEndif()
{
	m_ostream << "#endif";
}
