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
#include <Atema/Shader/Ast/AstCloner.hpp>

using namespace at;

AstStageExtractor::AstStageExtractor() : m_currentDependencies(nullptr)
{
}

AstStageExtractor::~AstStageExtractor()
{
}

UPtr<SequenceStatement> AstStageExtractor::getAst(AstShaderStage stage)
{
	// Create AST based on previously collected data
	m_ast = std::make_unique<SequenceStatement>();

	auto& entry = m_entries[stage];

	if (!entry.statement)
	{
		ATEMA_ERROR("Missing entry function declaration");
	}

	// Resolve dependencies
	m_astDependencies = DependencyData();

	resolveDependencies(entry.dependencies);

	m_addedStructs.clear();
	m_addedVariables.clear();
	m_addedFunctions.clear();

	// Add options
	//TODO: Remove useless ones
	for (auto& option : m_options)
		m_ast->statements.push_back(m_cloner.clone(option));

	// Add structs
	for (auto& structName : m_astDependencies.structNames)
		addStruct(structName);

	// Add external variables
	{
		auto externalStatement = std::make_unique<ExternalDeclarationStatement>();

		for (auto& externalName : m_astDependencies.externalNames)
			externalStatement->variables.push_back(m_externals[externalName].variable);

		m_ast->statements.push_back(std::move(externalStatement));
	}

	// Add global variables
	for (auto& variableName : m_astDependencies.variableNames)
		addVariable(variableName);

	// Add input
	for (auto& input : entry.inputs)
		m_ast->statements.push_back(m_cloner.clone(input));

	// Add output
	for (auto& output : entry.outputs)
		m_ast->statements.push_back(m_cloner.clone(output));

	// Add functions
	for (auto& functionName : m_astDependencies.functionNames)
		addFunction(functionName);

	// Add entry function
	m_ast->statements.push_back(m_cloner.clone(entry.statement));

	return std::move(m_ast);
}

void AstStageExtractor::visit(EntryFunctionDeclarationStatement& statement)
{
	if (m_entries[statement.stage].statement)
	{
		ATEMA_ERROR("Entry function already defined");
	}

	auto& entry = m_entries[statement.stage];

	entry.statement = m_cloner.clone(statement);

	if (statement.returnType.is<StructType>())
	{
		const auto& structName = statement.returnType.get<StructType>().name;
		entry.dependencies.structNames.emplace(structName);
	}

	for (auto& argument : statement.arguments)
	{
		if (argument.type.is<StructType>())
		{
			const auto& structName = argument.type.get<StructType>().name;
			entry.dependencies.structNames.emplace(structName);
		}
	}

	// Find dependencies
	m_currentDependencies = &entry.dependencies;
	AstRecursiveVisitor::visit(statement);
	m_currentDependencies = nullptr;
}

void AstStageExtractor::visit(InputDeclarationStatement& statement)
{
	auto& entry = m_entries[statement.stage];

	entry.inputs.push_back(m_cloner.clone(statement));

	for (auto& variable : statement.variables)
	{
		if (variable.type.is<StructType>())
		{
			const auto& structName = variable.type.get<StructType>().name;
			entry.dependencies.structNames.emplace(structName);
		}
	}
}

void AstStageExtractor::visit(OutputDeclarationStatement& statement)
{
	auto& entry = m_entries[statement.stage];

	entry.outputs.push_back(m_cloner.clone(statement));

	for (auto& variable : statement.variables)
	{
		if (variable.type.is<StructType>())
		{
			const auto& structName = variable.type.get<StructType>().name;
			entry.dependencies.structNames.emplace(structName);
		}
	}
}

void AstStageExtractor::visit(ExternalDeclarationStatement& statement)
{
	for (auto& variable : statement.variables)
	{
		if (m_externals.find(variable.name) != m_externals.end())
		{
			ATEMA_ERROR("External '" + variable.name + "' already defined");
		}
		
		auto& externalData = m_externals[variable.name];

		externalData.variable = variable;

		if (variable.type.is<StructType>())
		{
			const auto& structName = variable.type.get<StructType>().name;
			externalData.dependencies.structNames.emplace(structName);
		}
	}
}

void AstStageExtractor::visit(OptionDeclarationStatement& statement)
{
	m_options.push_back(m_cloner.clone(statement));

	AstRecursiveVisitor::visit(statement);
}

void AstStageExtractor::visit(StructDeclarationStatement& statement)
{
	if (m_structs.find(statement.name) != m_structs.end())
	{
		ATEMA_ERROR("Struct '" + statement.name + "' already defined");
	}

	auto& structure = m_structs[statement.name];

	structure.statement = m_cloner.clone(statement);

	for (auto& member : statement.members)
	{
		if (member.type.is<StructType>())
		{
			const auto& structName = member.type.get<StructType>().name;
			structure.dependencies.structNames.emplace(structName);
		}
	}
}

void AstStageExtractor::visit(FunctionDeclarationStatement& statement)
{
	if (m_functions.find(statement.name) != m_functions.end())
	{
		ATEMA_ERROR("Function '" + statement.name + "' already defined");
	}

	auto& function = m_functions[statement.name];

	function.statement = m_cloner.clone(statement);

	if (statement.returnType.is<StructType>())
	{
		const auto& structName = statement.returnType.get<StructType>().name;
		function.dependencies.structNames.emplace(structName);
	}

	for (auto& argument : statement.arguments)
	{
		if (argument.type.is<StructType>())
		{
			const auto& structName = argument.type.get<StructType>().name;
			function.dependencies.structNames.emplace(structName);
		}
	}

	// Find dependencies
	m_currentDependencies = &function.dependencies;
	AstRecursiveVisitor::visit(statement);
	m_currentDependencies = nullptr;
}

void AstStageExtractor::visit(VariableDeclarationStatement& statement)
{
	// Register global variables
	if (!m_currentDependencies)
	{
		if (m_variables.find(statement.name) != m_variables.end())
		{
			ATEMA_ERROR("Global variable '" + statement.name + "' already defined");
		}

		auto& variable = m_variables[statement.name];

		variable.statement = m_cloner.clone(statement);

		// Find dependencies
		m_currentDependencies = &variable.dependencies;
		AstRecursiveVisitor::visit(statement);
		m_currentDependencies = nullptr;
	}
	// Register local variable struct types in the current dependencies
	else
	{
		if (statement.type.is<StructType>())
		{
			auto& typeName = statement.type.get<StructType>().name;

			m_currentDependencies->structNames.emplace(typeName);
		}

		AstRecursiveVisitor::visit(statement);
	}
}

void AstStageExtractor::visit(VariableExpression& expression)
{
	if (m_currentDependencies)
	{
		m_currentDependencies->variableNames.emplace(expression.identifier);
	}
}

void AstStageExtractor::visit(FunctionCallExpression& expression)
{
	if (m_currentDependencies)
	{
		m_currentDependencies->functionNames.emplace(expression.identifier);

		AstRecursiveVisitor::visit(expression);
	}
}

void AstStageExtractor::resolveDependencies(const DependencyData& dependencies)
{
	// Recursively add other dependencies first
	for (auto& name : dependencies.structNames)
	{
		// Ensure the dependency wasn't already added
		if (m_astDependencies.structNames.find(name) == m_astDependencies.structNames.end())
		{
			// Ensure the dependency exists, then add it
			if (m_structs.find(name) != m_structs.end())
			{
				m_astDependencies.structNames.emplace(name);

				auto& data = m_structs[name];

				resolveDependencies(data.dependencies);
			}
		}
	}

	// Recursively add other dependencies first
	for (auto& name : dependencies.variableNames)
	{
		// Ensure the dependency wasn't already added
		if (m_astDependencies.variableNames.find(name) == m_astDependencies.variableNames.end())
		{
			// Ensure the dependency exists, then add it
			if (m_variables.find(name) != m_variables.end())
			{
				m_astDependencies.variableNames.emplace(name);

				auto& data = m_variables[name];

				resolveDependencies(data.dependencies);

				// If the variable is a struct instance, ensure we got the struct dependency
				const auto& variable = m_variables[name];
				if (variable.statement->type.is<StructType>())
				{
					const auto& structName = variable.statement->type.get<StructType>().name;

					// Ensure the dependency wasn't already added
					if (m_astDependencies.structNames.find(structName) == m_astDependencies.structNames.end())
					{
						// Ensure the dependency exists, then add it
						if (m_structs.find(structName) != m_structs.end())
						{
							m_astDependencies.structNames.emplace(structName);

							auto& structData = m_structs[structName];

							resolveDependencies(structData.dependencies);
						}
					}
				}
			}
			// If it's not a classic variable, it might be an external variable
			else if (m_externals.find(name) != m_externals.end())
			{
				// Ensure the dependency wasn't already added
				if (m_astDependencies.externalNames.find(name) == m_astDependencies.externalNames.end())
				{
					m_astDependencies.externalNames.emplace(name);

					auto& data = m_externals[name];

					resolveDependencies(data.dependencies);
				}
			}
		}
	}

	// Recursively add other dependencies first
	for (auto& name : dependencies.externalNames)
	{
		// Ensure the dependency wasn't already added
		if (m_astDependencies.externalNames.find(name) == m_astDependencies.externalNames.end())
		{
			// Ensure the dependency exists, then add it
			if (m_externals.find(name) != m_externals.end())
			{
				m_astDependencies.externalNames.emplace(name);

				auto& data = m_externals[name];

				resolveDependencies(data.dependencies);
			}
		}
	}

	// Recursively add other dependencies first
	for (auto& name : dependencies.functionNames)
	{
		// Ensure the dependency wasn't already added
		if (m_astDependencies.functionNames.find(name) == m_astDependencies.functionNames.end())
		{
			// Ensure the dependency exists, then add it
			if (m_functions.find(name) != m_functions.end())
			{
				m_astDependencies.functionNames.emplace(name);

				auto& data = m_functions[name];

				resolveDependencies(data.dependencies);
			}
		}
	}
}

void AstStageExtractor::addDependencies(const DependencyData& data)
{
	for (auto& name : data.structNames)
		addStruct(name);

	for (auto& name : data.variableNames)
		addVariable(name);

	for (auto& name : data.functionNames)
		addFunction(name);
}

void AstStageExtractor::addStruct(const std::string& name)
{
	if (m_addedStructs.count(name) > 0 || m_astDependencies.structNames.count(name) == 0)
		return;

	m_addedStructs.emplace(name);

	const auto& data = m_structs[name];

	addDependencies(data.dependencies);

	m_ast->statements.emplace_back(m_cloner.clone(data.statement));
}

void AstStageExtractor::addVariable(const std::string& name)
{
	if (m_addedVariables.count(name) > 0 || m_astDependencies.variableNames.count(name) == 0)
		return;

	m_addedVariables.emplace(name);

	const auto& data = m_variables[name];

	addDependencies(data.dependencies);

	m_ast->statements.emplace_back(m_cloner.clone(data.statement));
}

void AstStageExtractor::addFunction(const std::string& name)
{
	if (m_addedFunctions.count(name) > 0 || m_astDependencies.functionNames.count(name) == 0)
		return;

	m_addedFunctions.emplace(name);

	const auto& data = m_functions[name];

	addDependencies(data.dependencies);

	m_ast->statements.emplace_back(m_cloner.clone(data.statement));
}

void AstStageExtractor::clear()
{
	m_currentDependencies = nullptr;

	m_entries.clear();
	m_functions.clear();
	m_options.clear();
	m_externals.clear();
	m_structs.clear();
	m_variables.clear();
}
