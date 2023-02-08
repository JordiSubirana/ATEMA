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

size_t at::getComponentCount(ImageFormat format)
{
	switch (format)
	{
		// Color (8 bit components)
		case ImageFormat::R8_UNORM:
		case ImageFormat::R8_SNORM:
		case ImageFormat::R8_USCALED:
		case ImageFormat::R8_SSCALED:
		case ImageFormat::R8_UINT:
		case ImageFormat::R8_SINT:
		case ImageFormat::R8_SRGB:
		case ImageFormat::R16_UNORM:
		case ImageFormat::R16_SNORM:
		case ImageFormat::R16_USCALED:
		case ImageFormat::R16_SSCALED:
		case ImageFormat::R16_UINT:
		case ImageFormat::R16_SINT:
		case ImageFormat::R16_SFLOAT:
		case ImageFormat::R32_UINT:
		case ImageFormat::R32_SINT:
		case ImageFormat::R32_SFLOAT:
		case ImageFormat::R64_UINT:
		case ImageFormat::R64_SINT:
		case ImageFormat::R64_SFLOAT:
		case ImageFormat::D16_UNORM:
		case ImageFormat::D32_SFLOAT:
			return 1;
		case ImageFormat::RG8_UNORM:
		case ImageFormat::RG8_SNORM:
		case ImageFormat::RG8_USCALED:
		case ImageFormat::RG8_SSCALED:
		case ImageFormat::RG8_UINT:
		case ImageFormat::RG8_SINT:
		case ImageFormat::RG8_SRGB:
		case ImageFormat::RG16_UNORM:
		case ImageFormat::RG16_SNORM:
		case ImageFormat::RG16_USCALED:
		case ImageFormat::RG16_SSCALED:
		case ImageFormat::RG16_UINT:
		case ImageFormat::RG16_SINT:
		case ImageFormat::RG16_SFLOAT:
		case ImageFormat::RG32_UINT:
		case ImageFormat::RG32_SINT:
		case ImageFormat::RG32_SFLOAT:
		case ImageFormat::RG64_UINT:
		case ImageFormat::RG64_SINT:
		case ImageFormat::RG64_SFLOAT:
		case ImageFormat::D16_UNORM_S8_UINT:
		case ImageFormat::D24_UNORM_S8_UINT:
		case ImageFormat::D32_SFLOAT_S8_UINT:
			return 2;
		case ImageFormat::RGB8_UNORM:
		case ImageFormat::RGB8_SNORM:
		case ImageFormat::RGB8_USCALED:
		case ImageFormat::RGB8_SSCALED:
		case ImageFormat::RGB8_UINT:
		case ImageFormat::RGB8_SINT:
		case ImageFormat::RGB8_SRGB:
		case ImageFormat::BGR8_UNORM:
		case ImageFormat::BGR8_SNORM:
		case ImageFormat::BGR8_USCALED:
		case ImageFormat::BGR8_SSCALED:
		case ImageFormat::BGR8_UINT:
		case ImageFormat::BGR8_SINT:
		case ImageFormat::BGR8_SRGB:
		case ImageFormat::RGB16_UNORM:
		case ImageFormat::RGB16_SNORM:
		case ImageFormat::RGB16_USCALED:
		case ImageFormat::RGB16_SSCALED:
		case ImageFormat::RGB16_UINT:
		case ImageFormat::RGB16_SINT:
		case ImageFormat::RGB16_SFLOAT:
		case ImageFormat::RGB32_UINT:
		case ImageFormat::RGB32_SINT:
		case ImageFormat::RGB32_SFLOAT:
		case ImageFormat::RGB64_UINT:
		case ImageFormat::RGB64_SINT:
		case ImageFormat::RGB64_SFLOAT:
			return 3;
		case ImageFormat::RGBA8_UNORM:
		case ImageFormat::RGBA8_SNORM:
		case ImageFormat::RGBA8_USCALED:
		case ImageFormat::RGBA8_SSCALED:
		case ImageFormat::RGBA8_UINT:
		case ImageFormat::RGBA8_SINT:
		case ImageFormat::RGBA8_SRGB:
		case ImageFormat::BGRA8_UNORM:
		case ImageFormat::BGRA8_SNORM:
		case ImageFormat::BGRA8_USCALED:
		case ImageFormat::BGRA8_SSCALED:
		case ImageFormat::BGRA8_UINT:
		case ImageFormat::BGRA8_SINT:
		case ImageFormat::BGRA8_SRGB:
		case ImageFormat::RGBA16_UNORM:
		case ImageFormat::RGBA16_SNORM:
		case ImageFormat::RGBA16_USCALED:
		case ImageFormat::RGBA16_SSCALED:
		case ImageFormat::RGBA16_UINT:
		case ImageFormat::RGBA16_SINT:
		case ImageFormat::RGBA16_SFLOAT:
		case ImageFormat::RGBA32_UINT:
		case ImageFormat::RGBA32_SINT:
		case ImageFormat::RGBA32_SFLOAT:
		case ImageFormat::RGBA64_UINT:
		case ImageFormat::RGBA64_SINT:
		case ImageFormat::RGBA64_SFLOAT:
			return 4;
		default:
		{
			ATEMA_ERROR("Invalid image format");
		}
	}

	return 0;
}
