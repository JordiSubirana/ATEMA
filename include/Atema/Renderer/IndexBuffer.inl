/*
	Copyright 2017 Jordi SUBIRANA

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

#ifndef ATEMA_RENDERER_INDEXBUFFER_INL
#define ATEMA_RENDERER_INDEXBUFFER_INL

#include <Atema/Renderer/IndexBuffer.hpp>
#include <Atema/Core/Error.hpp>

namespace at
{
	namespace detail
	{
		template <typename T>
		struct IndexTypeToEnum {};

		template <> struct IndexTypeToEnum<unsigned char> { static constexpr IndexBuffer::Type value = IndexBuffer::Type::UnsignedByte; };
		template <> struct IndexTypeToEnum<unsigned short> { static constexpr IndexBuffer::Type value = IndexBuffer::Type::UnsignedShort; };
		template <> struct IndexTypeToEnum<unsigned> { static constexpr IndexBuffer::Type value = IndexBuffer::Type::UnsignedInt; };
	}
	
	template <typename T>
	void IndexBuffer::setData(const std::vector<T>& indices)
	{
		static_assert(!(std::is_literal_type<T>::value && !std::is_signed<T>::value), "Indices type must be an unsigned literal.");

		setData(indices.data(), sizeof(T)*indices.size(), detail::IndexTypeToEnum<T>::value);
	}
	
	template <typename T>
	void IndexBuffer::getData(std::vector<T>& indices)
	{
		static_assert(!(std::is_literal_type<T>::value && !std::is_signed<T>::value), "Indices type must be an unsigned literal.");

		if (getByteSize() != sizeof(T))
			ATEMA_ERROR("The index format doesn't match with the type.");

		indices.resize(getSize());

		getData(indices.data());
	}
}

#endif