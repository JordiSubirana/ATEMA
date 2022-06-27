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

#ifndef ATEMA_RENDERER_BUFFERLAYOUT_HPP
#define ATEMA_RENDERER_BUFFERLAYOUT_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Renderer/Enums.hpp>
#include <Atema/Shader/Ast/Enums.hpp>

namespace at
{
	class ATEMA_RENDERER_API BufferLayout
	{
	public:
		BufferLayout() = delete;
		BufferLayout(StructLayout structLayout);
		~BufferLayout();

		size_t getAlignment(BufferElementType elementType);
		size_t getSize(BufferElementType elementType);

		size_t add(BufferElementType elementType);
		size_t addArray(BufferElementType elementType, size_t size);
		size_t addMatrix(BufferElementType elementType, size_t rows, size_t columns, bool columnMajor = true);
		size_t addStruct(const BufferLayout& structLayout);
		size_t addStructArray(const BufferLayout& structLayout, size_t size);

		size_t getSize() const;

	private:
		size_t add(size_t alignment, size_t size);

		StructLayout m_structLayout;
		size_t m_address;
		size_t m_largestAlignment;
	};
}

#endif
