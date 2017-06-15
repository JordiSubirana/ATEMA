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

#ifndef ATEMA_OPENGL_BUFFER_HPP
#define ATEMA_OPENGL_BUFFER_HPP

#include <Atema/OpenGL/Config.hpp>

namespace at
{
	class ATEMA_OPENGL_API OpenGLBuffer
	{
	public:
		enum class Type
		{
			Array, AtomicCounter, CopyRead, CopyWrite, DispatchIndirect,
			DrawIndirect, ElementArray, PixelPack, PixelUnpack, Query,
			ShaderStorage, Texture, TransformFeedback, Uniform
		};

		OpenGLBuffer() = delete;
		OpenGLBuffer(Type type);
		virtual ~OpenGLBuffer();

		unsigned getId() const;

		void setData(const void* data, size_t byteSize);
		void getData(void* data);

		void* map();
		void unmap() const;

		size_t getTotalByteSize() const;

	private:
		unsigned m_id;
		size_t m_byteSize;
		int m_type;
		mutable void *m_map;
	};
}

#endif