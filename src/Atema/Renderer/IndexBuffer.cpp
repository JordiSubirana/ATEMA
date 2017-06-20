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

#include <Atema/Renderer/IndexBuffer.hpp>
#include <Atema/Renderer/RenderSystem.hpp>

namespace at
{
	IndexBuffer::Implementation::Implementation()
	{

	}

	IndexBuffer::IndexBuffer() : IndexBuffer(RenderSystem::getCurrent())
	{
	}

	IndexBuffer::IndexBuffer(RenderSystem *system) : m_impl(nullptr)
	{
		if (!system)
			ATEMA_ERROR("Invalid RenderSystem.");

		m_impl = system->createIndexBuffer();
	}

	IndexBuffer::~IndexBuffer()
	{
		delete m_impl;
	}

	IndexBuffer::Implementation* IndexBuffer::getImplementation()
	{
		return m_impl;
	}

	const IndexBuffer::Implementation* IndexBuffer::getImplementation() const
	{
		return m_impl;
	}

	void IndexBuffer::setData(const void* indices, size_t byteSize, Type type)
	{
		m_impl->setData(indices, byteSize, type);
	}

	void IndexBuffer::getData(void* indices)
	{
		m_impl->getData(indices);
	}

	size_t IndexBuffer::getSize() const
	{
		return m_impl->getSize();
	}

	size_t IndexBuffer::getByteSize() const
	{
		return m_impl->getByteSize();
	}

	size_t IndexBuffer::getTotalByteSize() const
	{
		return getSize()*getByteSize();
	}

	IndexBuffer::Type IndexBuffer::getType() const
	{
		return m_impl->getType();
	}
}
