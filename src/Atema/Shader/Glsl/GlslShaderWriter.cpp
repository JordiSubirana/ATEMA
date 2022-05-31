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

#include <Atema/Shader/Ast/AstStageExtractor.hpp>
#include <Atema/Shader/Glsl/GlslShaderWriter.hpp>
#include <Atema/Shader/Ast/AstUtils.hpp>

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

GlslShaderWriter::GlslShaderWriter(AstShaderStage stage, std::ostream& ostream, Settings settings) :
	m_isSequenceProcessed(false),
	m_stage(stage),
	m_ostream(ostream),
	m_settings(settings),
	m_indent(0)
{
}

GlslShaderWriter::~GlslShaderWriter()
{
}

void GlslShaderWriter::visit(ConditionalStatement& statement)
{
	for (auto& branch : statement.branches)
	{
		m_ostream << "if (";

		branch.condition->accept(*this);

		m_ostream << ")";

		beginBlock();

		branch.statement->accept(*this);

		endBlock();
	}
}

void GlslShaderWriter::visit(ForLoopStatement& statement)
{
	m_ostream << "for (";

	if (statement.initialization)
		statement.initialization->accept(*this);

	m_ostream << "; ";

	if (statement.condition)
		statement.condition->accept(*this);

	m_ostream << "; ";

	if (statement.increase)
		statement.increase->accept(*this);

	beginBlock();

	statement.statement->accept(*this);

	endBlock();
}

void GlslShaderWriter::visit(WhileLoopStatement& statement)
{
	m_ostream << "while (";

	statement.condition->accept(*this);

	m_ostream << ")";

	beginBlock();

	statement.statement->accept(*this);

	endBlock();
}

void GlslShaderWriter::visit(DoWhileLoopStatement& statement)
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

void GlslShaderWriter::visit(VariableDeclarationStatement& statement)
{
	if (statement.qualifiers & VariableQualifier::Const)
		m_ostream << "const ";

	writeVariableDeclaration(statement.type, statement.name, statement.value.get());
}

void GlslShaderWriter::visit(StructDeclarationStatement& statement)
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

	addDelimiter();

	// Save struct declaration for uniform interface block declaration
	if (!m_settings.allowLegacyUniformStructs)
	{
		AstCloner cloner;
		m_structDeclarations[statement.name] = cloner.clone(statement);
	}
}

void GlslShaderWriter::visit(InputDeclarationStatement& statement)
{
	for (auto& variable : statement.variables)
	{
		writeLayout(variable.location);
		
		m_ostream << " in ";

		if (variable.type.is<StructType>() && !m_settings.allowLegacyUniformStructs)
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

		// Add line except for last element
		if (&variable != &statement.variables.back())
			newLine();
	}
}

void GlslShaderWriter::visit(OutputDeclarationStatement& statement)
{
	for (auto& variable : statement.variables)
	{
		writeLayout(variable.location);

		m_ostream << " out ";

		if (variable.type.is<StructType>() && !m_settings.allowLegacyUniformStructs)
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

		// Add line except for last element
		if (&variable != &statement.variables.back())
			newLine();
	}
}

void GlslShaderWriter::visit(ExternalDeclarationStatement& statement)
{
	for (auto& variable : statement.variables)
	{
		writeLayout(variable.setIndex, variable.bindingIndex);

		m_ostream << " uniform ";

		if (variable.type.is<StructType>() && !m_settings.allowLegacyUniformStructs)
		{
			writeInterfaceBlock(variable.type.get<StructType>().name, variable.name, "U");

			// Add line except for last element
			if (&variable != &statement.variables.back())
				newLine();
		}
		else
		{
			writeVariableDeclaration(variable.type, variable.name);
		}

		// Add line except for last element
		if (&variable != &statement.variables.back())
			newLine();
	}
}

void GlslShaderWriter::visit(OptionDeclarationStatement& statement)
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

void GlslShaderWriter::visit(FunctionDeclarationStatement& statement)
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

void GlslShaderWriter::visit(EntryFunctionDeclarationStatement& statement)
{
	m_ostream << "void main()";

	beginBlock();

	if (statement.sequence)
		statement.sequence->accept(*this);

	endBlock();
}

void GlslShaderWriter::visit(ExpressionStatement& statement)
{
	statement.expression->accept(*this);

	addDelimiter();
}

void GlslShaderWriter::visit(BreakStatement& statement)
{
	m_ostream << "break";

	addDelimiter();
}

void GlslShaderWriter::visit(ContinueStatement& statement)
{
	m_ostream << "continue";

	addDelimiter();
}

void GlslShaderWriter::visit(ReturnStatement& statement)
{
	m_ostream << "return";

	if (statement.expression)
	{
		m_ostream << " ";

		statement.expression->accept(*this);
	}

	addDelimiter();
}

void GlslShaderWriter::visit(SequenceStatement& statement)
{
	if (!m_isSequenceProcessed)
	{
		// Extract desired stage from current sequence (when first calling this method)
		AstStageExtractor stageExtractor;

		statement.accept(stageExtractor);

		const auto processedStage = stageExtractor.getAst(m_stage);

		// Write header
		writeHeader();

		newLine();

		// Then write the desired shader stage
		m_isSequenceProcessed = true;

		processedStage->accept(*this);

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

void GlslShaderWriter::visit(ConstantExpression& expression)
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

void GlslShaderWriter::visit(VariableExpression& expression)
{
	m_ostream << expression.identifier;
}

void GlslShaderWriter::visit(AccessIndexExpression& expression)
{
	expression.expression->accept(*this);

	m_ostream << "[";

	expression.index->accept(*this);

	m_ostream << "]";
}

void GlslShaderWriter::visit(AccessIdentifierExpression& expression)
{
	expression.expression->accept(*this);

	m_ostream << "." << expression.identifier;
}

void GlslShaderWriter::visit(AssignmentExpression& expression)
{
	expression.left->accept(*this);

	m_ostream << " = ";

	expression.right->accept(*this);
}

void GlslShaderWriter::visit(UnaryExpression& expression)
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

void GlslShaderWriter::visit(BinaryExpression& expression)
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
		const auto precedence = getOperatorPrecedence(op);

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

void GlslShaderWriter::visit(FunctionCallExpression& expression)
{
	writeFunctionCall(expression.identifier, expression.arguments);
}

void GlslShaderWriter::visit(BuiltInFunctionCallExpression& expression)
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

void GlslShaderWriter::visit(CastExpression& expression)
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

void GlslShaderWriter::visit(SwizzleExpression& expression)
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

void GlslShaderWriter::visit(TernaryExpression& expression)
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

void GlslShaderWriter::writeLayout(unsigned location)
{
	m_ostream << "layout(location = " << location << ")";
}

void GlslShaderWriter::writeLayout(unsigned set, unsigned binding)
{
	m_ostream << "layout(set = " << set << ", binding = " << binding << ")";
}

void GlslShaderWriter::writeType(Type type)
{
	if (type.is<VoidType>())
	{
		m_ostream << "void";
	}
	else if (type.is<PrimitiveType>())
	{
		switch (type.get<PrimitiveType>())
		{
			case PrimitiveType::Bool:
			{
				m_ostream << "bool";
				break;
			}
			case PrimitiveType::Int:
			{
				m_ostream << "int";
				break;
			}
			case PrimitiveType::UInt:
			{
				m_ostream << "uint";
				break;
			}
			case PrimitiveType::Float:
			{
				m_ostream << "float";
				break;
			}
			default:
			{
				ATEMA_ERROR("Invalid primitive type");
			}
		}
	}
	else if (type.is<VectorType>())
	{
		const auto& data = type.get<VectorType>();

		switch (data.primitiveType)
		{
			case PrimitiveType::Bool:
			{
				m_ostream << "b";
				break;
			}
			case PrimitiveType::Int:
			{
				m_ostream << "i";
				break;
			}
			case PrimitiveType::UInt:
			{
				m_ostream << "u";
				break;
			}
			case PrimitiveType::Float:
			{
				break;
			}
			default:
			{
				ATEMA_ERROR("Invalid vector primitive type");
			}
		}

		m_ostream << "vec" << std::to_string(data.componentCount);
	}
	else if (type.is<MatrixType>())
	{
		const auto& data = type.get<MatrixType>();

		ATEMA_ASSERT(data.primitiveType == PrimitiveType::Float, "GLSL only handles float matrices");

		m_ostream << "mat" << std::to_string(data.rowCount);

		if (data.rowCount != data.columnCount)
			m_ostream << "x" + std::to_string(data.columnCount);
	}
	else if (type.is<SamplerType>())
	{
		const auto& data = type.get<SamplerType>();

		switch (data.primitiveType)
		{
			case PrimitiveType::Bool:
			{
				ATEMA_ERROR("GLSL does not support boolean samplers");
				break;
			}
			case PrimitiveType::Int:
			{
				m_ostream << "i";
				break;
			}
			case PrimitiveType::UInt:
			{
				m_ostream << "u";
				break;
			}
			case PrimitiveType::Float:
			{
				break;
			}
			default:
			{
				ATEMA_ERROR("Invalid primitive type");
			}
		}

		m_ostream << "sampler";

		switch (data.imageType)
		{
			case ImageType::Texture1D:
			{
				m_ostream << "1D";
				break;
			}
			case ImageType::Texture2D:
			{
				m_ostream << "2D";
				break;
			}
			case ImageType::Texture3D:
			{
				m_ostream << "3D";
				break;
			}
			case ImageType::Cubemap:
			{
				m_ostream << "Cube";
				break;
			}
			default:
			{
				ATEMA_ERROR("Invalid image type");
			}
		}
	}
	else if (type.is<StructType>())
	{
		m_ostream << type.get<StructType>().name;
	}
	else
	{
		ATEMA_ERROR("Invalid type");
	}
}

void GlslShaderWriter::writeVariableDeclaration(Type type, std::string name, Expression* value)
{
	writeType(type);

	m_ostream << " " << name;

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
		writeVariableDeclaration(member.type, member.name);

		// Add line except for last element
		if (&member != &statement.members.back())
			newLine();
	}

	endBlock();

	m_ostream << " " << instanceName;

	addDelimiter();
}
