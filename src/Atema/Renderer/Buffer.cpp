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

#include <Atema/Renderer/Buffer.hpp>
#include <Atema/Renderer/Renderer.hpp>

using namespace at;

// Buffer
Buffer::Buffer()
{
}

Buffer::~Buffer()
{
}

Ptr<Buffer> Buffer::create(const Settings& settings)
{
	return Renderer::instance().createBuffer(settings);
}

// BufferRange
BufferRange::BufferRange() :
	buffer(nullptr),
	offset(0),
	size(0)
{
}

BufferRange::BufferRange(Buffer& buffer) :
	buffer(&buffer),
	offset(0),
	size(buffer.getByteSize())
{

}

BufferRange::BufferRange(Buffer& buffer, size_t offset, size_t size) :
	buffer(&buffer),
	offset(offset),
	size(size)
{
}

void* BufferRange::map()
{
	if (!buffer || !size)
		return nullptr;
	
	return buffer->map(offset, size);
}

void BufferRange::unmap()
{
	if (!buffer || !size)
		return;

	buffer->unmap();
}
