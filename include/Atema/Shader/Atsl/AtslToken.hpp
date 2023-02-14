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

#ifndef ATEMA_SHADER_ATSLTOKEN_HPP
#define ATEMA_SHADER_ATSLTOKEN_HPP

#include <Atema/Shader/Config.hpp>
#include <Atema/Core/Variant.hpp>

#include <string>
#include <ostream>

namespace at
{
	enum class AtslTokenType
	{
		Symbol,
		Keyword,
		Identifier,
		Value,
	};

	enum class AtslSymbol
	{
		Plus,
		Minus,
		Multiply,
		Divide,
		Power,
		Modulo,

		Equal,
		Less,
		Greater,

		And,
		Or,
		Not,

		Dot,
		Comma,
		Colon,
		Semicolon,
		QuestionMark,

		LeftBrace,
		RightBrace,
		LeftBracket,
		RightBracket,
		LeftParenthesis,
		RightParenthesis,
	};

	enum class AtslKeyword
	{
		Input,
		Output,
		External,
		Option,
		Optional,

		Include,

		Const,

		If,
		Else,

		For,
		While,
		Do,

		Struct,

		Break,
		Continue,
		Return,
		Discard,
	};

	using AtslIdentifier = std::string;

	using AtslBasicValue = Variant<
		bool,
		int32_t,
		float>;

	using AtslTokenValue = Variant<
		AtslSymbol,
		AtslKeyword,
		AtslIdentifier,
		AtslBasicValue>;

	struct ATEMA_SHADER_API AtslToken
	{
		AtslToken() = delete;
		AtslToken(AtslSymbol symbol);
		AtslToken(AtslKeyword keyword);
		AtslToken(const AtslIdentifier& identifier);
		AtslToken(bool value);
		AtslToken(int32_t value);
		AtslToken(float value);

		bool is(AtslSymbol symbol) const noexcept;
		bool is(AtslKeyword keyword) const noexcept;
		bool is(const AtslIdentifier& identifier) const noexcept;
		bool is(bool value) const noexcept;
		bool is(int32_t value) const noexcept;
		bool is(float value) const noexcept;

		std::string toString() const;
		
		AtslTokenType type;
		AtslTokenValue value;

		size_t line;
		size_t column;
	};

	// Stream operator overload
	inline std::ostream& operator<<(std::ostream& os, const AtslBasicValue& value)
	{
		if (value.is<bool>())
			os << value.get<bool>();
		else if (value.is<int32_t>())
			os << value.get<int32_t>();
		else if (value.is<float>())
			os << value.get<float>();
		else
		{
			//TODO: Handle something or just do nothing ?
		}

		return os;
	}
}

#endif
