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

#include <Atema/Renderer/Utils.hpp>
#include <Atema/Core/Error.hpp>

using namespace at;

VertexInputFormat at::getVertexFormat(const Type& astVariableType)
{
	if (astVariableType.is<PrimitiveType>())
	{
		switch (astVariableType.get<PrimitiveType>())
		{
			case PrimitiveType::Int: return VertexInputFormat::R32_SINT;
			case PrimitiveType::UInt: return VertexInputFormat::R32_UINT;
			case PrimitiveType::Float: return VertexInputFormat::R32_SFLOAT;
			default: break;
		}
	}
	else if (astVariableType.is<VectorType>())
	{
		const auto& type = astVariableType.get<VectorType>();

		switch (type.primitiveType)
		{
			case PrimitiveType::Int:
			{
				switch (type.componentCount)
				{
					case 2: return VertexInputFormat::RG32_SINT;
					case 3: return VertexInputFormat::RGB32_SINT;
					case 4: return VertexInputFormat::RGBA32_SINT;
					default: break;
				}

				break;
			}
			case PrimitiveType::UInt:
			{
				switch (type.componentCount)
				{
					case 2: return VertexInputFormat::RG32_UINT;
					case 3: return VertexInputFormat::RGB32_UINT;
					case 4: return VertexInputFormat::RGBA32_UINT;
					default: break;
				}

				break;
			}
			case PrimitiveType::Float:
			{
				switch (type.componentCount)
				{
					case 2: return VertexInputFormat::RG32_SFLOAT;
					case 3: return VertexInputFormat::RGB32_SFLOAT;
					case 4: return VertexInputFormat::RGBA32_SFLOAT;
					default: break;
				}

				break;
			}
			default: break;
		}
	}

	ATEMA_ERROR("Invalid vertex input type");

	return VertexInputFormat::R32_SINT;
}

DescriptorType at::getDefaultDescriptorType(const Type& astVariableType)
{
	if (astVariableType.is<SamplerType>())
	{
		return DescriptorType::CombinedImageSampler;
	}
	else if (astVariableType.isOneOf<PrimitiveType, VectorType, MatrixType, StructType>())
	{
		return DescriptorType::UniformBuffer;
	}
	else if (astVariableType.is<ArrayType>())
	{
		return getDefaultDescriptorType(astVariableType.get<ArrayType>().componentType);
	}

	ATEMA_ERROR("Invalid variable type");

	return DescriptorType::UniformBuffer;
}

DescriptorType at::getDefaultDescriptorType(const ArrayType::ComponentType& astVariableType)
{
	if (astVariableType.is<SamplerType>())
	{
		return DescriptorType::CombinedImageSampler;
	}
	else if (astVariableType.isOneOf<PrimitiveType, VectorType, MatrixType, StructType>())
	{
		return DescriptorType::UniformBuffer;
	}

	ATEMA_ERROR("Invalid variable type");

	return DescriptorType::UniformBuffer;
}

uint32_t at::getDescriptorBindingCount(const Type& astVariableType)
{
	if (astVariableType.is<ArrayType>())
		return static_cast<uint32_t>(astVariableType.get<ArrayType>().size);

	return 1;
}

size_t at::getByteSize(IndexType indexType)
{
	switch (indexType)
	{
		case IndexType::U16: return 2;
		case IndexType::U32: return 4;
	}

	ATEMA_ERROR("Invalid index type");

	return 0;
}
