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

#include <Atema/Renderer/RenderTarget.hpp>
#include <Atema/Renderer/RenderSystem.hpp>
#include <Atema/Core/Error.hpp>

namespace at
{
	AbstractRenderTarget::Implementation::Implementation()
	{
	}

	AbstractRenderTarget::AbstractRenderTarget(): m_abstractImpl(nullptr)
	{
	}

	AbstractRenderTarget::~AbstractRenderTarget()
	{
	}

	AbstractRenderTarget::Implementation* AbstractRenderTarget::getImplementation()
	{
		return m_abstractImpl;
	}

	const AbstractRenderTarget::Implementation* AbstractRenderTarget::getImplementation() const
	{
		return m_abstractImpl;
	}

	RenderTarget::Implementation::Implementation()
	{
	}

	RenderTarget::RenderTarget() : RenderTarget(RenderSystem::getCurrent())
	{
	}

	RenderTarget::RenderTarget(RenderSystem *system) : m_impl(nullptr)
	{
		if (!system)
			ATEMA_ERROR("Invalid RenderSystem.");

		m_impl = system->createRenderTarget();
		m_abstractImpl = m_impl;
	}

	RenderTarget::~RenderTarget()
	{
		delete m_impl;
	}

	RenderTarget::Implementation* RenderTarget::getImplementation()
	{
		return m_impl;
	}

	const RenderTarget::Implementation* RenderTarget::getImplementation() const
	{
		return m_impl;
	}

	int RenderTarget::addAttachment(const Texture& texture, int index)
	{
		return m_impl->addAttachment(texture, index);
	}

	int RenderTarget::addAttachment(unsigned width, unsigned height, int index)
	{
		return m_impl->addAttachment(width, height, index);
	}

	void RenderTarget::removeAttachment(int index)
	{
		m_impl->removeAttachment(index);
	}

	void RenderTarget::removeAttachments()
	{
		m_impl->removeAttachments();
	}

	bool RenderTarget::isValid(int index) const
	{
		return m_impl->isValid(index);
	}

	void RenderTarget::setDrawArea(int x, int y, unsigned w, unsigned h)
	{
		m_impl->setDrawArea(x, y, w, h);
	}

	void RenderTarget::clearColor(const Color& color, int index)
	{
		m_impl->clearColor(color, index);
	}

	void RenderTarget::clearColor(const Color& color)
	{
		m_impl->clearColor(color);
	}

	void RenderTarget::clearDepth()
	{
		m_impl->clearDepth();
	}
}
