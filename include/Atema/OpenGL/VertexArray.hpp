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

#ifndef ATEMA_OPENGL_HPP
#define ATEMA_OPENGL_HPP

#include <Atema/OpenGL/Config.hpp>

namespace at
{
	class OpenGLContext;
	class OpenGLIndexBuffer;
	class OpenGLShader;
	class OpenGLVertexBuffer;
	
	class ATEMA_OPENGL_API OpenGLVertexArray
	{
	public:
		OpenGLVertexArray() = delete;
		OpenGLVertexArray(OpenGLContext *context, OpenGLShader *shader, OpenGLVertexBuffer *vbo, OpenGLIndexBuffer *ibo, OpenGLVertexBuffer *instanceData);
		virtual ~OpenGLVertexArray();

		unsigned getId() const;

		void invalidate();
		bool isValid() const;

	private:
		unsigned m_id;
		bool m_valid;
		OpenGLContext *m_context;
		OpenGLShader *m_shader;
		OpenGLVertexBuffer *m_vbo, *m_instanceData;
		OpenGLIndexBuffer *m_ibo;
	};
}

#endif