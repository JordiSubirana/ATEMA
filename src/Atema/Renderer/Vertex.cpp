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

#include <Atema/Renderer/Vertex.hpp>

using namespace at;


VertexAttribute::VertexAttribute(Role role, Format format) :
	VertexAttribute(static_cast<unsigned>(role), format)
{
}

VertexAttribute::VertexAttribute(unsigned role, Format format) :
	role(role),
	format(format)
{
}

size_t VertexAttribute::getByteSize() const
{
	switch (format)
	{
		case Format::R8_UINT:
		case Format::R8_SINT:
		{
			return 1;
		}
		case Format::RG8_UINT:
		case Format::RG8_SINT:
		{
			return 2;
		}
		case Format::RGB8_UINT:
		case Format::RGB8_SINT:
		{
			return 3;
		}
		case Format::RGBA8_UINT:
		case Format::RGBA8_SINT:
		{
			return 4;
		}
		case Format::R16_UINT:
		case Format::R16_SINT:
		case Format::R16_SFLOAT:
		{
			return 2;
		}
		case Format::RG16_UINT:
		case Format::RG16_SINT:
		case Format::RG16_SFLOAT:
		{
			return 4;
		}
		case Format::RGB16_UINT:
		case Format::RGB16_SINT:
		case Format::RGB16_SFLOAT:
		{
			return 6;
		}
		case Format::RGBA16_UINT:
		case Format::RGBA16_SINT:
		case Format::RGBA16_SFLOAT:
		{
			return 8;
		}
		case Format::R32_UINT:
		case Format::R32_SINT:
		case Format::R32_SFLOAT:
		{
			return 4;
		}
		case Format::RG32_UINT:
		case Format::RG32_SINT:
		case Format::RG32_SFLOAT:
		{
			return 8;
		}
		case Format::RGB32_UINT:
		case Format::RGB32_SINT:
		case Format::RGB32_SFLOAT:
		{
			return 12;
		}
		case Format::RGBA32_UINT:
		case Format::RGBA32_SINT:
		case Format::RGBA32_SFLOAT:
		{
			return 16;
		}
		case Format::R64_UINT:
		case Format::R64_SINT:
		case Format::R64_SFLOAT:
		{
			return 8;
		}
		case Format::RG64_UINT:
		case Format::RG64_SINT:
		case Format::RG64_SFLOAT:
		{
			return 16;
		}
		case Format::RGB64_UINT:
		case Format::RGB64_SINT:
		case Format::RGB64_SFLOAT:
		{
			return 24;
		}
		case Format::RGBA64_UINT:
		case Format::RGBA64_SINT:
		case Format::RGBA64_SFLOAT:
		{
			return 32;
		}
		default:
		{
			ATEMA_ERROR("Invalid attribute type");
		}
	}

	return 0;
}

VertexInput::VertexInput() :
	binding(0),
	location(0)
{
}

VertexInput::VertexInput(uint32_t binding, uint32_t location) :
	binding(binding),
	location(location)
{
}
