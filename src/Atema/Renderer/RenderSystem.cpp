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

#include <Atema/Renderer/RenderSystem.hpp>

namespace
{
	thread_local at::RenderSystem* s_current(nullptr);
}

namespace at
{
	RenderSystem::~RenderSystem()
	{
		if (s_current == this)
			s_current = nullptr;
	}

	void RenderSystem::makeCurrent() noexcept
	{
		s_current = this;
	}

	RenderSystem* RenderSystem::getCurrent() noexcept
	{
		return s_current;
	}

	RenderSystem::RenderSystem()
	{
		makeCurrent();
	}

	IndexBuffer::Implementation* RenderSystem::createIndexBuffer()
	{
		auto object = this->createIndexBufferImpl();

		object->m_system = this;

		return object;
	}

	Renderer::Implementation* RenderSystem::createRenderer()
	{
		auto object = this->createRendererImpl();

		object->m_system = this;

		return object;
	}

	RenderTarget::Implementation* RenderSystem::createRenderTarget()
	{
		auto object = this->createRenderTargetImpl();

		object->m_system = this;

		return object;
	}

	Shader::Implementation* RenderSystem::createShader()
	{
		auto object = this->createShaderImpl();

		object->m_system = this;

		return object;
	}

	Texture::Implementation* RenderSystem::createTexture()
	{
		auto object = this->createTextureImpl();

		object->m_system = this;

		return object;
	}

	VertexBuffer::Implementation* RenderSystem::createVertexBuffer()
	{
		auto object = this->createVertexBufferImpl();

		object->m_system = this;

		return object;
	}

	Window::Implementation* RenderSystem::createWindow()
	{
		auto object = this->createWindowImpl();

		object->m_system = this;

		return object;
	}
}
