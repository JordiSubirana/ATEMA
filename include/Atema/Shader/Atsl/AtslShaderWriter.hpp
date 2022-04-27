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

		void write(const ConditionalStatement& statement) override;
		void write(const ForLoopStatement& statement) override;
		void write(const WhileLoopStatement& statement) override;
		void write(const DoWhileLoopStatement& statement) override;
		void write(const VariableDeclarationStatement& statement) override;
		void write(const StructDeclarationStatement& statement) override;
		void write(const InputDeclarationStatement& statement) override;
		void write(const OutputDeclarationStatement& statement) override;
		void write(const ExternalDeclarationStatement& statement) override;
		void write(const OptionDeclarationStatement& statement) override;
		void write(const FunctionDeclarationStatement& statement) override;
		void write(const EntryFunctionDeclarationStatement& statement) override;
		void write(const ExpressionStatement& statement) override;
		void write(const BreakStatement& statement) override;
		void write(const ContinueStatement& statement) override;
		void write(const ReturnStatement& statement) override;
		void write(const SequenceStatement& statement) override;

		void write(const ConstantExpression& expression) override;
		void write(const VariableExpression& expression) override;
		void write(const AccessIndexExpression& expression) override;
		void write(const AccessIdentifierExpression& expression) override;
		void write(const AssignmentExpression& expression) override;
		void write(const UnaryExpression& expression) override;
		void write(const BinaryExpression& expression) override;
		void write(const FunctionCallExpression& expression) override;
		void write(const BuiltInFunctionCallExpression& expression) override;
		void write(const CastExpression& expression) override;
		void write(const SwizzleExpression& expression) override;
		void write(const TernaryExpression& expression) override;

	private:
		struct Attribute
		{
			std::string name;
			Variant<AtslBasicValue, AtslIdentifier> value;
		};

		void newLine();

		void indent();
		void unindent();

		void beginBlock();
		void endBlock();

		void addDelimiter();

		void writeType(Type type);
		void writeAttributes(const std::vector<Attribute>& attributes);
		void writeVariableDeclaration(Type type, std::string name, const Expression* value = nullptr);
		void writeFunctionCall(const std::string& functionName, const std::vector<UPtr<Expression>>& arguments);

		std::ostream& m_ostream;
		int m_indent;
	};
}

#endif