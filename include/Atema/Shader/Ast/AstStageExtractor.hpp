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

#ifndef ATEMA_SHADER_AST_ASTSTAGEEXTRACTOR_HPP
#define ATEMA_SHADER_AST_ASTSTAGEEXTRACTOR_HPP

#include <Atema/Shader/Config.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Shader/Ast/Statement.hpp>
#include <Atema/Shader/Ast/Expression.hpp>
#include <Atema/Shader/Ast/AstRecursiveVisitor.hpp>
#include <Atema/Shader/Ast/AstCloner.hpp>

#include <unordered_map>
#include <unordered_set>

namespace at
{
	class ATEMA_SHADER_API AstStageExtractor : public AstRecursiveVisitor
	{
	public:
		AstStageExtractor();
		~AstStageExtractor();

		UPtr<SequenceStatement> getAst(AstShaderStage stage);

		void clear();

		void visit(EntryFunctionDeclarationStatement& statement) override;
		void visit(InputDeclarationStatement& statement) override;
		void visit(OutputDeclarationStatement& statement) override;
		void visit(ExternalDeclarationStatement& statement) override;
		void visit(OptionDeclarationStatement& statement) override;
		void visit(StructDeclarationStatement& statement) override;
		void visit(FunctionDeclarationStatement& statement) override;
		void visit(VariableDeclarationStatement& statement) override;

		void visit(VariableExpression& expression) override;
		void visit(FunctionCallExpression& expression) override;

	private:
		struct DependencyData
		{
			std::unordered_set<std::string> functionNames;
			std::unordered_set<std::string> structNames;
			std::unordered_set<std::string> variableNames;
			std::unordered_set<std::string> externalNames;
		};

		struct EntryData
		{
			UPtr<EntryFunctionDeclarationStatement> statement;
			std::list<UPtr<InputDeclarationStatement>> inputs;
			std::list<UPtr<OutputDeclarationStatement>> outputs;
			DependencyData dependencies;
		};

		struct FunctionData
		{
			UPtr<FunctionDeclarationStatement> statement;
			DependencyData dependencies;
		};

		struct ExternalData
		{
			ExternalDeclarationStatement::Variable variable;
			DependencyData dependencies;
		};

		struct StructData
		{
			UPtr<StructDeclarationStatement> statement;
			DependencyData dependencies;
		};

		struct VariableData
		{
			UPtr<VariableDeclarationStatement> statement;
			DependencyData dependencies;
		};

		void resolveDependencies(const DependencyData& dependencies);

		AstCloner m_cloner;

		std::unordered_map<AstShaderStage, EntryData> m_entries;
		std::unordered_map<std::string, FunctionData> m_functions;
		std::list<UPtr<OptionDeclarationStatement>> m_options;
		std::unordered_map<std::string, ExternalData> m_externals;
		std::unordered_map<std::string, StructData> m_structs;
		std::unordered_map<std::string, VariableData> m_variables;

		DependencyData* m_currentDependencies;

		UPtr<SequenceStatement> m_ast;

		DependencyData m_astDependencies;
	};
}

#endif
