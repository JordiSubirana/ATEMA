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

#ifndef ATEMA_RENDERER_INDEXBUFFER_HPP
#define ATEMA_RENDERER_INDEXBUFFER_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Renderer/RendererDependent.hpp>
#include <vector>

namespace at
{
	class ATEMA_RENDERER_API IndexBuffer
	{
	public:
		enum class Type
		{
			UnsignedByte, UnsignedShort, UnsignedInt
		};

		class ATEMA_RENDERER_API Implementation : public RendererDependent
		{
		public:
			Implementation();
			virtual ~Implementation() = default;

			virtual void setData(const void *indices, size_t byteSize, Type type) = 0;
			virtual void getData(void* indices) = 0;

			virtual size_t getSize() const = 0;
			virtual size_t getByteSize() const = 0;

			virtual Type getType() const = 0;
		};

		IndexBuffer();
		explicit IndexBuffer(RenderSystem *system);
		virtual ~IndexBuffer();

		Implementation* getImplementation();
		const Implementation* getImplementation() const;

		void setData(const void *indices, size_t byteSize, Type type);
		template <typename T>
		void setData(const std::vector<T>& indices);
		void getData(void* indices);
		template <typename T>
		void getData(std::vector<T>& indices);

		size_t getSize() const;
		size_t getByteSize() const;
		size_t getTotalByteSize() const;

		Type getType() const;

	private:
		Implementation *m_impl;
	};
}

#include <Atema/Renderer/IndexBuffer.inl>

#endif