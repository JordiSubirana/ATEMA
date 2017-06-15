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

#ifndef ATEMA_RENDERER_DRAWER_HPP
#define ATEMA_RENDERER_DRAWER_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Renderer/Drawable.hpp>
#include <Atema/Renderer/IndexBuffer.hpp>
#include <Atema/Renderer/RendererDependent.hpp>
#include <Atema/Renderer/RenderTarget.hpp>
#include <Atema/Renderer/Shader.hpp>
#include <Atema/Renderer/VertexBuffer.hpp>

namespace at
{
	class Window;

	class ATEMA_RENDERER_API Renderer
	{
	public:
		class ATEMA_RENDERER_API Implementation : public RendererDependent
		{
		public:
			Implementation();
			virtual ~Implementation() = default;

			virtual void setTarget(RenderTarget* target) = 0;
			virtual void setTarget(Window* window) = 0;
			virtual void setShader(Shader* shader) = 0;
			virtual void setVertexBuffer(VertexBuffer* vertexBuffer) = 0;
			virtual void setIndexBuffer(IndexBuffer* indexBuffer) = 0;
			virtual void setInstanceBuffer(VertexBuffer* instanceData) = 0;
			virtual void draw() = 0;

			virtual void draw(VertexBuffer& vertexBuffer) = 0;
			virtual void draw(VertexBuffer& vertexBuffer, VertexBuffer& instanceData) = 0;
			virtual void draw(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) = 0;
			virtual void draw(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, VertexBuffer& instanceData) = 0;
			
			virtual void preDraw() = 0;
			virtual void postDraw() = 0;
		};

		Renderer();
		virtual ~Renderer();

		void setTarget(RenderTarget* target);
		void setTarget(Window* window);
		void setShader(Shader* shader);
		void setVertexBuffer(VertexBuffer* vertexBuffer);
		void setIndexBuffer(IndexBuffer* indexBuffer);
		void setInstanceBuffer(VertexBuffer* instanceData);
		void draw();

		void draw(VertexBuffer& vertexBuffer);
		void draw(VertexBuffer& vertexBuffer, VertexBuffer& instanceData);
		void draw(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer);
		void draw(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, VertexBuffer& instanceData);
		void draw(Drawable& drawable);

	private:
		Implementation *m_impl;
	};
}

#endif