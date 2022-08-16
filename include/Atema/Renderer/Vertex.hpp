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

#ifndef ATEMA_RENDERER_VERTEX_HPP
#define ATEMA_RENDERER_VERTEX_HPP

#include <Atema/Renderer/Config.hpp>

namespace at
{
	enum class VertexInputFormat
	{
		R8_UINT,
		R8_SINT,

		RG8_UINT,
		RG8_SINT,

		RGB8_UINT,
		RGB8_SINT,

		RGBA8_UINT,
		RGBA8_SINT,

		R16_UINT,
		R16_SINT,
		R16_SFLOAT,

		RG16_UINT,
		RG16_SINT,
		RG16_SFLOAT,

		RGB16_UINT,
		RGB16_SINT,
		RGB16_SFLOAT,

		RGBA16_UINT,
		RGBA16_SINT,
		RGBA16_SFLOAT,

		R32_UINT,
		R32_SINT,
		R32_SFLOAT,

		RG32_UINT,
		RG32_SINT,
		RG32_SFLOAT,

		RGB32_UINT,
		RGB32_SINT,
		RGB32_SFLOAT,

		RGBA32_UINT,
		RGBA32_SINT,
		RGBA32_SFLOAT,

		R64_UINT,
		R64_SINT,
		R64_SFLOAT,

		RG64_UINT,
		RG64_SINT,
		RG64_SFLOAT,

		RGB64_UINT,
		RGB64_SINT,
		RGB64_SFLOAT,

		RGBA64_UINT,
		RGBA64_SINT,
		RGBA64_SFLOAT
	};

	struct ATEMA_RENDERER_API VertexInput
	{
		VertexInput() = delete;
		VertexInput(uint32_t binding, uint32_t location, VertexInputFormat format);

		size_t getByteSize() const;

		uint32_t binding;
		uint32_t location;
		VertexInputFormat format;
	};
}

#endif
