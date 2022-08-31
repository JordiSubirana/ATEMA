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

#ifndef ATEMA_SHADER_AST_ASTREFLECTOR_HPP
#define ATEMA_SHADER_AST_ASTREFLECTOR_HPP


#include <Atema/Shader/Config.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Shader/Ast/Statement.hpp>
#include <Atema/Shader/Ast/Expression.hpp>
#include <Atema/Shader/Ast/AstRecursiveVisitor.hpp>
#include <Atema/Shader/Ast/AstCloner.hpp>
#include <Atema/Shader/Ast/Reflection.hpp>

#include <unordered_map>
#include <unordered_set>

namespace at
{
	//*
	class ATEMA_SHADER_API AstReflector : public AstConstRecursiveVisitor, public NonCopyable
	{
	public:
		AstReflector();
		~AstReflector();

		AstReflection getReflection(AstShaderStage stage);
		UPtr<SequenceStatement> getAst(AstShaderStage stage);

		void clear();

		void visit(const EntryFunctionDeclarationStatement& statement) override;
		void visit(const InputDeclarationStatement& statement) override;
		void visit(const OutputDeclarationStatement& statement) override;
		void visit(const ExternalDeclarationStatement& statement) override;
		void visit(const OptionDeclarationStatement& statement) override;
		void visit(const StructDeclarationStatement& statement) override;
		void visit(const FunctionDeclarationStatement& statement) override;
		void visit(const VariableDeclarationStatement& statement) override;

		void visit(const VariableExpression& expression) override;
		void visit(const FunctionCallExpression& expression) override;

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

		DependencyData& getStageDependencies(AstShaderStage stage);

		void resolveDependencies(DependencyData& dstDependencies, const DependencyData& srcDependencies);

		void addDependencies(const DependencyData& data);
		void addStruct(const std::string& name);
		void addVariable(const std::string& name);
		void addFunction(const std::string& name);

		AstCloner m_cloner;

		std::unordered_map<AstShaderStage, EntryData> m_entries;
		std::unordered_map<std::string, FunctionData> m_functions;
		std::list<UPtr<OptionDeclarationStatement>> m_options;
		std::unordered_map<std::string, ExternalData> m_externals;
		std::unordered_map<std::string, StructData> m_structs;
		std::unordered_map<std::string, VariableData> m_variables;

		SequenceStatement* m_currentAst;
		DependencyData* m_currentDependencies;

		std::unordered_map<AstShaderStage, DependencyData> m_stageDependencies;
		
		std::unordered_set<std::string> m_addedStructs;
		std::unordered_set<std::string> m_addedVariables;
		std::unordered_set<std::string> m_addedFunctions;
	};
}

#endif
