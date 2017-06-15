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

#include <Atema/Renderer/VertexBuffer.hpp>
#include <Atema/Renderer/RenderSystem.hpp>

namespace at
{
	VertexBuffer::Implementation::Implementation()
	{
		
	}

	VertexBuffer::VertexBuffer() : m_impl(nullptr)
	{
		RenderSystem *renderer = RenderSystem::getCurrent();

		if (!renderer)
			ATEMA_ERROR("There is no current RenderSystem.");

		m_impl = renderer->createVertexBuffer();

		setDrawMode(DrawMode::Triangles);
	}

	VertexBuffer::~VertexBuffer()
	{

	}

	VertexBuffer::Implementation* VertexBuffer::getImplementation()
	{
		return m_impl;
	}

	const VertexBuffer::Implementation* VertexBuffer::getImplementation() const
	{
		return m_impl;
	}

	void VertexBuffer::setData(const void* vertices, size_t byteSize, const VertexFormat& format)
	{
		m_impl->setData(vertices, byteSize, format);
	}

	void VertexBuffer::getData(void* vertices)
	{
		m_impl->getData(vertices);
	}

	void* VertexBuffer::map()
	{
		return m_impl->map();
	}

	void VertexBuffer::setDrawMode(DrawMode mode)
	{
		m_impl->setDrawMode(mode);
	}

	size_t VertexBuffer::getSize() const
	{
		return m_impl->getSize();
	}

	size_t VertexBuffer::getByteSize() const
	{
		return m_impl->getByteSize();
	}

	size_t VertexBuffer::getTotalByteSize() const
	{
		return getSize()*getByteSize();
	}

	VertexFormat& VertexBuffer::getFormat()
	{
		return m_impl->getFormat();
	}

	const VertexFormat& VertexBuffer::getFormat() const
	{
		return m_impl->getFormat();
	}
}
