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
#include <Atema/Math/Math.hpp>
#include <Atema/Renderer/BufferLayout.hpp>

using namespace at;

BufferLayout::BufferLayout() :
	BufferLayout(StructLayout::Default)
{
}

BufferLayout::BufferLayout(StructLayout structLayout) :
	m_structLayout(structLayout),
	m_address(0),
	m_largestAlignment(0)
{
}

BufferLayout::~BufferLayout()
{
}

size_t BufferLayout::getAlignment(BufferElementType elementType, StructLayout structLayout)
{
	switch (structLayout)
	{
		case StructLayout::Std140:
		{
			switch (elementType)
			{
				case BufferElementType::Bool:
				case BufferElementType::Int:
				case BufferElementType::UInt:
				case BufferElementType::Float:
					return 4;
				case BufferElementType::Double:
				case BufferElementType::Bool2:
				case BufferElementType::Int2:
				case BufferElementType::UInt2:
				case BufferElementType::Float2:
					return 8;
				case BufferElementType::Double2:
					return 16;
				case BufferElementType::Bool3:
				case BufferElementType::Int3:
				case BufferElementType::UInt3:
				case BufferElementType::Float3:
				case BufferElementType::Bool4:
				case BufferElementType::Int4:
				case BufferElementType::UInt4:
				case BufferElementType::Float4:
					return 16;
				case BufferElementType::Double3:
				case BufferElementType::Double4:
					return 32;
				default:
				{
					ATEMA_ERROR("Invalid BufferElementType");
				}
			}
		}
		default:
		{
			ATEMA_ERROR("Invalid StructLayout");
		}
	}
}

size_t BufferLayout::getArrayAlignment(BufferElementType elementType, StructLayout structLayout)
{
	auto alignment = getAlignment(elementType, structLayout);

	if (structLayout == StructLayout::Std140)
	{
		const auto multiple = getAlignment(BufferElementType::Float4, structLayout);

		alignment = Math::getNextMultiple(alignment, multiple);
	}

	return alignment;
}

size_t BufferLayout::getSize(BufferElementType elementType, StructLayout structLayout)
{
	switch (elementType)
	{
		case BufferElementType::Bool:
		case BufferElementType::Int:
		case BufferElementType::UInt:
		case BufferElementType::Float:
			return 4;
		case BufferElementType::Double:
		case BufferElementType::Bool2:
		case BufferElementType::Int2:
		case BufferElementType::UInt2:
		case BufferElementType::Float2:
			return 8;
		case BufferElementType::Double2:
			return 16;
		case BufferElementType::Bool3:
		case BufferElementType::Int3:
		case BufferElementType::UInt3:
		case BufferElementType::Float3:
			return 12;
		case BufferElementType::Bool4:
		case BufferElementType::Int4:
		case BufferElementType::UInt4:
		case BufferElementType::Float4:
			return 16;
		case BufferElementType::Double3:
		case BufferElementType::Double4:
			return 32;
		default:
		{
			ATEMA_ERROR("Invalid BufferElementType");
		}
	}

	return 0;
}

size_t BufferLayout::add(BufferElementType elementType)
{
	const auto alignment = getAlignment(elementType, m_structLayout);
	const auto size = getSize(elementType, m_structLayout);

	return add(alignment, size);
}

size_t BufferLayout::addArray(BufferElementType elementType, size_t size)
{
	ATEMA_ASSERT(size > 0, "Array size must be greater than zero");

	const auto alignment = getArrayAlignment(elementType, m_structLayout);
	const auto elementSize = getSize(elementType, m_structLayout);

	const auto firstElementAddress = add(alignment, elementSize);

	for (size_t i = 1; i < size; i++)
		add(alignment, elementSize);

	return firstElementAddress;
}

size_t BufferLayout::addMatrix(BufferElementType elementType, size_t rows, size_t columns, bool columnMajor)
{
	ATEMA_ASSERT(elementType == BufferElementType::Int || elementType == BufferElementType::UInt || elementType == BufferElementType::Float || elementType == BufferElementType::Double, "elementType must be one of : Int, UInt, Float, Double");
	ATEMA_ASSERT(1 < rows && rows <= 4, "Rows must be between 2 and 4");
	ATEMA_ASSERT(1 < columns && columns <= 4, "Columns must be between 2 and 4");

	if (columnMajor)
		return addArray(static_cast<BufferElementType>(static_cast<int>(elementType) + rows - 1), columns);

	return addArray(static_cast<BufferElementType>(static_cast<int>(elementType) + columns - 1), rows);
}

size_t BufferLayout::addMatrixArray(BufferElementType elementType, size_t rows, size_t columns, bool columnMajor, size_t size)
{
	ATEMA_ASSERT(size > 0, "Array size must be greater than zero");
	ATEMA_ASSERT(elementType == BufferElementType::Int || elementType == BufferElementType::UInt || elementType == BufferElementType::Float || elementType == BufferElementType::Double, "elementType must be one of : Int, UInt, Float, Double");
	ATEMA_ASSERT(1 < rows && rows <= 4, "Rows must be between 2 and 4");
	ATEMA_ASSERT(1 < columns && columns <= 4, "Columns must be between 2 and 4");

	if (columnMajor)
		return addArray(static_cast<BufferElementType>(static_cast<int>(elementType) + rows - 1), columns * size);

	return addArray(static_cast<BufferElementType>(static_cast<int>(elementType) + columns - 1), rows * size);
}

size_t BufferLayout::addStruct(const BufferLayout& structLayout)
{
	auto alignment = structLayout.m_largestAlignment;

	if (m_structLayout == StructLayout::Std140)
	{
		const auto multiple = getAlignment(BufferElementType::Float4, m_structLayout);

		alignment = Math::getNextMultiple(alignment, multiple);
	}

	return add(alignment, structLayout.getSize());
}

size_t BufferLayout::addStructArray(const BufferLayout& structLayout, size_t size)
{
	ATEMA_ASSERT(size > 0, "Array size must be greater than zero");

	const auto firstElementAddress = addStruct(structLayout);

	for (size_t i = 1; i < size; i++)
		addStruct(structLayout);

	return firstElementAddress;
}

size_t BufferLayout::getSize() const
{
	return m_address;
}

size_t BufferLayout::add(size_t alignment, size_t size)
{
	m_largestAlignment = std::max(alignment, m_largestAlignment);

	const auto address = Math::getNextMultiple(m_address, alignment);

	m_address = address + size;

	return address;
}
