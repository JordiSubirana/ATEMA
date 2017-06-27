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

#ifndef ATEMA_RENDERER_VERTEXBUFFER_HPP
#define ATEMA_RENDERER_VERTEXBUFFER_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Renderer/RendererDependent.hpp>
#include <Atema/Renderer/Vertex.hpp>
#include <vector>

namespace at
{
	class ATEMA_RENDERER_API VertexBuffer
	{
	public:
		enum class DrawMode
		{
			Triangles
		};

		class ATEMA_RENDERER_API Implementation : public RendererDependent
		{
		public:
			Implementation();
			virtual ~Implementation() = default;

			virtual void setData(const void *vertices, size_t byteSize, const VertexFormat& format) = 0;
			virtual void getData(void* vertices) = 0;

			virtual void* map() = 0;

			virtual void setDrawMode(DrawMode mode) = 0;

			virtual size_t getSize() const = 0;
			virtual size_t getByteSize() const = 0;

			virtual VertexFormat& getFormat() = 0;
			virtual const VertexFormat& getFormat() const = 0;
		};

		VertexBuffer();
		explicit VertexBuffer(RenderSystem *system);
		virtual ~VertexBuffer();

		Implementation* getImplementation();
		const Implementation* getImplementation() const;

		void setData(const void *vertices, size_t byteSize, const VertexFormat& format);
		template <typename T>
		void setData(const std::vector<T>& vertices, const VertexFormat& format);
		void getData(void* vertices);
		template <typename T>
		void getData(std::vector<T>& vertices);

		void* map();
		template <typename T>
		T* map();

		void setDrawMode(DrawMode mode);

		size_t getSize() const;
		size_t getByteSize() const;
		size_t getTotalByteSize() const;

		VertexFormat& getFormat();
		const VertexFormat& getFormat() const;

	private:
		Implementation *m_impl;
	};
}

#include <Atema/Renderer/VertexBuffer.inl>

#endif