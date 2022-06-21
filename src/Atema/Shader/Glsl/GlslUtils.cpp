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
	std::string getPrimitivePrefix(PrimitiveType type)
	{
		switch (type)
		{
			case PrimitiveType::Bool: return "b";
			case PrimitiveType::Int: return "i";
			case PrimitiveType::UInt: return "u";
			case PrimitiveType::Float: return "";
			default:
			{
				ATEMA_ERROR("Invalid primitive type");
			}
		}

		return "";
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
		return getPrimitivePrefix(type.primitiveType) + "vec" + std::to_string(type.componentCount);
	}

	std::string getTypeStr(const MatrixType& type)
	{
		std::string typeStr = "mat" + std::to_string(type.rowCount);

		if (type.rowCount != type.columnCount)
			typeStr += "x" + std::to_string(type.columnCount);

		return getPrimitivePrefix(type.primitiveType) + typeStr;
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
			default:
			{
				ATEMA_ERROR("Invalid image type");
			}
		}

		return getPrimitivePrefix(type.primitiveType) + typeStr;
	}

	std::string getTypeStr(const StructType& type)
	{
		return type.name;
	}

	std::string getTypeStr(const ArrayType& type)
	{
		const std::string typeStr = glsl::getTypeStr(type.componentType);

		std::string countStr;

		if (type.size != ArrayType::ImplicitSize)
			countStr = std::to_string(type.size);

		return typeStr + "[" + countStr + "]";
	}
}

std::string glsl::getTypeStr(const Type& type)
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

std::string glsl::getTypeStr(const ArrayType::ComponentType& type)
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
