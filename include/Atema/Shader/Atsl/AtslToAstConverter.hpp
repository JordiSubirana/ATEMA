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

#ifndef ATEMA_SHADER_ATSLTOASTCONVERTER_HPP
#define ATEMA_SHADER_ATSLTOASTCONVERTER_HPP

#include <Atema/Core/Pointer.hpp>
#include <Atema/Core/Variant.hpp>
#include <Atema/Shader/Config.hpp>
#include <Atema/Shader/Atsl/AtslToken.hpp>
#include <Atema/Shader/Ast/Statement.hpp>
#include <Atema/Shader/Ast/Expression.hpp>

#include <vector>
#include <string>
#include <unordered_map>

namespace at
{
	class ATEMA_SHADER_API AtslToAstConverter
	{
	public:
		AtslToAstConverter();
		virtual ~AtslToAstConverter();

		UPtr<SequenceStatement> createAst(const std::vector<AtslToken>& tokens);

	private:
		using Attribute = Variant<AtslBasicValue, AtslIdentifier>;
		using AttributeMap = std::unordered_map<AtslIdentifier, Attribute>;
		struct VariableData
		{
			Flags<VariableQualifier> qualifiers;
			Type type;
			std::string name;
			UPtr<Expression> value;
		};

		// Token iteration methods
		// All offsets are from current index
		bool remains(size_t offset = 1) const;
		const AtslToken& get(size_t offset = 0) const;
		const AtslToken& iterate(size_t offset = 1);

		void expect(const AtslToken& token, AtslSymbol symbol) const;
		void expect(const AtslToken& token, AtslKeyword keyword) const;
		
		bool hasAttribute(const AtslIdentifier& identifier) const;
		const Attribute& getAttribute(const AtslIdentifier& identifier) const;
		const AtslIdentifier& expectAttributeIdentifier(const AtslIdentifier& name) const;
		bool expectAttributeBool(const AtslIdentifier& name) const;
		uint32_t expectAttributeInt(const AtslIdentifier& name) const;
		float expectAttributeFloat(const AtslIdentifier& name) const;
		
		void createAttributes();
		UPtr<Expression> parseExpression();
		std::vector<UPtr<Expression>> parseArguments();
		UPtr<Expression> parseParenthesisExpression();
		UPtr<Expression> createFunctionCall();
		VariableData parseVariableDeclarationData();
		void createVariableBlock();
		void createOptions();
		void createConsts();
		void createStruct();
		UPtr<SequenceStatement> createBlockSequence();
		UPtr<Statement> createBlockStatement();
		
		// Statements : at the end, semicolon is parsed
		UPtr<ConditionalStatement> parseConditionalBranch();
		UPtr<ForLoopStatement> parseForLoop();
		UPtr<WhileLoopStatement> parseWhileLoop();
		UPtr<DoWhileLoopStatement> parseDoWhileLoop();
		UPtr<VariableDeclarationStatement> parseVariableDeclaration();
		UPtr<StructDeclarationStatement> parseStructDeclaration();
		UPtr<FunctionDeclarationStatement> parseFunctionDeclaration();
		UPtr<BreakStatement> parseBreak();
		UPtr<ContinueStatement> parseContinue();
		UPtr<ReturnStatement> parseReturn();

		// Expressions : delimiter is not known, so no parsing
		UPtr<Expression> parseFunctionCall(); // Classic function or Built-in function
		UPtr<CastExpression> parseCast();

		void clearAttributes();
		
		UPtr<SequenceStatement> m_root;

		SequenceStatement *m_currentSequence;
		SequenceStatement *m_parentSequence;

		std::unordered_map<AtslIdentifier, Attribute> m_attributes;
		
		const std::vector<AtslToken>* m_tokens;
		
		size_t m_currentIndex;
	};
}

#endif
