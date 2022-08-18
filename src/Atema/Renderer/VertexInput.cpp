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

#include <Atema/Renderer/VertexInput.hpp>
#include <Atema/Core/Error.hpp>

using namespace at;

VertexInput::VertexInput() :
	VertexInput(0, 0, VertexInputFormat::RGB32_SFLOAT)
{
}

VertexInput::VertexInput(uint32_t binding, uint32_t location, VertexInputFormat format) :
	binding(binding),
	location(location),
	format(format)
{
}

size_t VertexInput::getByteSize() const
{
	switch (format)
	{
		case VertexInputFormat::R8_UINT:
		case VertexInputFormat::R8_SINT:
		{
			return 1;
		}
		case VertexInputFormat::RG8_UINT:
		case VertexInputFormat::RG8_SINT:
		{
			return 2;
		}
		case VertexInputFormat::RGB8_UINT:
		case VertexInputFormat::RGB8_SINT:
		{
			return 3;
		}
		case VertexInputFormat::RGBA8_UINT:
		case VertexInputFormat::RGBA8_SINT:
		{
			return 4;
		}
		case VertexInputFormat::R16_UINT:
		case VertexInputFormat::R16_SINT:
		case VertexInputFormat::R16_SFLOAT:
		{
			return 2;
		}
		case VertexInputFormat::RG16_UINT:
		case VertexInputFormat::RG16_SINT:
		case VertexInputFormat::RG16_SFLOAT:
		{
			return 4;
		}
		case VertexInputFormat::RGB16_UINT:
		case VertexInputFormat::RGB16_SINT:
		case VertexInputFormat::RGB16_SFLOAT:
		{
			return 6;
		}
		case VertexInputFormat::RGBA16_UINT:
		case VertexInputFormat::RGBA16_SINT:
		case VertexInputFormat::RGBA16_SFLOAT:
		{
			return 8;
		}
		case VertexInputFormat::R32_UINT:
		case VertexInputFormat::R32_SINT:
		case VertexInputFormat::R32_SFLOAT:
		{
			return 4;
		}
		case VertexInputFormat::RG32_UINT:
		case VertexInputFormat::RG32_SINT:
		case VertexInputFormat::RG32_SFLOAT:
		{
			return 8;
		}
		case VertexInputFormat::RGB32_UINT:
		case VertexInputFormat::RGB32_SINT:
		case VertexInputFormat::RGB32_SFLOAT:
		{
			return 12;
		}
		case VertexInputFormat::RGBA32_UINT:
		case VertexInputFormat::RGBA32_SINT:
		case VertexInputFormat::RGBA32_SFLOAT:
		{
			return 16;
		}
		case VertexInputFormat::R64_UINT:
		case VertexInputFormat::R64_SINT:
		case VertexInputFormat::R64_SFLOAT:
		{
			return 8;
		}
		case VertexInputFormat::RG64_UINT:
		case VertexInputFormat::RG64_SINT:
		case VertexInputFormat::RG64_SFLOAT:
		{
			return 16;
		}
		case VertexInputFormat::RGB64_UINT:
		case VertexInputFormat::RGB64_SINT:
		case VertexInputFormat::RGB64_SFLOAT:
		{
			return 24;
		}
		case VertexInputFormat::RGBA64_UINT:
		case VertexInputFormat::RGBA64_SINT:
		case VertexInputFormat::RGBA64_SFLOAT:
		{
			return 32;
		}
		default:
		{
			ATEMA_ERROR("Invalid vertex format");
		}
	}

	return 0;
}

size_t VertexInput::getElementSize() const
{
	switch (format)
	{
		case VertexInputFormat::R8_UINT:
		case VertexInputFormat::R8_SINT:
		case VertexInputFormat::R16_UINT:
		case VertexInputFormat::R16_SINT:
		case VertexInputFormat::R16_SFLOAT:
		case VertexInputFormat::R32_UINT:
		case VertexInputFormat::R32_SINT:
		case VertexInputFormat::R32_SFLOAT:
		case VertexInputFormat::R64_UINT:
		case VertexInputFormat::R64_SINT:
		case VertexInputFormat::R64_SFLOAT:
			return 1;
		case VertexInputFormat::RG8_UINT:
		case VertexInputFormat::RG8_SINT:
		case VertexInputFormat::RG16_UINT:
		case VertexInputFormat::RG16_SINT:
		case VertexInputFormat::RG16_SFLOAT:
		case VertexInputFormat::RG32_UINT:
		case VertexInputFormat::RG32_SINT:
		case VertexInputFormat::RG32_SFLOAT:
		case VertexInputFormat::RG64_UINT:
		case VertexInputFormat::RG64_SINT:
		case VertexInputFormat::RG64_SFLOAT:
			return 2;
		case VertexInputFormat::RGB8_UINT:
		case VertexInputFormat::RGB8_SINT:
		case VertexInputFormat::RGB16_UINT:
		case VertexInputFormat::RGB16_SINT:
		case VertexInputFormat::RGB16_SFLOAT:
		case VertexInputFormat::RGB32_UINT:
		case VertexInputFormat::RGB32_SINT:
		case VertexInputFormat::RGB32_SFLOAT:
		case VertexInputFormat::RGB64_UINT:
		case VertexInputFormat::RGB64_SINT:
		case VertexInputFormat::RGB64_SFLOAT:
			return 3;
		case VertexInputFormat::RGBA8_UINT:
		case VertexInputFormat::RGBA8_SINT:
		case VertexInputFormat::RGBA16_UINT:
		case VertexInputFormat::RGBA16_SINT:
		case VertexInputFormat::RGBA16_SFLOAT:
		case VertexInputFormat::RGBA32_UINT:
		case VertexInputFormat::RGBA32_SINT:
		case VertexInputFormat::RGBA32_SFLOAT:
		case VertexInputFormat::RGBA64_UINT:
		case VertexInputFormat::RGBA64_SINT:
		case VertexInputFormat::RGBA64_SFLOAT:
			return 4;
		default:
			ATEMA_ERROR("Invalid vertex format");
	}

	return 0;
}

size_t VertexInput::getElementByteSize() const
{
	return getByteSize() / getElementSize();
}

bool VertexInput::isFloatingPoint() const
{
	switch (format)
	{
		case VertexInputFormat::R16_SFLOAT:
		case VertexInputFormat::RG16_SFLOAT:
		case VertexInputFormat::RGB16_SFLOAT:
		case VertexInputFormat::RGBA16_SFLOAT:
		case VertexInputFormat::R32_SFLOAT:
		case VertexInputFormat::RG32_SFLOAT:
		case VertexInputFormat::RGB32_SFLOAT:
		case VertexInputFormat::RGBA32_SFLOAT:
		case VertexInputFormat::R64_SFLOAT:
		case VertexInputFormat::RG64_SFLOAT:
		case VertexInputFormat::RGB64_SFLOAT:
		case VertexInputFormat::RGBA64_SFLOAT:
			return true;
	}

	return false;
}
