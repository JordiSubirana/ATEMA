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

#ifndef ATEMA_OPENGL_DRAWER_HPP
#define ATEMA_OPENGL_DRAWER_HPP

#include <Atema/OpenGL/Config.hpp>
#include <Atema/Renderer/Renderer.hpp>

namespace at
{
	class OpenGLContext;
	class OpenGLFrameBuffer;
	class OpenGLIndexBuffer;
	class OpenGLRenderTarget;
	class OpenGLShader;
	class OpenGLVertexBuffer;

	class ATEMA_OPENGL_API OpenGLRenderer : public Renderer::Implementation
	{
	public:
		OpenGLRenderer();
		virtual ~OpenGLRenderer();

		void setTarget(RenderTarget* target) override;
		void setTarget(Window* window) override;
		void setShader(Shader* shader) override;
		void setVertexBuffer(VertexBuffer* vertexBuffer) override;
		void setIndexBuffer(IndexBuffer* indexBuffer) override;
		void setInstanceBuffer(VertexBuffer* instanceData) override;
		void draw() override;

		void draw(VertexBuffer& vertexBuffer) override;
		void draw(VertexBuffer& vertexBuffer, VertexBuffer& instanceData) override;
		void draw(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) override;
		void draw(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, VertexBuffer& instanceData) override;

		void preDraw() override;
		void postDraw() override;

	private:
		void setTarget(OpenGLFrameBuffer* framebuffer);
		void draw(OpenGLVertexBuffer *vbo, OpenGLIndexBuffer *ibo, OpenGLVertexBuffer *instanceData);

		OpenGLFrameBuffer *m_target;
		OpenGLShader *m_shader;
		OpenGLVertexBuffer *m_vbo;
		OpenGLIndexBuffer *m_ibo;
		OpenGLVertexBuffer *m_instanceData;
		OpenGLContext *m_context;
		unsigned m_safe;
		bool m_updateVao;
		unsigned m_vao;
	};
}

#endif