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

#include <Atema/Shader/Atsl/Utils.hpp>
#include <Atema/Core/Error.hpp>

#include <unordered_map>
#include <unordered_set>

using namespace at;
using namespace atsl;

namespace
{
	std::unordered_map<AtslKeyword, std::string> s_keywordToStr =
	{
		{ AtslKeyword::Input, "input" },
		{ AtslKeyword::Output, "output" },
		{ AtslKeyword::External, "external" },
		{ AtslKeyword::Option, "option" },
		{ AtslKeyword::Const, "const" },
		{ AtslKeyword::If, "if" },
		{ AtslKeyword::Else, "else" },
		{ AtslKeyword::Struct, "struct" },
		{ AtslKeyword::Return, "return" }
	};

	std::unordered_map<std::string, AtslKeyword> s_strToKeyword =
	{
		{ "input", AtslKeyword::Input },
		{ "output", AtslKeyword::Output },
		{ "external", AtslKeyword::External },
		{ "option", AtslKeyword::Option },
		{ "const", AtslKeyword::Const },
		{ "if", AtslKeyword::If },
		{ "else", AtslKeyword::Else },
		{ "struct", AtslKeyword::Struct },
		{ "return", AtslKeyword::Return }
	};

	/*std::unordered_map<BuiltInFunction, std::string> s_builtInFunctionToStr =
	{
		{ BuiltInFunction::Min, "min" },
		{ BuiltInFunction::Max, "max" },
		{ BuiltInFunction::Cross, "cross" },
		{ BuiltInFunction::Dot, "dot" },
		{ BuiltInFunction::Norm, "norm" },
		{ BuiltInFunction::Sample, "sample" }
	};*/

	std::unordered_map<std::string, BuiltInFunction> s_strToBuiltInFunction =
	{
		{ "min", BuiltInFunction::Min },
		{ "max", BuiltInFunction::Max },
		{ "cross", BuiltInFunction::Cross },
		{ "dot", BuiltInFunction::Dot },
		{ "norm", BuiltInFunction::Norm },
		{ "sample", BuiltInFunction::Sample }
	};

	size_t getComponentCount(char c)
	{
		const size_t count = c - '0';

		if (count < 2 || count > 4)
		{
			ATEMA_ERROR("Component count must be in [2;4]");
		}

		return count;
	}

	PrimitiveType getComponentPrimitiveType(char c)
	{
		switch (c)
		{
			case 'i': return PrimitiveType::Int;
			case 'u': return PrimitiveType::UInt;
			case 'f': return PrimitiveType::Float;
			default:
			{
				ATEMA_ERROR("Invalid primitive type");
			}
		}

		return PrimitiveType::Int;
	}

	ImageType getSamplerImageType(const std::string& str)
	{
		if (str == "1D")
			return ImageType::Texture1D;

		if (str == "2D")
			return ImageType::Texture2D;

		if (str == "3D")
			return ImageType::Texture3D;

		if (str == "Cube")
			return ImageType::Cubemap;

		ATEMA_ERROR("Invalid sampler image type");

		return ImageType::Texture1D;
	}

	PrimitiveType getSamplerPrimitiveType(char c)
	{
		switch (c)
		{
			case 'i': return PrimitiveType::Int;
			case 'u': return PrimitiveType::UInt;
			case 'f': return PrimitiveType::Float;
			default:
			{
				ATEMA_ERROR("Invalid sampler primitive type");
			}
		}

		return PrimitiveType::Int;
	}
}

char atsl::getSymbol(AtslSymbol symbol)
{
	switch (symbol)
	{
		case AtslSymbol::Plus: return '+';
		case AtslSymbol::Minus: return '-';
		case AtslSymbol::Multiply: return '*';
		case AtslSymbol::Divide: return '/';
		case AtslSymbol::Power: return '^';
		case AtslSymbol::Modulo: return '%';
		case AtslSymbol::Equal: return '=';
		case AtslSymbol::Less: return '<';
		case AtslSymbol::Greater: return '>';
		case AtslSymbol::And: return '&';
		case AtslSymbol::Or: return '|';
		case AtslSymbol::Not: return '!';
		case AtslSymbol::Dot: return '.';
		case AtslSymbol::Comma: return ',';
		case AtslSymbol::Colon: return ':';
		case AtslSymbol::Semicolon: return ';';
		case AtslSymbol::LeftBrace: return '{';
		case AtslSymbol::RightBrace: return '}';
		case AtslSymbol::LeftBracket: return '[';
		case AtslSymbol::RightBracket: return ']';
		case AtslSymbol::LeftParenthesis: return '(';
		case AtslSymbol::RightParenthesis: return ')';
		default:
		{
			ATEMA_ERROR("Invalid symbol character");
			break;
		}
	}

	return -1;
}

AtslSymbol atsl::getSymbol(char symbol)
{
	switch (symbol)
	{
		case '+': return AtslSymbol::Plus;
		case '-': return AtslSymbol::Minus;
		case '*': return AtslSymbol::Multiply;
		case '/': return AtslSymbol::Divide;
		case '^': return AtslSymbol::Power;
		case '%': return AtslSymbol::Modulo;
		case '=': return AtslSymbol::Equal;
		case '<': return AtslSymbol::Less;
		case '>': return AtslSymbol::Greater;
		case '&': return AtslSymbol::And;
		case '|': return AtslSymbol::Or;
		case '!': return AtslSymbol::Not;
		case '.': return AtslSymbol::Dot;
		case ',': return AtslSymbol::Comma;
		case ':': return AtslSymbol::Colon;
		case ';': return AtslSymbol::Semicolon;
		case '{': return AtslSymbol::LeftBrace;
		case '}': return AtslSymbol::RightBrace;
		case '[': return AtslSymbol::LeftBracket;
		case ']': return AtslSymbol::RightBracket;
		case '(': return AtslSymbol::LeftParenthesis;
		case ')': return AtslSymbol::RightParenthesis;
		default:
		{
			ATEMA_ERROR("Invalid symbol character");
			break;
		}
	}

	return AtslSymbol::Plus;
}

const std::string& atsl::getKeyword(AtslKeyword keyword)
{
	const auto it = s_keywordToStr.find(keyword);

	if (it == s_keywordToStr.end())
	{
		ATEMA_ERROR("Invalid keyword");

		return "";
	}

	return it->second;
}

AtslKeyword atsl::getKeyword(const std::string& keyword)
{
	const auto it = s_strToKeyword.find(keyword);

	if (it == s_strToKeyword.end())
	{
		ATEMA_ERROR("Invalid keyword");

		return AtslKeyword::Input;
	}

	return it->second;
}

bool atsl::isKeyword(const std::string& keyword)
{
	const auto it = s_strToKeyword.find(keyword);

	return it != s_strToKeyword.end();
}

bool atsl::isType(const std::string& typeStr)
{
	static const std::unordered_set<std::string> s_types =
	{
		"bool", "int", "uint", "float",
		"vec2i", "vec2u", "vec2f",
		"vec3i", "vec3u", "vec3f",
		"vec4i", "vec4u", "vec4f",
		"mat2i", "mat2u", "mat2f",
		"mat3i", "mat3u", "mat3f",
		"mat4i", "mat4u", "mat4f",
		"sampler1Di", "sampler1Du", "sampler1Df",
		"sampler2Di", "sampler2Du", "sampler2Df",
		"sampler3Di", "sampler3Du", "sampler3Df",
		"sampler4Di", "sampler4Du", "sampler4Df",
		"samplerCubei", "samplerCubeu", "samplerCubef"
	};

	return s_types.find(typeStr) != s_types.end();
}

Type atsl::getType(const std::string& typeStr)
{
	if (typeStr == "void")
		return VoidType();

	if (typeStr == "bool")
		return PrimitiveType::Bool;

	if (typeStr == "int")
		return PrimitiveType::Int;

	if (typeStr == "uint")
		return PrimitiveType::UInt;

	if (typeStr == "float")
		return PrimitiveType::Float;

	if (typeStr.compare(0, 3, "vec") && typeStr.size() == 5)
	{
		VectorType type;

		type.componentCount = getComponentCount(typeStr[3]);
		type.primitiveType = getComponentPrimitiveType(typeStr[4]);

		return type;
	}

	if (typeStr.compare(0, 3, "mat") && typeStr.size() == 5)
	{
		MatrixType type;

		type.rowCount = getComponentCount(typeStr[3]);
		type.columnCount = type.rowCount;
		type.primitiveType = getComponentPrimitiveType(typeStr[4]);

		return type;
	}

	if (typeStr.compare(0, 7, "sampler") && typeStr.size() >= 9)
	{
		SamplerType type;

		type.imageType = getSamplerImageType(typeStr.substr(7, typeStr.size() - 2));
		type.primitiveType = getSamplerPrimitiveType(typeStr.back());

		return type;
	}

	return StructType{ typeStr };
}

bool atsl::isBuiltInFunction(const std::string& str)
{
	return s_strToBuiltInFunction.find(str) != s_strToBuiltInFunction.end();
}

BuiltInFunction atsl::getBuiltInFunction(const std::string& str)
{
	const auto it = s_strToBuiltInFunction.find(str);

	if (it == s_strToBuiltInFunction.end())
	{
		ATEMA_ERROR("Invalid built-in function");
	}

	return it->second;
}
