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
#include <Atema/Graphics/Mesh.hpp>
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/ShaderData.hpp>
#include <Atema/Renderer/RenderFrame.hpp>
#include <Atema/Renderer/Buffer.hpp>
#include <Atema/Graphics/SurfaceMaterial.hpp>

using namespace at;

StaticModel::StaticModel() :
	m_updateTransform(true)
{
	Buffer::Settings bufferSettings;
	bufferSettings.usages = BufferUsage::Uniform | BufferUsage::TransferDst;
	bufferSettings.byteSize = TransformData::getLayout().getByteSize();

	m_objectBuffer = Buffer::create(bufferSettings);

	m_objectSet = Graphics::instance().getObjectLayout()->createSet();
	m_objectSet->update(0, *m_objectBuffer);

	m_objectBinding.index = SurfaceMaterial::ObjectSetIndex;
	m_objectBinding.descriptorSet = m_objectSet.get();
	m_objectBinding.dynamicBufferOffsets = { 0 };
}

void StaticModel::setModel(const Ptr<Model>& model)
{
	m_model = model;

	updateRenderElements();

	updateAABB();
}

void StaticModel::setTransform(const Transform& transform)
{
	m_transform = transform;

	m_updateTransform = true;

	updateAABB();
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

void StaticModel::update(RenderFrame& renderFrame, CommandBuffer& commandBuffer)
{
	if (m_updateTransform)
	{
		auto stagingBuffer = renderFrame.allocateStagingBuffer(m_objectBuffer->getByteSize());

		auto data = stagingBuffer->map();

		mapMemory<Matrix4f>(data, 0) = m_transform.getMatrix();

		commandBuffer.copyBuffer(stagingBuffer->getBuffer(), *m_objectBuffer, stagingBuffer->getSize(), stagingBuffer->getOffset());
		
		m_updateTransform = false;
	}
}

void StaticModel::getRenderElements(std::vector<RenderElement>& renderElements) const
{
	for (const auto& renderElement : m_renderElements)
		renderElements.emplace_back(renderElement);
}

size_t StaticModel::getRenderElementsSize() const noexcept
{
	if (!m_model)
		return 0;
	
	return m_model->getMeshes().size();
}

void StaticModel::updateAABB()
{
	if (!m_model)
		return;
	
	m_aabb = m_transform.getMatrix() * m_model->getAABB();
}

void StaticModel::updateRenderElements()
{
	m_renderElements.clear();
	m_materialInstances.clear();

	if (!m_model)
		return;

	auto& graphics = Graphics::instance();

	m_materialInstances.reserve(m_model->getMaterialData().size());

	for (const auto& materialData : m_model->getMaterialData())
	{
		auto surfaceMaterial = graphics.getSurfaceMaterial(*materialData);
		m_materialInstances.emplace_back(graphics.getSurfaceMaterialInstance(surfaceMaterial, *materialData));
	}

	for (const auto& mesh : m_model->getMeshes())
	{
		if (mesh->getMaterialID() >= m_materialInstances.size())
			continue;

		auto& renderElement = m_renderElements.emplace_back();

		renderElement.aabb = mesh->getAABB();
		renderElement.vertexBuffer = mesh->getVertexBuffer().get();
		renderElement.indexBuffer = mesh->getIndexBuffer().get();
		renderElement.materialInstance = m_materialInstances[mesh->getMaterialID()].get();
		renderElement.objectBinding = &m_objectBinding;
	}
}
