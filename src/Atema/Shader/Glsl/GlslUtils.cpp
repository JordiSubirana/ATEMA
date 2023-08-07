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

#include <Atema/Core/Error.hpp>
#include <Atema/Shader/Glsl/GlslUtils.hpp>

using namespace at;
using namespace glsl;

namespace
{
	std::string getPrimitivePrefix(AstPrimitiveType type)
	{
		switch (type)
		{
			case AstPrimitiveType::Bool: return "b";
			case AstPrimitiveType::Int: return "i";
			case AstPrimitiveType::UInt: return "u";
			case AstPrimitiveType::Float: return "";
			default:
			{
				ATEMA_ERROR("Invalid primitive type");
			}
		}

		return "";
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
		return getPrimitivePrefix(type.primitiveType) + "vec" + std::to_string(type.componentCount);
	}

	std::string getTypeStr(const AstMatrixType& type)
	{
		std::string typeStr = "mat" + std::to_string(type.rowCount);

		if (type.rowCount != type.columnCount)
			typeStr += "x" + std::to_string(type.columnCount);

		return getPrimitivePrefix(type.primitiveType) + typeStr;
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

		return getPrimitivePrefix(type.primitiveType) + typeStr;
	}

	std::string getTypeStr(const AstStructType& type)
	{
		return type.name;
	}

	std::string getTypeStr(const AstArrayType& type)
	{
		return glsl::getTypeStr(type.componentType) + "[" + glsl::getArraySizeStr(type) + "]";
	}
}

std::string glsl::getTypeStr(const AstType& type)
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

std::string glsl::getTypeStr(const AstArrayType::ComponentType& type)
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

ATEMA_SHADER_API std::string glsl::getArraySizeStr(const AstArrayType& type)
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

std::string glsl::getStructLayoutStr(StructLayout structLayout)
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
