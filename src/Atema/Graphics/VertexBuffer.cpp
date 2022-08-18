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
#include <Atema/Graphics/VertexBuffer.hpp>

using namespace at;

VertexBuffer::VertexBuffer(const Settings& settings) :
	m_format(settings.vertexFormat),
	m_data(nullptr),
	m_size(settings.size)
{
	ATEMA_ASSERT(m_format && m_format->getByteSize() > 0, "Invalid vertex format");
	ATEMA_ASSERT(m_size > 0, "Invalid size");

	Buffer::Settings bufferSettings;
	bufferSettings.usages = settings.usages | BufferUsage::Vertex;
	bufferSettings.byteSize = m_size * m_format->getByteSize();

	m_buffer = Buffer::create(bufferSettings);
}

const Ptr<const VertexFormat>& VertexBuffer::getFormat() const noexcept
{
	return m_format;
}

const Ptr<Buffer>& VertexBuffer::getBuffer() const noexcept
{
	return m_buffer;
}

size_t VertexBuffer::getSize() const
{
	return m_size;
}

size_t VertexBuffer::getByteSize() const
{
	return m_buffer->getByteSize();
}

void* VertexBuffer::map(size_t byteOffset, size_t byteSize)
{
	m_data = m_buffer->map(byteOffset, byteSize);

	return m_data;
}

void VertexBuffer::unmap()
{
	m_buffer->unmap();

	m_data = nullptr;
}

MemoryMapper VertexBuffer::mapComponent(VertexComponentType type) const
{
	ATEMA_ASSERT(m_data, "VertexBuffer is not mapped");

	return { m_data, m_format->getByteSize(), m_format->getComponent(type).getByteOffset() };
}
