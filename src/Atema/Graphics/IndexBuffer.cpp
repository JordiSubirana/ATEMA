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

#include <Atema/Graphics/IndexBuffer.hpp>
#include <Atema/Renderer/Utils.hpp>
#include <Atema/Core/Error.hpp>

using namespace at;

IndexBuffer::IndexBuffer(const Settings& settings) :
	m_indexType(settings.indexType),
	m_size(settings.size)
{
	ATEMA_ASSERT(m_size > 0, "Invalid size");

	Buffer::Settings bufferSettings;
	bufferSettings.usages = settings.usages | BufferUsage::Index;
	bufferSettings.byteSize = m_size * ::getByteSize(m_indexType);

	m_buffer = Buffer::create(bufferSettings);
}

IndexType IndexBuffer::getIndexType() const noexcept
{
	return m_indexType;
}

const Ptr<Buffer>& IndexBuffer::getBuffer() const noexcept
{
	return m_buffer;
}

size_t IndexBuffer::getSize() const
{
	return m_size;
}

size_t IndexBuffer::getByteSize() const
{
	return m_buffer->getByteSize();
}

void* IndexBuffer::map(size_t byteOffset, size_t byteSize)
{
	return m_buffer->map(byteOffset, byteSize);
}

void IndexBuffer::unmap()
{
	m_buffer->unmap();
}
