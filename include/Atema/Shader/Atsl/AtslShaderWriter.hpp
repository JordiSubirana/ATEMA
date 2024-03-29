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

#ifndef ATEMA_SHADER_ATSLSHADERWRITER_HPP
#define ATEMA_SHADER_ATSLSHADERWRITER_HPP

#include <Atema/Shader/Config.hpp>
#include <Atema/Shader/ShaderWriter.hpp>
#include <Atema/Shader/Atsl/AtslToken.hpp>

#include <ostream>


namespace at
{
	class ATEMA_SHADER_API AtslShaderWriter : public ShaderWriter
	{
	public:
		AtslShaderWriter() = delete;
		AtslShaderWriter(std::ostream& ostream);
		~AtslShaderWriter();

		void visit(const ConditionalStatement& statement) override;
		void visit(const ForLoopStatement& statement) override;
		void visit(const WhileLoopStatement& statement) override;
		void visit(const DoWhileLoopStatement& statement) override;
		void visit(const VariableDeclarationStatement& statement) override;
		void visit(const StructDeclarationStatement& statement) override;
		void visit(const InputDeclarationStatement& statement) override;
		void visit(const OutputDeclarationStatement& statement) override;
		void visit(const ExternalDeclarationStatement& statement) override;
		void visit(const OptionDeclarationStatement& statement) override;
		void visit(const FunctionDeclarationStatement& statement) override;
		void visit(const EntryFunctionDeclarationStatement& statement) override;
		void visit(const ExpressionStatement& statement) override;
		void visit(const BreakStatement& statement) override;
		void visit(const ContinueStatement& statement) override;
		void visit(const ReturnStatement& statement) override;
		void visit(const DiscardStatement& statement) override;
		void visit(const SequenceStatement& statement) override;
		void visit(const OptionalStatement& statement) override;
		void visit(const IncludeStatement& statement) override;

		void visit(const ConstantExpression& expression) override;
		void visit(const VariableExpression& expression) override;
		void visit(const AccessIndexExpression& expression) override;
		void visit(const AccessIdentifierExpression& expression) override;
		void visit(const AssignmentExpression& expression) override;
		void visit(const UnaryExpression& expression) override;
		void visit(const BinaryExpression& expression) override;
		void visit(const FunctionCallExpression& expression) override;
		void visit(const BuiltInFunctionCallExpression& expression) override;
		void visit(const CastExpression& expression) override;
		void visit(const SwizzleExpression& expression) override;
		void visit(const TernaryExpression& expression) override;

	private:
		struct Attribute
		{
			std::string name;
			Variant<AtslBasicValue, AtslIdentifier, Ptr<Expression>> value;
		};

		void newLine();

		void indent();
		void unindent();

		void beginBlock();
		void endBlock();

		void addDelimiter();

		void writeType(AstType type);
		void writeType(AstArrayType::ComponentType type);
		void writeAttributes(const std::vector<Attribute>& attributes);
		void writeVariableDeclaration(AstType type, std::string name, Expression* value = nullptr);
		void writeFunctionCall(const std::string& functionName, const std::vector<UPtr<Expression>>& arguments);

		std::ostream& m_ostream;
		int m_indent;
	};
}

#endif