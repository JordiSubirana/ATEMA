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

#include <Atema/Shader/Ast/Reflection.hpp>

using namespace at;

AstVariable::AstVariable(const std::string& name, const Type& type) :
	name(name),
	type(type)
{
}

AstStruct::AstStruct(const std::string& name, const std::vector<AstVariable>& variables) :
	name(name),
	variables(variables)
{
}

AstInput::AstInput() :
	AstInput("", Type(), 0)
{
}

AstInput::AstInput(const std::string& name, const Type& type, uint32_t location) :
	AstVariable(name, type),
	location(location)
{
}

AstOutput::AstOutput() :
	AstOutput("", Type(), 0)
{
}

AstOutput::AstOutput(const std::string& name, const Type& type, uint32_t location) :
	AstVariable(name, type),
	location(location)
{
}

AstExternal::AstExternal() :
	AstExternal("", Type(), 0, 0)
{
}

AstExternal::AstExternal(const std::string& name, const Type& type, uint32_t set, uint32_t binding) :
	AstVariable(name, type),
	set(set),
	binding(binding)
{
}

// Reflection
AstReflection::AstReflection()
{
}

AstReflection::AstReflection(AstReflection&& other) noexcept
{
	operator=(std::move(other));
}

AstReflection& AstReflection::operator=(AstReflection&& other) noexcept
{
	m_inputs = std::move(other.m_inputs);
	m_outputs = std::move(other.m_outputs);
	m_externals = std::move(other.m_externals);
	m_structs = std::move(other.m_structs);

	m_inputFromName = std::move(other.m_inputFromName);
	m_inputFromLocation = std::move(other.m_inputFromLocation);
	m_outputFromName = std::move(other.m_outputFromName);
	m_outputFromLocation = std::move(other.m_outputFromLocation);
	m_externalFromName = std::move(other.m_externalFromName);
	m_externalFromSetBinding = std::move(other.m_externalFromSetBinding);
	m_structFromName = std::move(other.m_structFromName);

	return *this;
}

void AstReflection::addInput(const AstInput& input)
{
	const auto index = m_inputs.size();

	m_inputs.emplace_back(input);

	m_inputFromName[input.name] = index;
	m_inputFromLocation[input.location] = index;
}

void AstReflection::addOutput(const AstOutput& output)
{
	const auto index = m_outputs.size();

	m_outputs.emplace_back(output);

	m_outputFromName[output.name] = index;
	m_outputFromLocation[output.location] = index;
}

void AstReflection::addExternal(const AstExternal& external)
{
	const auto index = m_externals.size();

	m_externals.emplace_back(external);

	m_externalFromName[external.name] = index;
	m_externalFromSetBinding[{ external.set, external.binding }] = index;
}

void AstReflection::addStruct(const AstStruct& structData)
{
	const auto index = m_structs.size();

	m_structs.emplace_back(structData);

	m_structFromName[structData.name] = index;
}

const std::vector<AstInput>& AstReflection::getInputs() const noexcept
{
	return m_inputs;
}

const std::vector<AstOutput>& AstReflection::getOutputs() const noexcept
{
	return m_outputs;
}

const std::vector<AstExternal>& AstReflection::getExternals() const noexcept
{
	return m_externals;
}

const std::vector<AstStruct>& AstReflection::getStructs() const noexcept
{
	return m_structs;
}

const AstInput& AstReflection::getInput(const std::string& name) const
{
	const auto it = m_inputFromName.find(name);

	if (it == m_inputFromName.end())
		ATEMA_ERROR("Invalid input name : '" + name + "'");

	return m_inputs[it->second];
}

const AstInput& AstReflection::getInput(uint32_t location) const
{
	const auto it = m_inputFromLocation.find(location);

	if (it == m_inputFromLocation.end())
		ATEMA_ERROR("Invalid input location : " + std::to_string(location));

	return m_inputs[it->second];
}

const AstOutput& AstReflection::getOutput(const std::string& name) const
{
	const auto it = m_outputFromName.find(name);

	if (it == m_outputFromName.end())
		ATEMA_ERROR("Invalid output name : '" + name + "'");

	return m_outputs[it->second];
}

const AstOutput& AstReflection::getOutput(uint32_t location) const
{
	const auto it = m_outputFromLocation.find(location);

	if (it == m_outputFromLocation.end())
		ATEMA_ERROR("Invalid output location : " + std::to_string(location));

	return m_outputs[it->second];
}

const AstExternal& AstReflection::getExternal(const std::string& name) const
{
	const auto it = m_externalFromName.find(name);

	if (it == m_externalFromName.end())
		ATEMA_ERROR("Invalid external name : '" + name + "'");

	return m_externals[it->second];
}

const AstExternal& AstReflection::getExternal(uint32_t set, uint32_t binding) const
{
	const auto it = m_externalFromSetBinding.find({ set, binding });

	if (it == m_externalFromSetBinding.end())
		ATEMA_ERROR("Invalid external set/binding : " + std::to_string(set) + "/" + std::to_string(binding));

	return m_externals[it->second];
}

const AstStruct& AstReflection::getStruct(const std::string& name) const
{
	const auto it = m_structFromName.find(name);

	if (it == m_structFromName.end())
		ATEMA_ERROR("Invalid struct name : '" + name + "'");

	return m_structs[it->second];
}
