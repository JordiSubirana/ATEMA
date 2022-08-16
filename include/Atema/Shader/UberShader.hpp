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

#ifndef ATEMA_SHADER_UBERSHADER_HPP
#define ATEMA_SHADER_UBERSHADER_HPP

#include <Atema/Shader/Config.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Core/ResourceManager.hpp>
#include <Atema/Shader/Enums.hpp>
#include <Atema/Shader/Ast/Statement.hpp>
#include <Atema/Shader/Ast/Constant.hpp>
#include <Atema/Shader/Ast/AstReflector.hpp>

namespace at
{
	class ATEMA_SHADER_API UberShader
	{
	public:
		struct Option
		{
			Option(const std::string& name, const ConstantValue& value) : name(name), value(value) {}

			std::string name;
			ConstantValue value;
		};

		UberShader() = delete;
		UberShader(const Ptr<SequenceStatement>& ast);
		UberShader(UPtr<SequenceStatement>&& ast);
		~UberShader();

		Ptr<UberShader> createInstance(const std::vector<Option>& options) const;
		Ptr<UberShader> extractStage(AstShaderStage stage) const;
		const Ptr<const AstReflection>& getReflection(AstShaderStage stage) const;

		const Ptr<SequenceStatement>& getAst() const;

	private:
		void initializeExtractor() const;

		Ptr<SequenceStatement> m_ast;

		// The following do not modify the ast in any way, this is just for saving state
		// So use ugly mutables to keep class methods const
		mutable AstReflector m_astReflector;
		mutable bool m_extractorReady;
		mutable std::unordered_map<AstShaderStage, Ptr<const AstReflection>> m_stageReflections;
	};
}

#endif
