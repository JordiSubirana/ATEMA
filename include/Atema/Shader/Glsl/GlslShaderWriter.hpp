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

#ifndef ATEMA_SHADER_GLSLSHADERWRITER_HPP
#define ATEMA_SHADER_GLSLSHADERWRITER_HPP

#include <Atema/Shader/Config.hpp>
#include <Atema/Shader/ShaderWriter.hpp>

#include <ostream>
#include <unordered_map>
#include <optional>

namespace at
{
	// Inherits NonCopyable to store unique pointers
	class ATEMA_SHADER_API GlslShaderWriter : public ShaderWriter, public NonCopyable
	{
	public:
		struct Settings
		{
			std::optional<AstShaderStage> stage;

			// OpenGL version major
			unsigned versionMajor = 3;
			// OpenGL version minor
			unsigned versionMinor = 3;
		};

		GlslShaderWriter() = delete;
		GlslShaderWriter(std::ostream& ostream, Settings settings = Settings());
		~GlslShaderWriter();

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
		void newLine();

		void indent();
		void unindent();

		void beginBlock();
		void endBlock();

		void addDelimiter();

		void writeHeader();
		void writeLayout(const Ptr<Expression>& location);
		void writeLayout(const Ptr<Expression>& set, const Ptr<Expression>& binding);
		void writeLayout(const Ptr<Expression>& set, const Ptr<Expression>& binding, StructLayout structLayout);
		void writeType(Type type);
		void writeType(ArrayType::ComponentType type);
		void writeVariableDeclaration(Type type, std::string name, Expression* value = nullptr);
		void writeFunctionCall(const std::string& functionName, const std::vector<UPtr<Expression>>& arguments);
		void writeInterfaceBlock(const std::string& blockName, const std::string& instanceName, const std::string& suffix);

		void preprocessorIf(Expression& expression);
		void preprocessorEndif();

		bool m_isSequenceProcessed;
		std::optional<AstShaderStage> m_stage;
		std::ostream& m_ostream;
		Settings m_settings;
		int m_indent;
		std::unordered_map<std::string, UPtr<StructDeclarationStatement>> m_structDeclarations;
		std::unordered_map<std::string, size_t> m_interfaceBlockCount;
	};
}

#endif