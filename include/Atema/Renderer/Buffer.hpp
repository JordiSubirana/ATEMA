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

#ifndef ATEMA_RENDERER_BUFFER_HPP
#define ATEMA_RENDERER_BUFFER_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Renderer/Enums.hpp>

namespace at
{
	class ATEMA_RENDERER_API Buffer : public NonCopyable
	{
	public:
		struct Settings
		{
			Flags<BufferUsage> usages;
			size_t byteSize = 0;
		};

		virtual ~Buffer();

		static Ptr<Buffer> create(const Settings& settings);

		virtual Flags<BufferUsage> getUsages() const = 0;
		virtual size_t getByteSize() const = 0;

		// Requires BufferUsage::Map
		virtual void* map(size_t byteOffset = 0, size_t byteSize = 0) = 0;
		// Requires BufferUsage::Map
		virtual void unmap() = 0;

	protected:
		Buffer();
	};

	struct ATEMA_RENDERER_API BufferRange
	{
		BufferRange();
		BufferRange(Buffer& buffer);
		BufferRange(Buffer& buffer, size_t offset, size_t size);
		BufferRange(const BufferRange& other) = default;
		BufferRange(BufferRange&& other) noexcept = default;
		~BufferRange() = default;

		// Requires BufferUsage::Map
		void* map();
		// Requires BufferUsage::Map
		void unmap();

		BufferRange& operator=(const BufferRange& other) = default;
		BufferRange& operator=(BufferRange&& other) noexcept = default;

		Buffer* buffer;
		// Byte offset
		size_t offset;
		// Byte size
		size_t size;
	};


}

#endif
