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

#include <Atema/Renderer/Renderer.hpp>
#include <Atema/Core/Error.hpp>
#include <Atema/Renderer/RenderSystem.hpp>

namespace at
{
	Renderer::Implementation::Implementation()
	{
	}

	Renderer::Renderer() : Renderer(RenderSystem::getCurrent())
	{
	}

	Renderer::Renderer(RenderSystem *system) : m_impl(nullptr)
	{
		if (!system)
			ATEMA_ERROR("Invalid RenderSystem.");

		m_impl = system->createRenderer();
	}

	Renderer::~Renderer()
	{
		delete m_impl;
	}

	void Renderer::setTarget(RenderTarget* target)
	{
		m_impl->setTarget(target);
	}

	void Renderer::setTarget(Window* window)
	{
		m_impl->setTarget(window);
	}

	void Renderer::setShader(Shader* shader)
	{
		m_impl->setShader(shader);
	}

	void Renderer::setVertexBuffer(VertexBuffer* vertexBuffer)
	{
		m_impl->setVertexBuffer(vertexBuffer);
	}

	void Renderer::setIndexBuffer(IndexBuffer* indexBuffer)
	{
		m_impl->setIndexBuffer(indexBuffer);
	}

	void Renderer::setInstanceBuffer(VertexBuffer* instanceData)
	{
		m_impl->setInstanceBuffer(instanceData);
	}

	void Renderer::draw()
	{
		m_impl->draw();
	}

	void Renderer::draw(VertexBuffer& vertexBuffer)
	{
		m_impl->draw(vertexBuffer);
	}

	void Renderer::draw(VertexBuffer& vertexBuffer, VertexBuffer& instanceData)
	{
		m_impl->draw(vertexBuffer, instanceData);
	}

	void Renderer::draw(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer)
	{
		m_impl->draw(vertexBuffer, indexBuffer);
	}

	void Renderer::draw(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, VertexBuffer& instanceData)
	{
		m_impl->draw(vertexBuffer, indexBuffer, instanceData);
	}

	void Renderer::draw(Drawable& drawable)
	{
		m_impl->preDraw();
		
		drawable.draw(*this);

		m_impl->postDraw();
	}
}
