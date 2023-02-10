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
		{ AtslKeyword::Const, "const" },
		{ AtslKeyword::If, "if" },
		{ AtslKeyword::Else, "else" },
		{ AtslKeyword::For, "for" },
		{ AtslKeyword::While, "while" },
		{ AtslKeyword::Do, "do" },
		{ AtslKeyword::Struct, "struct" },
		{ AtslKeyword::Break, "break" },
		{ AtslKeyword::Continue, "continue" },
		{ AtslKeyword::Return, "return" }
	};

	std::unordered_map<std::string, AtslKeyword> s_strToKeyword =
	{
		{ "input", AtslKeyword::Input },
		{ "output", AtslKeyword::Output },
		{ "external", AtslKeyword::External },
		{ "option", AtslKeyword::Option },
		{ "optional", AtslKeyword::Optional },
		{ "const", AtslKeyword::Const },
		{ "if", AtslKeyword::If },
		{ "else", AtslKeyword::Else },
		{ "for", AtslKeyword::For },
		{ "while", AtslKeyword::While },
		{ "do", AtslKeyword::Do },
		{ "struct", AtslKeyword::Struct },
		{ "break", AtslKeyword::Break },
		{ "continue", AtslKeyword::Continue },
		{ "return", AtslKeyword::Return }
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

	char getPrimitiveSuffix(PrimitiveType type)
	{
		switch (type)
		{
			case PrimitiveType::Bool: return 'b';
			case PrimitiveType::Int: return 'i';
			case PrimitiveType::UInt: return 'u';
			case PrimitiveType::Float: return 'f';
			default:
			{
				ATEMA_ERROR("Invalid primitive type");
			}
		}

		return '\0';
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

		if (str == "1DArray")
			return ImageType::TextureArray1D;

		if (str == "2DArray")
			return ImageType::TextureArray2D;

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

	std::string getTypeStr(const VoidType& type)
	{
		return "void";
	}

	std::string getTypeStr(const PrimitiveType& type)
	{
		switch (type)
		{
			case PrimitiveType::Bool: return "bool";
			case PrimitiveType::Int: return "int";
			case PrimitiveType::UInt: return "uint";
			case PrimitiveType::Float: return "float";
			default:
			{
				ATEMA_ERROR("Invalid primitive type");
			}
		}

		return "";
	}

	std::string getTypeStr(const VectorType& type)
	{
		return "vec" + std::to_string(type.componentCount) + getPrimitiveSuffix(type.primitiveType);
	}

	std::string getTypeStr(const MatrixType& type)
	{
		std::string typeStr = "mat" + std::to_string(type.rowCount);

		if (type.rowCount != type.columnCount)
			typeStr += "x" + std::to_string(type.columnCount);

		return typeStr + getPrimitiveSuffix(type.primitiveType);
	}

	std::string getTypeStr(const SamplerType& type)
	{
		std::string typeStr = "sampler";

		switch (type.imageType)
		{
			case ImageType::Texture1D:
			{
				typeStr += "1D";
				break;
			}
			case ImageType::Texture2D:
			{
				typeStr += "2D";
				break;
			}
			case ImageType::Texture3D:
			{
				typeStr += "3D";
				break;
			}
			case ImageType::Cubemap:
			{
				typeStr += "Cube";
				break;
			}
			case ImageType::TextureArray1D:
			{
				return typeStr + "1DArray";
			}
			case ImageType::TextureArray2D:
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

	std::string getTypeStr(const StructType& type)
	{
		return type.name;
	}

	std::string getTypeStr(const ArrayType& type)
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

	if (!typeStr.compare(0, 3, "vec") && typeStr.size() == 5)
	{
		VectorType type;

		type.componentCount = getComponentCount(typeStr[3]);
		type.primitiveType = getComponentPrimitiveType(typeStr[4]);

		return type;
	}

	if (!typeStr.compare(0, 3, "mat") && typeStr.size() == 5)
	{
		MatrixType type;

		type.rowCount = getComponentCount(typeStr[3]);
		type.columnCount = type.rowCount;
		type.primitiveType = getComponentPrimitiveType(typeStr[4]);

		return type;
	}

	if (!typeStr.compare(0, 7, "sampler") && typeStr.size() >= 9)
	{
		SamplerType type;

		// Array sampler
		if (typeStr.back() == 'y')
		{
			type.imageType = getSamplerImageType(typeStr.substr(7, typeStr.size() - 7));
			type.primitiveType = PrimitiveType::Float;
		}
		else
		{
			type.imageType = getSamplerImageType(typeStr.substr(7, typeStr.size() - 8));
			type.primitiveType = getSamplerPrimitiveType(typeStr.back());
		}

		return type;
	}

	return StructType{ typeStr };
}

std::string atsl::getTypeStr(const Type& type)
{
	if (type.is<VoidType>())
		return ::getTypeStr(type.get<VoidType>());
	else if (type.is<PrimitiveType>())
		return ::getTypeStr(type.get<PrimitiveType>());
	else if (type.is<ArrayType>())
		return ::getTypeStr(type.get<ArrayType>());
	else if (type.is<VectorType>())
		return ::getTypeStr(type.get<VectorType>());
	else if (type.is<MatrixType>())
		return ::getTypeStr(type.get<MatrixType>());
	else if (type.is<SamplerType>())
		return ::getTypeStr(type.get<SamplerType>());
	else if (type.is<StructType>())
		return ::getTypeStr(type.get<StructType>());

	ATEMA_ERROR("Invalid type");

	return "";
}

std::string atsl::getTypeStr(const ArrayType::ComponentType& type)
{
	if (type.is<PrimitiveType>())
		return ::getTypeStr(type.get<PrimitiveType>());
	else if (type.is<VectorType>())
		return ::getTypeStr(type.get<VectorType>());
	else if (type.is<MatrixType>())
		return ::getTypeStr(type.get<MatrixType>());
	else if (type.is<SamplerType>())
		return ::getTypeStr(type.get<SamplerType>());
	else if (type.is<StructType>())
		return ::getTypeStr(type.get<StructType>());

	ATEMA_ERROR("Invalid array type");

	return "";
}

ATEMA_SHADER_API std::string atsl::getArraySizeStr(const ArrayType& type)
{
	std::string sizeStr;

	switch (type.sizeType)
	{
		case ArrayType::SizeType::Constant:
		{
			sizeStr = std::to_string(type.size);
			break;
		}
		case ArrayType::SizeType::Implicit:
		{
			// The size is context dependent, no need to specify it
			break;
		}
		case ArrayType::SizeType::Option:
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
