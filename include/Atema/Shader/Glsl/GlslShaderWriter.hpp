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

#ifndef ATEMA_SHADER_GLSLSHADERWRITER_HPP
#define ATEMA_SHADER_GLSLSHADERWRITER_HPP

#include <Atema/Shader/Config.hpp>
#include <Atema/Shader/ShaderWriter.hpp>

#include <ostream>

namespace at
{
	class ATEMA_SHADER_API GlslShaderWriter : public ShaderWriter
	{
	public:
		struct OpenGLSettings
		{
			unsigned versionMajor = 3;
			unsigned versionMinor = 3;
		};

		GlslShaderWriter() = delete;
		GlslShaderWriter(AstShaderStage stage, std::ostream& ostream, OpenGLSettings openglSettings = OpenGLSettings());
		~GlslShaderWriter();

		void visit(ConditionalStatement& statement) override;
		void visit(ForLoopStatement& statement) override;
		void visit(WhileLoopStatement& statement) override;
		void visit(DoWhileLoopStatement& statement) override;
		void visit(VariableDeclarationStatement& statement) override;
		void visit(StructDeclarationStatement& statement) override;
		void visit(InputDeclarationStatement& statement) override;
		void visit(OutputDeclarationStatement& statement) override;
		void visit(ExternalDeclarationStatement& statement) override;
		void visit(OptionDeclarationStatement& statement) override;
		void visit(FunctionDeclarationStatement& statement) override;
		void visit(EntryFunctionDeclarationStatement& statement) override;
		void visit(ExpressionStatement& statement) override;
		void visit(BreakStatement& statement) override;
		void visit(ContinueStatement& statement) override;
		void visit(ReturnStatement& statement) override;
		void visit(SequenceStatement& statement) override;

		void visit(ConstantExpression& expression) override;
		void visit(VariableExpression& expression) override;
		void visit(AccessIndexExpression& expression) override;
		void visit(AccessIdentifierExpression& expression) override;
		void visit(AssignmentExpression& expression) override;
		void visit(UnaryExpression& expression) override;
		void visit(BinaryExpression& expression) override;
		void visit(FunctionCallExpression& expression) override;
		void visit(BuiltInFunctionCallExpression& expression) override;
		void visit(CastExpression& expression) override;
		void visit(SwizzleExpression& expression) override;
		void visit(TernaryExpression& expression) override;

	private:
		void newLine();

		void indent();
		void unindent();

		void beginBlock();
		void endBlock();

		void addDelimiter();

		void writeHeader();
		void writeLayout(uint32_t location);
		void writeLayout(uint32_t set, uint32_t binding);
		void writeType(Type type);
		void writeVariableDeclaration(Type type, std::string name, Expression* value = nullptr);
		void writeFunctionCall(const std::string& functionName, const std::vector<UPtr<Expression>>& arguments);

		bool m_isSequenceProcessed;
		AstShaderStage m_stage;
		std::ostream& m_ostream;
		OpenGLSettings m_openglSettings;
		int m_indent;
	};
}

#endif