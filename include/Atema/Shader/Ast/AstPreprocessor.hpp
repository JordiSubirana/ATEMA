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

#ifndef ATEMA_SHADER_ASTPREPROCESSOR_HPP
#define ATEMA_SHADER_ASTPREPROCESSOR_HPP

#include <Atema/Shader/Config.hpp>
#include <Atema/Shader/Ast/AstRecursiveVisitor.hpp>
#include <Atema/Shader/Ast/AstCloner.hpp>

#include <unordered_map>
#include <optional>

namespace at
{
	class ATEMA_SHADER_API AstPreprocessor : public AstRecursiveVisitor
	{
	public:
		AstPreprocessor();
		~AstPreprocessor();

		void setOption(const std::string& optionName, const ConstantValue& value);

		void clear();

		void visit(OptionDeclarationStatement& statement) override;

		UPtr<Statement> process(const Statement& statement);
		UPtr<Expression> process(const Expression& expression);

#define ATEMA_MACROLIST_SHADERASTSTATEMENT(at_statement) UPtr<Statement> process(const at_statement ## Statement& statement);
#include <Atema/Shader/Ast/StatementMacroList.hpp>

#define ATEMA_MACROLIST_SHADERASTEXPRESSION(at_expression) UPtr<Expression> process(const at_expression ## Expression& expression);
#include <Atema/Shader/Ast/ExpressionMacroList.hpp>
		
	private:
		std::optional<ConstantValue> evaluate(Expression& expression);
		std::optional<bool> evaluateCondition(Expression& expression);

		UPtr<Expression> createConstantIfPossible(UPtr<Expression>&& value);
		UPtr<ConstantExpression> createConstantExpression(const ConstantValue& value);

		//std::optional<ConstantValue> getUserOption(const std::string& optionName);

		std::unordered_map<std::string, ConstantValue> m_options;
		std::unordered_map<std::string, ConstantValue> m_defaultOptions;

		AstCloner m_cloner;
	};
}

#endif
