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

#include <Atema/Shader/Atsl/AtslUtils.hpp>
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
		{ AtslKeyword::Optional, "optional" },
		{ AtslKeyword::Include, "include" },
		{ AtslKeyword::Const, "const" },
		{ AtslKeyword::If, "if" },
		{ AtslKeyword::Else, "else" },
		{ AtslKeyword::For, "for" },
		{ AtslKeyword::While, "while" },
		{ AtslKeyword::Do, "do" },
		{ AtslKeyword::Struct, "struct" },
		{ AtslKeyword::Break, "break" },
		{ AtslKeyword::Continue, "continue" },
		{ AtslKeyword::Return, "return" },
		{ AtslKeyword::Discard, "discard" }
	};

	std::unordered_map<std::string, AtslKeyword> s_strToKeyword =
	{
		{ "input", AtslKeyword::Input },
		{ "output", AtslKeyword::Output },
		{ "external", AtslKeyword::External },
		{ "option", AtslKeyword::Option },
		{ "optional", AtslKeyword::Optional },
		{ "include", AtslKeyword::Include },
		{ "const", AtslKeyword::Const },
		{ "if", AtslKeyword::If },
		{ "else", AtslKeyword::Else },
		{ "for", AtslKeyword::For },
		{ "while", AtslKeyword::While },
		{ "do", AtslKeyword::Do },
		{ "struct", AtslKeyword::Struct },
		{ "break", AtslKeyword::Break },
		{ "continue", AtslKeyword::Continue },
		{ "return", AtslKeyword::Return },
		{ "discard", AtslKeyword::Discard }
	};

	std::unordered_map<BuiltInFunction, std::string> s_builtInFunctionToStr =
	{
		{ BuiltInFunction::Min, "min" },
		{ BuiltInFunction::Max, "max" },
		{ BuiltInFunction::Cross, "cross" },
		{ BuiltInFunction::Dot, "dot" },
		{ BuiltInFunction::Norm, "norm" },
		{ BuiltInFunction::Normalize, "normalize" },
		{ BuiltInFunction::Sample, "sample" },
		{ BuiltInFunction::SetVertexPosition, "setVertexPosition" },
		{ BuiltInFunction::GetFragmentCoordinates, "getFragmentCoordinates" },
	};

	std::unordered_map<std::string, BuiltInFunction> s_strToBuiltInFunction =
	{
		{ "min", BuiltInFunction::Min },
		{ "max", BuiltInFunction::Max },
		{ "cross", BuiltInFunction::Cross },
		{ "dot", BuiltInFunction::Dot },
		{ "norm", BuiltInFunction::Norm },
		{ "normalize", BuiltInFunction::Normalize },
		{ "sample", BuiltInFunction::Sample },
		{ "setVertexPosition", BuiltInFunction::SetVertexPosition },
		{ "getFragmentCoordinates", BuiltInFunction::GetFragmentCoordinates },
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

	AstPrimitiveType getComponentPrimitiveType(char c)
	{
		switch (c)
		{
			case 'i': return AstPrimitiveType::Int;
			case 'u': return AstPrimitiveType::UInt;
			case 'f': return AstPrimitiveType::Float;
			default:
			{
				ATEMA_ERROR("Invalid primitive type");
			}
		}

		return AstPrimitiveType::Int;
	}

	char getPrimitiveSuffix(AstPrimitiveType type)
	{
		switch (type)
		{
			case AstPrimitiveType::Bool: return 'b';
			case AstPrimitiveType::Int: return 'i';
			case AstPrimitiveType::UInt: return 'u';
			case AstPrimitiveType::Float: return 'f';
			default:
			{
				ATEMA_ERROR("Invalid primitive type");
			}
		}

		return '\0';
	}

	AstImageType getSamplerImageType(const std::string& str)
	{
		if (str == "1D")
			return AstImageType::Texture1D;

		if (str == "2D")
			return AstImageType::Texture2D;

		if (str == "3D")
			return AstImageType::Texture3D;

		if (str == "Cube")
			return AstImageType::Cubemap;

		if (str == "1DArray")
			return AstImageType::TextureArray1D;

		if (str == "2DArray")
			return AstImageType::TextureArray2D;

		ATEMA_ERROR("Invalid sampler image type");

		return AstImageType::Texture1D;
	}

	AstPrimitiveType getSamplerPrimitiveType(char c)
	{
		switch (c)
		{
			case 'i': return AstPrimitiveType::Int;
			case 'u': return AstPrimitiveType::UInt;
			case 'f': return AstPrimitiveType::Float;
			default:
			{
				ATEMA_ERROR("Invalid sampler primitive type");
			}
		}

		return AstPrimitiveType::Int;
	}

	std::string getTypeStr(const AstVoidType& type)
	{
		return "void";
	}

	std::string getTypeStr(const AstPrimitiveType& type)
	{
		switch (type)
		{
			case AstPrimitiveType::Bool: return "bool";
			case AstPrimitiveType::Int: return "int";
			case AstPrimitiveType::UInt: return "uint";
			case AstPrimitiveType::Float: return "float";
			default:
			{
				ATEMA_ERROR("Invalid primitive type");
			}
		}

		return "";
	}

	std::string getTypeStr(const AstVectorType& type)
	{
		return "vec" + std::to_string(type.componentCount) + getPrimitiveSuffix(type.primitiveType);
	}

	std::string getTypeStr(const AstMatrixType& type)
	{
		std::string typeStr = "mat" + std::to_string(type.rowCount);

		if (type.rowCount != type.columnCount)
			typeStr += "x" + std::to_string(type.columnCount);

		return typeStr + getPrimitiveSuffix(type.primitiveType);
	}

	std::string getTypeStr(const AstSamplerType& type)
	{
		std::string typeStr = "sampler";

		switch (type.imageType)
		{
			case AstImageType::Texture1D:
			{
				typeStr += "1D";
				break;
			}
			case AstImageType::Texture2D:
			{
				typeStr += "2D";
				break;
			}
			case AstImageType::Texture3D:
			{
				typeStr += "3D";
				break;
			}
			case AstImageType::Cubemap:
			{
				typeStr += "Cube";
				break;
			}
			case AstImageType::TextureArray1D:
			{
				return typeStr + "1DArray";
			}
			case AstImageType::TextureArray2D:
			{
				return typeStr + "2DArray";
			}
			default:
			{
				ATEMA_ERROR("Invalid image type");
			}
		}

		return typeStr + getPrimitiveSuffix(type.primitiveType);
	}

	std::string getTypeStr(const AstStructType& type)
	{
		return type.name;
	}

	std::string getTypeStr(const AstArrayType& type)
	{
		return atsl::getTypeStr(type.componentType) + "[" + atsl::getArraySizeStr(type) + "]";
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
		case AtslSymbol::Caret: return '^';
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
		case AtslSymbol::QuestionMark: return '?';
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
		case '^': return AtslSymbol::Caret;
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
		case '?': return AtslSymbol::QuestionMark;
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

AstType atsl::getType(const std::string& typeStr)
{
	if (typeStr == "void")
		return AstVoidType();

	if (typeStr == "bool")
		return AstPrimitiveType::Bool;

	if (typeStr == "int")
		return AstPrimitiveType::Int;

	if (typeStr == "uint")
		return AstPrimitiveType::UInt;

	if (typeStr == "float")
		return AstPrimitiveType::Float;

	if (!typeStr.compare(0, 3, "vec") && typeStr.size() == 5)
	{
		AstVectorType type;

		type.componentCount = getComponentCount(typeStr[3]);
		type.primitiveType = getComponentPrimitiveType(typeStr[4]);

		return type;
	}

	if (!typeStr.compare(0, 3, "mat") && typeStr.size() == 5)
	{
		AstMatrixType type;

		type.rowCount = getComponentCount(typeStr[3]);
		type.columnCount = type.rowCount;
		type.primitiveType = getComponentPrimitiveType(typeStr[4]);

		return type;
	}

	if (!typeStr.compare(0, 7, "sampler") && typeStr.size() >= 9)
	{
		AstSamplerType type;

		// Array sampler
		if (typeStr.back() == 'y')
		{
			type.imageType = getSamplerImageType(typeStr.substr(7, typeStr.size() - 7));
			type.primitiveType = AstPrimitiveType::Float;
		}
		else
		{
			type.imageType = getSamplerImageType(typeStr.substr(7, typeStr.size() - 8));
			type.primitiveType = getSamplerPrimitiveType(typeStr.back());
		}

		return type;
	}

	return AstStructType{ typeStr };
}

std::string atsl::getTypeStr(const AstType& type)
{
	if (type.is<AstVoidType>())
		return ::getTypeStr(type.get<AstVoidType>());
	else if (type.is<AstPrimitiveType>())
		return ::getTypeStr(type.get<AstPrimitiveType>());
	else if (type.is<AstArrayType>())
		return ::getTypeStr(type.get<AstArrayType>());
	else if (type.is<AstVectorType>())
		return ::getTypeStr(type.get<AstVectorType>());
	else if (type.is<AstMatrixType>())
		return ::getTypeStr(type.get<AstMatrixType>());
	else if (type.is<AstSamplerType>())
		return ::getTypeStr(type.get<AstSamplerType>());
	else if (type.is<AstStructType>())
		return ::getTypeStr(type.get<AstStructType>());

	ATEMA_ERROR("Invalid type");

	return "";
}

std::string atsl::getTypeStr(const AstArrayType::ComponentType& type)
{
	if (type.is<AstPrimitiveType>())
		return ::getTypeStr(type.get<AstPrimitiveType>());
	else if (type.is<AstVectorType>())
		return ::getTypeStr(type.get<AstVectorType>());
	else if (type.is<AstMatrixType>())
		return ::getTypeStr(type.get<AstMatrixType>());
	else if (type.is<AstSamplerType>())
		return ::getTypeStr(type.get<AstSamplerType>());
	else if (type.is<AstStructType>())
		return ::getTypeStr(type.get<AstStructType>());

	ATEMA_ERROR("Invalid array type");

	return "";
}

ATEMA_SHADER_API std::string atsl::getArraySizeStr(const AstArrayType& type)
{
	std::string sizeStr;

	switch (type.sizeType)
	{
		case AstArrayType::SizeType::Constant:
		{
			sizeStr = std::to_string(type.size);
			break;
		}
		case AstArrayType::SizeType::Implicit:
		{
			// The size is context dependent, no need to specify it
			break;
		}
		case AstArrayType::SizeType::Option:
		{
			sizeStr = type.optionName;
			break;
		}
		default:
		{
			ATEMA_ERROR("Invalid array size type");
		}
	}

	return sizeStr;
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

std::string atsl::getBuiltInFunctionStr(BuiltInFunction function)
{
	const auto it = s_builtInFunctionToStr.find(function);

	if (it == s_builtInFunctionToStr.end())
	{
		ATEMA_ERROR("Invalid built-in function");
	}

	return it->second;
}

AstShaderStage atsl::getShaderStage(const std::string& stage)
{
	if (stage == "vertex")
		return AstShaderStage::Vertex;
	else if (stage == "fragment")
		return AstShaderStage::Fragment;

	ATEMA_ERROR("Invalid shader stage '" + stage + "'");

	return AstShaderStage::Vertex;
}

std::string atsl::getShaderStageStr(AstShaderStage stage)
{
	switch (stage)
	{
		case AstShaderStage::Vertex: return "vertex";
		case AstShaderStage::Fragment: return "fragment";
		default:
		{
			ATEMA_ERROR("Invalid shader stage");
		}
	}

	return "";
}

StructLayout atsl::getStructLayout(const std::string& str)
{
	if (str == "std140")
		return StructLayout::Std140;

	ATEMA_ERROR("Invalid struct layout '" + str + "'");

	return StructLayout::Default;
}

std::string atsl::getStructLayoutStr(StructLayout structLayout)
{
	switch (structLayout)
	{
		case StructLayout::Std140: return "std140";
		default:
		{
			ATEMA_ERROR("Invalid struct layout");
		}
	}

	return "";
}

bool atsl::isExpressionDelimiter(AtslSymbol symbol)
{
	switch (symbol)
	{
		case AtslSymbol::Comma:
		case AtslSymbol::Colon:
		case AtslSymbol::Semicolon:
		case AtslSymbol::RightBrace:
		case AtslSymbol::RightBracket:
		case AtslSymbol::RightParenthesis:
			return true;

		default:
			break;
	}

	return false;
}
