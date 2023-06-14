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
#include <Atema/Renderer/RenderFrame.hpp>
#include <Atema/Renderer/Buffer.hpp>
#include <Atema/Graphics/SurfaceMaterial.hpp>

using namespace at;

StaticModel::StaticModel() :
	m_updateTransform(true)
{
	Buffer::Settings bufferSettings;
	bufferSettings.usages = BufferUsage::Uniform | BufferUsage::TransferDst;
	bufferSettings.byteSize = SurfaceMaterial::ObjectData::getBufferLayout().getSize();

	m_objectBuffer = Buffer::create(bufferSettings);

	m_objectSet = SurfaceMaterial::getObjectLayout()->createSet();
	m_objectSet->update(0, *m_objectBuffer);

	m_objectBinding.index = SurfaceMaterial::ObjectSetIndex;
	m_objectBinding.descriptorSet = m_objectSet.get();
	m_objectBinding.dynamicBufferOffsets = { 0 };
}

void StaticModel::setModel(const Ptr<Model>& model)
{
	m_model = model;

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

void StaticModel::updateResources(RenderFrame& renderFrame, CommandBuffer& commandBuffer)
{
	if (m_updateTransform)
	{
		SurfaceMaterial::ObjectData objectData;
		objectData.model = m_transform.getMatrix();

		auto stagingBuffer = renderFrame.createStagingBuffer(m_objectBuffer->getByteSize());

		auto data = stagingBuffer.map();

		objectData.copyTo(data);

		stagingBuffer.unmap();

		commandBuffer.copyBuffer(*stagingBuffer.buffer, *m_objectBuffer, stagingBuffer.size, stagingBuffer.offset);

		m_updateTransform = false;
	}
}

void StaticModel::getRenderElements(std::vector<RenderElement>& renderElements) const
{
	if (!m_model)
		return;

	auto& graphics = Graphics::instance();

	m_materialInstances.clear();
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
		
		auto& renderElement = renderElements.emplace_back();

		renderElement.aabb = mesh->getAABB();
		renderElement.vertexBuffer = mesh->getVertexBuffer().get();
		renderElement.indexBuffer = mesh->getIndexBuffer().get();
		renderElement.materialInstance = m_materialInstances[mesh->getMaterialID()].get();
		renderElement.objectBinding = &m_objectBinding;
	}
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
