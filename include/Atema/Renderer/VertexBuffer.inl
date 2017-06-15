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

#ifndef ATEMA_RENDERER_VERTEXBUFFER_INL
#define ATEMA_RENDERER_VERTEXBUFFER_INL

#include <Atema/Renderer/VertexBuffer.hpp>
#include <Atema/Core/Error.hpp>

namespace at
{
	template <typename T>
	void VertexBuffer::setData(const std::vector<T>& vertices, const VertexFormat& format)
	{
		auto i = sizeof(T);
		auto j = format.getByteSize();
		if (format.getByteSize() != sizeof(T))
			ATEMA_ERROR("The vertex format doesn't match with the type.");

		setData(vertices.data(), sizeof(T)*vertices.size(), format);
	}
	
	template <typename T>
	void VertexBuffer::getData(std::vector<T>& vertices)
	{
		if (getFormat().getByteSize() != sizeof(T))
			ATEMA_ERROR("The vertex format doesn't match with the type.");

		vertices.resize(getSize());

		getData(vertices.data());
	}

	template <typename T>
	T* VertexBuffer::map()
	{
		return reinterpret_cast<T*>(map());
	}
}

#endif