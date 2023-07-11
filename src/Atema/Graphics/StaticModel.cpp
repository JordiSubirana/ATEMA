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

#include <Atema/Graphics/StaticModel.hpp>
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/StaticRenderModel.hpp>

using namespace at;

Ptr<RenderObject> StaticModel::createRenderObject(RenderScene& renderScene)
{
	return std::make_shared<StaticRenderModel>(renderScene, *this);
}

void StaticModel::setModel(const Ptr<Model>& model)
{
	m_modelConnectionGuard.disconnect();

	m_model = model;

	m_modelConnectionGuard.connect(m_model->onGeometryUpdate, [this]()
		{
			updateAABB();

			onModelUpdate();
		});

	updateAABB();

	onModelUpdate();
}

void StaticModel::setTransform(const Transform& transform)
{
	m_transform = transform;

	updateAABB();

	onTransformUpdate();
}

const Ptr<Model>& StaticModel::getModel() const noexcept
{
	return m_model;
}

const Transform& StaticModel::getTransform() const noexcept
{
	return m_transform;
}

const AABBf& StaticModel::getAABB() const noexcept
{
	return m_aabb;
}

void StaticModel::updateAABB()
{
	if (!m_model)
		return;

	m_aabb = m_transform.getMatrix() * m_model->getAABB();
}
