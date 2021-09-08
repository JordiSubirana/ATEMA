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

VertexInput::VertexInput(uint32_t binding, uint32_t location, VertexFormat format) :
	binding(binding),
	location(location),
	format(format)
{
}

size_t VertexInput::getByteSize() const
{
	switch (format)
	{
		case VertexFormat::R8_UINT:
		case VertexFormat::R8_SINT:
		{
			return 1;
		}
		case VertexFormat::RG8_UINT:
		case VertexFormat::RG8_SINT:
		{
			return 2;
		}
		case VertexFormat::RGB8_UINT:
		case VertexFormat::RGB8_SINT:
		{
			return 3;
		}
		case VertexFormat::RGBA8_UINT:
		case VertexFormat::RGBA8_SINT:
		{
			return 4;
		}
		case VertexFormat::R16_UINT:
		case VertexFormat::R16_SINT:
		case VertexFormat::R16_SFLOAT:
		{
			return 2;
		}
		case VertexFormat::RG16_UINT:
		case VertexFormat::RG16_SINT:
		case VertexFormat::RG16_SFLOAT:
		{
			return 4;
		}
		case VertexFormat::RGB16_UINT:
		case VertexFormat::RGB16_SINT:
		case VertexFormat::RGB16_SFLOAT:
		{
			return 6;
		}
		case VertexFormat::RGBA16_UINT:
		case VertexFormat::RGBA16_SINT:
		case VertexFormat::RGBA16_SFLOAT:
		{
			return 8;
		}
		case VertexFormat::R32_UINT:
		case VertexFormat::R32_SINT:
		case VertexFormat::R32_SFLOAT:
		{
			return 4;
		}
		case VertexFormat::RG32_UINT:
		case VertexFormat::RG32_SINT:
		case VertexFormat::RG32_SFLOAT:
		{
			return 8;
		}
		case VertexFormat::RGB32_UINT:
		case VertexFormat::RGB32_SINT:
		case VertexFormat::RGB32_SFLOAT:
		{
			return 12;
		}
		case VertexFormat::RGBA32_UINT:
		case VertexFormat::RGBA32_SINT:
		case VertexFormat::RGBA32_SFLOAT:
		{
			return 16;
		}
		case VertexFormat::R64_UINT:
		case VertexFormat::R64_SINT:
		case VertexFormat::R64_SFLOAT:
		{
			return 8;
		}
		case VertexFormat::RG64_UINT:
		case VertexFormat::RG64_SINT:
		case VertexFormat::RG64_SFLOAT:
		{
			return 16;
		}
		case VertexFormat::RGB64_UINT:
		case VertexFormat::RGB64_SINT:
		case VertexFormat::RGB64_SFLOAT:
		{
			return 24;
		}
		case VertexFormat::RGBA64_UINT:
		case VertexFormat::RGBA64_SINT:
		case VertexFormat::RGBA64_SFLOAT:
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
