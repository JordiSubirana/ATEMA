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

#ifndef ATEMA_GRAPHICS_INDEXBUFFER_HPP
#define ATEMA_GRAPHICS_INDEXBUFFER_HPP

#include <Atema/Core/Pointer.hpp>
#include <Atema/Graphics/Config.hpp>
#include <Atema/Renderer/Buffer.hpp>
#include <Atema/Renderer/Enums.hpp>

namespace at
{
	class ATEMA_GRAPHICS_API IndexBuffer
	{
	public:
		struct Settings
		{
			// Index type defining each element
			IndexType indexType;

			// Element size (not byte size)
			size_t size = 0;

			// Additional buffer usages
			// BufferUsage::Index will be added if not specified
			Flags<BufferUsage> usages = BufferUsage::Index;
		};

		IndexBuffer() = delete;
		IndexBuffer(const IndexBuffer& other) = default;
		IndexBuffer(IndexBuffer&& other) noexcept = default;
		IndexBuffer(const Settings& settings);
		~IndexBuffer() = default;

		IndexBuffer& operator=(const IndexBuffer& other) = default;
		IndexBuffer& operator=(IndexBuffer&& other) noexcept = default;

		IndexType getIndexType() const noexcept;
		const Ptr<Buffer>& getBuffer() const noexcept;

		size_t getSize() const;
		size_t getByteSize() const;

		// Requires BufferUsage::Map
		void* map(size_t byteOffset = 0, size_t byteSize = 0);
		// Requires BufferUsage::Map
		void unmap();

	private:
		IndexType m_indexType;
		Ptr<Buffer> m_buffer;
		size_t m_size;
	};
}

#endif
