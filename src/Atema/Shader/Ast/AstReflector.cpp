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

#include <Atema/Shader/Ast/AstEvaluator.hpp>
#include <Atema/Shader/Ast/AstReflector.hpp>

using namespace at;

namespace
{
	uint32_t getUintValue(const std::optional<ConstantValue>& optionalValue)
	{
		if (!optionalValue.has_value() || !optionalValue.value().isOneOf<int32_t, uint32_t>())
			ATEMA_ERROR("Required value is not evaluable");

		if (optionalValue.value().is<int32_t>())
			return static_cast<uint32_t>(optionalValue.value().get<int32_t>());
		
		return optionalValue.value().get<uint32_t>();
	}
}

AstReflector::AstReflector() :
	m_currentAst(nullptr),
	m_currentDependencies(nullptr)
{
}

AstReflector::~AstReflector()
{
}

AstReflection AstReflector::getReflection(AstShaderStage stage)
{
	AstReflection reflection;

	AstEvaluator evaluator;

	const auto& entry = m_entries[stage];

	if (!entry.statement)
		ATEMA_ERROR("Missing entry function declaration");

	const auto& stageDependencies = getStageDependencies(stage);

	// Inputs
	for (const auto& statement : entry.inputs)
	{
		for (const auto& variable : statement->variables)
		{
			const auto location = evaluator.evaluate(*variable.location);

			reflection.addInput({ variable.name, variable.type, getUintValue(location) });
		}
	}

	// Outputs
	for (const auto& statement : entry.outputs)
	{
		for (const auto& variable : statement->variables)
		{
			const auto location = evaluator.evaluate(*variable.location);

			reflection.addOutput({ variable.name, variable.type, getUintValue(location) });
		}
	}

	// Externals
	for (const auto& externalName : stageDependencies.externalNames)
	{
		const auto& variable = m_externals[externalName].variable;

		const auto set = evaluator.evaluate(*variable.setIndex);
		const auto binding = evaluator.evaluate(*variable.bindingIndex);

		reflection.addExternal({ variable.name, variable.type, getUintValue(set), getUintValue(binding) });
	}

	// Structs
	for (const auto& structName : stageDependencies.structNames)
	{
		const auto& structData = m_structs[structName];

		std::vector<AstVariable> variables;
		for (const auto& variable : structData.statement->members)
			variables.emplace_back(variable.name, variable.type);

		reflection.addStruct({ structName, variables });
	}

	return reflection;
}

UPtr<SequenceStatement> AstReflector::getAst(AstShaderStage stage)
{
	// Create AST based on previously collected data
	auto ast = std::make_unique<SequenceStatement>();

	const auto& entry = m_entries[stage];

	if (!entry.statement)
		ATEMA_ERROR("Missing entry function declaration");

	auto& stageDependencies = getStageDependencies(stage);

	// Initialize current state
	m_currentAst = ast.get();
	m_currentDependencies = &stageDependencies;

	// Add options
	for (const auto& option : m_options)
		ast->statements.push_back(m_cloner.clone(option));

	// Add structs
	for (const auto& structName : stageDependencies.structNames)
		addStruct(structName);

	// Add external variables
	{
		auto externalStatement = std::make_unique<ExternalDeclarationStatement>();

		for (const auto& externalName : stageDependencies.externalNames)
			externalStatement->variables.push_back(m_externals[externalName].variable);

		ast->statements.push_back(std::move(externalStatement));
	}

	// Add global variables
	for (const auto& variableName : stageDependencies.variableNames)
		addVariable(variableName);

	// Add input
	for (const auto& input : entry.inputs)
		ast->statements.push_back(m_cloner.clone(input));

	// Add output
	for (const auto& output : entry.outputs)
		ast->statements.push_back(m_cloner.clone(output));

	// Add functions
	for (const auto& functionName : stageDependencies.functionNames)
		addFunction(functionName);

	// Add entry function
	ast->statements.push_back(m_cloner.clone(entry.statement));

	// Don't forget to clear history !
	m_currentAst = nullptr;
	m_currentDependencies = nullptr;

	m_addedStructs.clear();
	m_addedVariables.clear();
	m_addedFunctions.clear();

	return ast;
}

void AstReflector::visit(const EntryFunctionDeclarationStatement& statement)
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
	AstConstRecursiveVisitor::visit(statement);
	m_currentDependencies = nullptr;
}

void AstReflector::visit(const InputDeclarationStatement& statement)
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

void AstReflector::visit(const OutputDeclarationStatement& statement)
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

void AstReflector::visit(const ExternalDeclarationStatement& statement)
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

void AstReflector::visit(const OptionDeclarationStatement& statement)
{
	m_options.push_back(m_cloner.clone(statement));

	AstConstRecursiveVisitor::visit(statement);
}

void AstReflector::visit(const StructDeclarationStatement& statement)
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

void AstReflector::visit(const FunctionDeclarationStatement& statement)
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
	AstConstRecursiveVisitor::visit(statement);
	m_currentDependencies = nullptr;
}

void AstReflector::visit(const VariableDeclarationStatement& statement)
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
		AstConstRecursiveVisitor::visit(statement);
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

		AstConstRecursiveVisitor::visit(statement);
	}
}

void AstReflector::visit(const VariableExpression& expression)
{
	if (m_currentDependencies)
	{
		m_currentDependencies->variableNames.emplace(expression.identifier);
	}
}

void AstReflector::visit(const FunctionCallExpression& expression)
{
	if (m_currentDependencies)
	{
		m_currentDependencies->functionNames.emplace(expression.identifier);

		AstConstRecursiveVisitor::visit(expression);
	}
}

AstReflector::DependencyData& AstReflector::getStageDependencies(AstShaderStage stage)
{
	// Ensure current dependencies are not set
	m_currentDependencies = nullptr;

	// If dependencies were already resolved, just return them
	const auto it = m_stageDependencies.find(stage);

	if (it != m_stageDependencies.end())
		return it->second;

	// Otherwise we need to resolve stage's dependencies
	const auto& entry = m_entries[stage];

	if (!entry.statement)
		ATEMA_ERROR("Missing entry function declaration");

	auto& stageDependencies = m_stageDependencies[stage];

	resolveDependencies(stageDependencies, entry.dependencies);

	return stageDependencies;
}

void AstReflector::resolveDependencies(DependencyData& dstDependencies, const DependencyData& srcDependencies)
{
	// Recursively add other dependencies first
	for (auto& name : srcDependencies.structNames)
	{
		// Ensure the dependency wasn't already added
		if (dstDependencies.structNames.find(name) == dstDependencies.structNames.end())
		{
			// Ensure the dependency exists, then add it
			if (m_structs.find(name) != m_structs.end())
			{
				dstDependencies.structNames.emplace(name);

				auto& data = m_structs[name];

				resolveDependencies(dstDependencies, data.dependencies);
			}
		}
	}

	// Recursively add other dependencies first
	for (auto& name : srcDependencies.variableNames)
	{
		// Ensure the dependency wasn't already added
		if (dstDependencies.variableNames.find(name) == dstDependencies.variableNames.end())
		{
			// Ensure the dependency exists, then add it
			if (m_variables.find(name) != m_variables.end())
			{
				dstDependencies.variableNames.emplace(name);

				auto& data = m_variables[name];

				resolveDependencies(dstDependencies, data.dependencies);

				// If the variable is a struct instance, ensure we got the struct dependency
				const auto& variable = m_variables[name];
				if (variable.statement->type.is<StructType>())
				{
					const auto& structName = variable.statement->type.get<StructType>().name;

					// Ensure the dependency wasn't already added
					if (dstDependencies.structNames.find(structName) == dstDependencies.structNames.end())
					{
						// Ensure the dependency exists, then add it
						if (m_structs.find(structName) != m_structs.end())
						{
							dstDependencies.structNames.emplace(structName);

							auto& structData = m_structs[structName];

							resolveDependencies(dstDependencies, structData.dependencies);
						}
					}
				}
			}
			// If it's not a classic variable, it might be an external variable
			else if (m_externals.find(name) != m_externals.end())
			{
				// Ensure the dependency wasn't already added
				if (dstDependencies.externalNames.find(name) == dstDependencies.externalNames.end())
				{
					dstDependencies.externalNames.emplace(name);

					auto& data = m_externals[name];

					resolveDependencies(dstDependencies, data.dependencies);
				}
			}
		}
	}

	// Recursively add other dependencies first
	for (auto& name : srcDependencies.externalNames)
	{
		// Ensure the dependency wasn't already added
		if (dstDependencies.externalNames.find(name) == dstDependencies.externalNames.end())
		{
			// Ensure the dependency exists, then add it
			if (m_externals.find(name) != m_externals.end())
			{
				dstDependencies.externalNames.emplace(name);

				auto& data = m_externals[name];

				resolveDependencies(dstDependencies, data.dependencies);
			}
		}
	}

	// Recursively add other dependencies first
	for (auto& name : srcDependencies.functionNames)
	{
		// Ensure the dependency wasn't already added
		if (dstDependencies.functionNames.find(name) == dstDependencies.functionNames.end())
		{
			// Ensure the dependency exists, then add it
			if (m_functions.find(name) != m_functions.end())
			{
				dstDependencies.functionNames.emplace(name);

				auto& data = m_functions[name];

				resolveDependencies(dstDependencies, data.dependencies);
			}
		}
	}
}

void AstReflector::addDependencies(const DependencyData& data)
{
	for (auto& name : data.structNames)
		addStruct(name);

	for (auto& name : data.variableNames)
		addVariable(name);

	for (auto& name : data.functionNames)
		addFunction(name);
}

void AstReflector::addStruct(const std::string& name)
{
	if (m_addedStructs.count(name) > 0 || m_currentDependencies->structNames.count(name) == 0)
		return;

	m_addedStructs.emplace(name);

	const auto& data = m_structs[name];

	addDependencies(data.dependencies);

	m_currentAst->statements.emplace_back(m_cloner.clone(data.statement));
}

void AstReflector::addVariable(const std::string& name)
{
	if (m_addedVariables.count(name) > 0 || m_currentDependencies->variableNames.count(name) == 0)
		return;

	m_addedVariables.emplace(name);

	const auto& data = m_variables[name];

	addDependencies(data.dependencies);

	m_currentAst->statements.emplace_back(m_cloner.clone(data.statement));
}

void AstReflector::addFunction(const std::string& name)
{
	if (m_addedFunctions.count(name) > 0 || m_currentDependencies->functionNames.count(name) == 0)
		return;

	m_addedFunctions.emplace(name);

	const auto& data = m_functions[name];

	addDependencies(data.dependencies);

	m_currentAst->statements.emplace_back(m_cloner.clone(data.statement));
}

void AstReflector::clear()
{
	m_currentDependencies = nullptr;

	m_entries.clear();
	m_functions.clear();
	m_options.clear();
	m_externals.clear();
	m_structs.clear();
	m_variables.clear();
}
