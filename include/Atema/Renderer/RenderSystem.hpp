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

#ifndef ATEMA_RENDERER_RENDERER_HPP
#define ATEMA_RENDERER_RENDERER_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Renderer/IndexBuffer.hpp>
#include <Atema/Renderer/Renderer.hpp>
#include <Atema/Renderer/RenderTarget.hpp>
#include <Atema/Renderer/Shader.hpp>
#include <Atema/Renderer/Texture.hpp>
#include <Atema/Renderer/VertexBuffer.hpp>
#include <Atema/Renderer/Window.hpp>

namespace at
{
	class ATEMA_RENDERER_API RenderSystem
	{
		friend class IndexBuffer;
		friend class Renderer;
		friend class RenderTarget;
		friend class Shader;
		friend class Texture;
		friend class VertexBuffer;
		friend class Window;

	public:
		virtual ~RenderSystem();

		void makeCurrent() noexcept;
		static RenderSystem* getCurrent() noexcept;

	protected:
		RenderSystem();

		virtual IndexBuffer::Implementation* createIndexBufferImpl() = 0;
		virtual Renderer::Implementation* createRendererImpl() = 0;
		virtual RenderTarget::Implementation* createRenderTargetImpl() = 0;
		virtual Shader::Implementation* createShaderImpl() = 0;
		virtual Texture::Implementation* createTextureImpl() = 0;
		virtual VertexBuffer::Implementation* createVertexBufferImpl() = 0;
		virtual Window::Implementation* createWindowImpl() = 0;

	private:
		IndexBuffer::Implementation* createIndexBuffer();
		Renderer::Implementation* createRenderer();
		RenderTarget::Implementation* createRenderTarget();
		Shader::Implementation* createShader();
		Texture::Implementation* createTexture();
		VertexBuffer::Implementation* createVertexBuffer();
		Window::Implementation* createWindow();
	};
}

#endif