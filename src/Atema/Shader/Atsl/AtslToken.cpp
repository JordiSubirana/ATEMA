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

#include <Atema/Shader/Atsl/AtslToken.hpp>

using namespace at;

AtslToken::AtslToken(AtslSymbol symbol) :
	line(0),
	column(0),
	type(AtslTokenType::Symbol),
	value(symbol)
{
}

AtslToken::AtslToken(AtslKeyword keyword) :
	line(0),
	column(0),
	type(AtslTokenType::Keyword),
	value(keyword)
{
}

AtslToken::AtslToken(const AtslIdentifier& identifier) :
	line(0),
	column(0),
	type(AtslTokenType::Identifier),
	value(identifier)
{
}

AtslToken::AtslToken(bool value) :
	line(0),
	column(0),
	type(AtslTokenType::Value),
	value(AtslBasicValue(value))
{
}

AtslToken::AtslToken(uint32_t value) :
	line(0),
	column(0),
	type(AtslTokenType::Value),
	value(AtslBasicValue(value))
{
}

AtslToken::AtslToken(float value) :
	line(0),
	column(0),
	type(AtslTokenType::Value),
	value(AtslBasicValue(value))
{
}

bool AtslToken::is(AtslSymbol symbol) const noexcept
{
	return value.is<AtslSymbol>() && value.get<AtslSymbol>() == symbol;
}

bool AtslToken::is(AtslKeyword keyword) const noexcept
{
	return value.is<AtslKeyword>() && value.get<AtslKeyword>() == keyword;
}

bool AtslToken::is(const AtslIdentifier& identifier) const noexcept
{
	return value.is<AtslIdentifier>() && value.get<AtslIdentifier>() == identifier;
}

bool AtslToken::is(bool basicValue) const noexcept
{
	if (!value.is<AtslBasicValue>())
		return false;

	auto& val = value.get<AtslBasicValue>();
	
	return val.is<bool>() && val.get<bool>() == basicValue;
}

bool AtslToken::is(uint32_t basicValue) const noexcept
{
	if (!value.is<AtslBasicValue>())
		return false;

	auto& val = value.get<AtslBasicValue>();

	return val.is<uint32_t>() && val.get<uint32_t>() == basicValue;
}

bool AtslToken::is(float basicValue) const noexcept
{
	if (!value.is<AtslBasicValue>())
		return false;

	auto& val = value.get<AtslBasicValue>();

	return val.is<float>() && val.get<float>() == basicValue;
}
