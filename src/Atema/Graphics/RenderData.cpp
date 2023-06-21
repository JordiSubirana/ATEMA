/*
	Copyright 2023 Jordi SUBIRANA

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

#include <Atema/Graphics/RenderData.hpp>

using namespace at;

RenderData::RenderData() :
	m_camera(nullptr),
	m_resetRenderablesEveryFrame(false)
{
}

bool RenderData::isValid() const noexcept
{
	return m_camera;
}

void RenderData::setCamera(const Camera& camera)
{
	m_camera = &camera;
}

void RenderData::resetRenderablesEveryFrame(bool reset)
{
	if (m_resetRenderablesEveryFrame != reset)
	{
		m_resetRenderablesEveryFrame = reset;

		m_renderableIndices.clear();

		// We will need renderable indices : initialize them
		if (!m_resetRenderablesEveryFrame)
		{
			for (size_t i = 0; i < m_renderables.size(); i++)
				m_renderableIndices[m_renderables[i]] = i;
		}
	}
}

void RenderData::addRenderable(Renderable& renderable)
{
	if (m_resetRenderablesEveryFrame)
	{
		m_renderables.emplace_back(&renderable);
	}
	else if (m_renderableIndices.find(&renderable) == m_renderableIndices.end())
	{
		m_renderables.emplace_back(&renderable);
		m_renderableIndices[&renderable] = m_renderables.size() - 1;
	}
}

void RenderData::removeRenderable(const Renderable& renderable)
{
	if (m_resetRenderablesEveryFrame)
	{
		const auto it = std::find(m_renderables.begin(), m_renderables.end(), &renderable);

		if (it != m_renderables.end())
		{
			std::swap(*it, m_renderables.back());

			m_renderables.resize(m_renderables.size() - 1);
		}
	}
	else
	{
		const auto it = m_renderableIndices.find(&renderable);

		if (it != m_renderableIndices.end())
		{
			const auto& last = m_renderables.back();

			m_renderables[it->second] = last;
			m_renderables.resize(m_renderables.size() - 1);

			m_renderableIndices[last] = it->second;
			m_renderableIndices.erase(it);
		}
	}
}

void RenderData::clearRenderables()
{
	m_renderables.clear();
	m_renderableIndices.clear();
}

const Camera& RenderData::getCamera() const noexcept
{
	return *m_camera;
}

const std::vector<Renderable*>& RenderData::getRenderables() const noexcept
{
	return m_renderables;
}
