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

#include <Atema/Core/Benchmark.hpp>
#include <Atema/Graphics/Mesh.hpp>
#include <Atema/Graphics/RenderScene.hpp>
#include <Atema/Graphics/StaticRenderModel.hpp>
#include <Atema/Graphics/StaticModel.hpp>

using namespace at;

StaticRenderModel::StaticRenderModel(RenderScene& renderScene, StaticModel& staticModel) :
	RenderObject(renderScene, staticModel),
	m_staticModel(&staticModel),
	m_modelValid(false),
	m_transformValid(false)
{
	Buffer::Settings bufferSettings;
	bufferSettings.usages = BufferUsage::Uniform | BufferUsage::TransferDst;
	bufferSettings.byteSize = TransformData::getLayout().getByteSize();

	m_transformBuffer = getResourceManager().createBuffer(bufferSettings);

	m_connectionGuard.connect(staticModel.onModelUpdate, [this]()
		{
			m_modelValid = false;
		});

	m_connectionGuard.connect(staticModel.onTransformUpdate, [this]()
		{
			m_transformValid = false;
		});
}

void StaticRenderModel::getRenderElements(std::vector<RenderElement>& renderElements) const
{
	for (const auto& renderElement : m_renderElements)
		renderElements.emplace_back(renderElement);
}

size_t StaticRenderModel::getRenderElementsSize() const noexcept
{
	return m_renderElements.size();
}

void StaticRenderModel::updateResources()
{
	auto& renderFrame = getResourceManager().getRenderFrame();
	auto& commandBuffer = getResourceManager().getCommandBuffer();

	if (!m_transformValid)
	{
		void* data = getResourceManager().mapBuffer(*m_transformBuffer);

		mapMemory<Matrix4f>(data, 0) = m_staticModel->getTransform().getMatrix();

		m_transformValid = true;
	}
	
	if (!m_modelValid)
	{
		auto& renderScene = getRenderScene();
		auto& model = *m_staticModel->getModel();

		// Destroy previous descriptor sets
		for (auto& descriptorSet : m_transformDescriptorSets)
			destroyAfterUse(std::move(descriptorSet));

		// Update RenderMaterialInstances
		std::vector<uint32_t> transformSetIndices;
		transformSetIndices.reserve(model.getMaterialInstances().size());

		m_renderMaterialInstances.clear();
		m_transformDescriptorSets.clear();
		for (auto& materialInstance : model.getMaterialInstances())
		{
			auto& renderMaterialInstance = renderScene.getRenderMaterialInstance(*materialInstance);
			const auto& renderMaterial = renderMaterialInstance.getRenderMaterial();

			m_connectionGuard.connect(renderMaterialInstance.onDestroy, [this]()
				{
					m_modelValid = false;
				});

			m_renderMaterialInstances.emplace_back(&renderMaterialInstance);

			const auto& transformBinding = renderMaterial.getBinding("TransformData");
			auto descriptorSet = renderMaterial.createSet(transformBinding.set);
			descriptorSet->update(transformBinding.binding, m_transformBuffer->getBuffer(), m_transformBuffer->getOffset(), m_transformBuffer->getSize());

			m_transformDescriptorSets.emplace_back(std::move(descriptorSet));
			transformSetIndices.emplace_back(transformBinding.set);
		}

		// Update RenderElements
		m_renderElements.clear();
		for (const auto& mesh : model.getMeshes())
		{
			if (mesh->getMaterialID() >= m_renderMaterialInstances.size())
				continue;

			auto& renderElement = m_renderElements.emplace_back();

			renderElement.aabb = mesh->getAABB();
			renderElement.vertexBuffer = mesh->getVertexBuffer().get();
			renderElement.indexBuffer = mesh->getIndexBuffer().get();
			renderElement.renderMaterialInstance = m_renderMaterialInstances[mesh->getMaterialID()];
			renderElement.transformSetIndex = transformSetIndices[mesh->getMaterialID()];
			renderElement.transformDescriptorSet = m_transformDescriptorSets[mesh->getMaterialID()].get();
		}

		m_modelValid = true;
	}
}
