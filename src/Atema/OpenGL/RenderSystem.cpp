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

#include <Atema/OpenGL/RenderSystem.hpp>
#include <Atema/OpenGL/Context.hpp>
#include <Atema/OpenGL/IndexBuffer.hpp>
#include <Atema/OpenGL/RenderTarget.hpp>
#include <Atema/OpenGL/Shader.hpp>
#include <Atema/OpenGL/Texture.hpp>
#include <Atema/OpenGL/VertexBuffer.hpp>
#include <Atema/OpenGL/Renderer.hpp>
#include <Atema/OpenGL/Window.hpp>

namespace at
{
	OpenGLRenderSystem::OpenGLRenderSystem()
	{
	}

	OpenGLRenderSystem::~OpenGLRenderSystem()
	{
	}

	IndexBuffer::Implementation* OpenGLRenderSystem::createIndexBufferImpl()
	{
		return new OpenGLIndexBuffer();
	}

	Renderer::Implementation* OpenGLRenderSystem::createRendererImpl()
	{
		return new OpenGLRenderer();
	}

	RenderTarget::Implementation* OpenGLRenderSystem::createRenderTargetImpl()
	{
		return new OpenGLRenderTarget();
	}

	Shader::Implementation* OpenGLRenderSystem::createShaderImpl()
	{
		return new OpenGLShader();
	}

	Texture::Implementation* OpenGLRenderSystem::createTextureImpl()
	{
		return new OpenGLTexture();
	}

	VertexBuffer::Implementation* OpenGLRenderSystem::createVertexBufferImpl()
	{
		return new OpenGLVertexBuffer();
	}

	Window::Implementation* OpenGLRenderSystem::createWindowImpl()
	{
		return new OpenGLWindow();
	}
}
