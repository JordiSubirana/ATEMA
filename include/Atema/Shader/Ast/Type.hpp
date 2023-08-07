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

#ifndef ATEMA_SHADER_AST_TYPE_HPP
#define ATEMA_SHADER_AST_TYPE_HPP

#include <Atema/Shader/Config.hpp>

#include <string>
#include <Atema/Core/Variant.hpp>
#include <vector>

namespace at
{
	struct AstVoidType
	{
		
	};

	enum class AstPrimitiveType
	{
		Bool,
		Int,
		UInt,
		Float
	};

	struct AstVectorType
	{
		AstPrimitiveType primitiveType;
		size_t componentCount;
	};

	struct AstMatrixType
	{
		AstPrimitiveType primitiveType;
		size_t rowCount;
		size_t columnCount;
	};

	enum class AstImageType
	{
		Texture1D,
		Texture2D,
		Texture3D,
		Cubemap,
		TextureArray1D,
		TextureArray2D
	};
	
	struct AstSamplerType
	{
		AstImageType imageType;
		AstPrimitiveType primitiveType;
	};

	struct AstStructType
	{
		std::string name;
	};

	struct AstArrayType
	{
		using ComponentType = Variant<AstPrimitiveType, AstVectorType, AstMatrixType, AstSamplerType, AstStructType>;

		enum class SizeType
		{
			// The size is a literal constant
			Constant,
			// The size is context dependent
			Implicit,
			// The size is an optional value and needs preprocessing to be evaluated
			Option
		};

		ComponentType componentType;
		SizeType sizeType = SizeType::Implicit;
		// Constant size, valid for SizeType::Constant
		size_t size = 0;
		// Option size, valid for SizeType::Option
		std::string optionName;
	};

	using AstType = Variant<
		AstVoidType,
		AstPrimitiveType,
		AstArrayType,
		AstVectorType,
		AstMatrixType,
		AstSamplerType,
		AstStructType>;

	ATEMA_SHADER_API bool isInOutType(const AstType& type);
	ATEMA_SHADER_API bool isReturnType(const AstType& type);
}

#endif
