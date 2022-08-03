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

#ifndef ATEMA_SHADER_AST_REFLECTION_HPP
#define ATEMA_SHADER_AST_REFLECTION_HPP

#include <Atema/Shader/Config.hpp>
#include <Atema/Shader/Ast/Type.hpp>
#include <Atema/Shader/Ast/Constant.hpp>

#include <unordered_map>

namespace at
{
	struct ATEMA_SHADER_API AstVariable
	{
		AstVariable() = default;
		AstVariable(const std::string& name, const Type& type);

		std::string name;
		Type type;
	};

	struct ATEMA_SHADER_API AstStruct
	{
		AstStruct() = default;
		AstStruct(const std::string& name, const std::vector<AstVariable>& variables = {});

		std::string name;
		std::vector<AstVariable> variables;
	};

	struct ATEMA_SHADER_API AstInput : public AstVariable
	{
		AstInput();
		AstInput(const std::string& name, const Type& type, uint32_t location);

		uint32_t location;
	};

	struct ATEMA_SHADER_API AstOutput : public AstVariable
	{
		AstOutput();
		AstOutput(const std::string& name, const Type& type, uint32_t location);

		uint32_t location;
	};

	struct ATEMA_SHADER_API AstExternal : public AstVariable
	{
		AstExternal();
		AstExternal(const std::string& name, const Type& type, uint32_t set, uint32_t binding);

		uint32_t set;
		uint32_t binding;
	};

	class ATEMA_SHADER_API AstReflection
	{
	public:
		AstReflection();
		AstReflection(AstReflection&& other) noexcept;

		AstReflection& operator=(AstReflection&& other) noexcept;

		void addInput(const AstInput& input);
		void addOutput(const AstOutput& output);
		void addExternal(const AstExternal& external);
		void addStruct(const AstStruct& structData);

		const std::vector<AstInput>& getInputs() const noexcept;
		const std::vector<AstOutput>& getOutputs() const noexcept;
		const std::vector<AstExternal>& getExternals() const noexcept;
		const std::vector<AstStruct>& getStructs() const noexcept;

		const AstInput& getInput(const std::string& name) const;
		const AstInput& getInput(uint32_t location) const;
		const AstOutput& getOutput(const std::string& name) const;
		const AstOutput& getOutput(uint32_t location) const;
		const AstExternal& getExternal(const std::string& name) const;
		const AstExternal& getExternal(uint32_t set, uint32_t binding) const;
		const AstStruct& getStruct(const std::string& name) const;

	private:
		std::vector<AstInput> m_inputs;
		std::vector<AstOutput> m_outputs;
		std::vector<AstExternal> m_externals;
		std::vector<AstStruct> m_structs;

		std::unordered_map<std::string, size_t> m_inputFromName;
		std::unordered_map<uint32_t, size_t> m_inputFromLocation;
		std::unordered_map<std::string, size_t> m_outputFromName;
		std::unordered_map<uint32_t, size_t> m_outputFromLocation;
		std::unordered_map<std::string, size_t> m_externalFromName;
		std::unordered_map<Vector2u, size_t> m_externalFromSetBinding;
		std::unordered_map<std::string, size_t> m_structFromName;
	};
}

#endif
