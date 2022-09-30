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

#ifndef ATEMA_CORE_MEMORYMAPPER_HPP
#define ATEMA_CORE_MEMORYMAPPER_HPP

#include <Atema/Core/Config.hpp>

namespace at
{
	class ATEMA_CORE_API MemoryMapper
	{
	public:
		MemoryMapper();
		MemoryMapper(void* data, size_t byteOffset, size_t blockByteSize);
		MemoryMapper(const MemoryMapper& other) = default;
		MemoryMapper(MemoryMapper&& other) noexcept = default;
		~MemoryMapper() = default;

		void setData(void* data);
		void setBlockSize(size_t byteSize);

		const void* getData() const noexcept;
		size_t getBlockSize() const noexcept;

		template <typename T>
		T& map(size_t index, size_t elementByteOffset = 0);

		MemoryMapper& operator=(const MemoryMapper& other) = default;
		MemoryMapper& operator=(MemoryMapper&& other) noexcept = default;

	private:
		void* m_data;
		size_t m_byteOffset;
		size_t m_blockByteSize;
	};
}

#include <Atema/Core/MemoryMapper.inl>

#endif
