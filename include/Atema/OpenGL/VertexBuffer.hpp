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

#ifndef ATEMA_OPENGL_VERTEXBUFFER_HPP
#define ATEMA_OPENGL_VERTEXBUFFER_HPP

#include <Atema/OpenGL/Config.hpp>
#include <Atema/Renderer/VertexBuffer.hpp>
#include <Atema/OpenGL/Buffer.hpp>
#include <Atema/OpenGL/VaoKeeper.hpp>
#include <memory>

namespace at
{
	class OpenGLContext;
	class OpenGLIndexBuffer;
	class OpenGLShader;
	class OpenGLVertexArray;

	class ATEMA_OPENGL_API OpenGLVertexBuffer : public VertexBuffer::Implementation, public OpenGLBuffer, public OpenGLVaoKeeper
	{
	public:
		OpenGLVertexBuffer();
		virtual ~OpenGLVertexBuffer();

		void setData(const void* vertices, size_t byteSize, const VertexFormat& format) override;
		void getData(void* vertices) override;

		void* map() override;

		size_t getSize() const override;
		size_t getByteSize() const override;

		VertexFormat& getFormat() override;
		const VertexFormat& getFormat() const override;

		void setDrawMode(VertexBuffer::DrawMode mode) override;
		
		unsigned getVaoId(OpenGLContext *context, OpenGLShader *shader, OpenGLIndexBuffer *ibo, OpenGLVertexBuffer *instance);
		unsigned getOpenGLDrawMode() const;

	private:
		using VaoKey = std::tuple<OpenGLContext*, OpenGLShader*, OpenGLIndexBuffer*, OpenGLVertexBuffer*>;

		VertexFormat m_format;
		VertexBuffer::DrawMode m_mode;
		unsigned m_glMode;
		size_t m_size;
		std::vector<std::pair<VaoKey, std::shared_ptr<OpenGLVertexArray>>> m_vaos;
	};
}

#endif