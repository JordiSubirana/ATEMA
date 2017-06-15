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

#ifndef ATEMA_OPENGL_INDEXBUFFER_HPP
#define ATEMA_OPENGL_INDEXBUFFER_HPP

#include <Atema/OpenGL/Config.hpp>
#include <Atema/Renderer/IndexBuffer.hpp>
#include <Atema/OpenGL/Buffer.hpp>
#include <Atema/OpenGL/VaoKeeper.hpp>

namespace at
{
	class ATEMA_OPENGL_API OpenGLIndexBuffer : public IndexBuffer::Implementation, public OpenGLBuffer, public OpenGLVaoKeeper
	{
	public:
		OpenGLIndexBuffer();
		virtual ~OpenGLIndexBuffer();

		void setData(const void* indices, size_t byteSize, IndexBuffer::Type type) override;
		void getData(void* indices) override;

		size_t getSize() const override;
		size_t getByteSize() const override;

		IndexBuffer::Type getType() const override;

		unsigned getOpenGLType() const;

	private:
		IndexBuffer::Type m_type;
		size_t m_size;
		unsigned m_glType;
	};
}

#endif