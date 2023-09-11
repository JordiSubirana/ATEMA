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

#include <Atema/Graphics/Passes/GBufferPass.hpp>
#include <Atema/Graphics/FrameGraphBuilder.hpp>
#include <Atema/Graphics/RenderScene.hpp>
#include <Atema/Graphics/FrameGraphContext.hpp>
#include <Atema/Graphics/Camera.hpp>
#include <Atema/Graphics/VertexBuffer.hpp>
#include <Atema/Graphics/IndexBuffer.hpp>
#include <Atema/Core/TaskManager.hpp>
#include <Atema/Graphics/Graphics.hpp>

using namespace at;

namespace
{
	inline IntersectionType getFrustumIntersection(const Frustumf& frustum, const AABBf& aabb)
	{
		const auto sphere = aabb.getBoundingSphere();

		const auto sphereIntersection = frustum.getIntersectionType(sphere);

		if (sphereIntersection != IntersectionType::Intersection)
			return sphereIntersection;

		return frustum.getIntersectionType(aabb);
	}

	inline uint32_t getRenderPriority(const RenderElement& renderElement)
	{
		if (!renderElement.renderMaterialInstance)
			return 0;

		uint32_t renderPriority = 0;
		renderPriority |= static_cast<uint32_t>(renderElement.renderMaterialInstance->getRenderMaterial().getID() & 0xFFFF) << 16;
		renderPriority |= static_cast<uint32_t>(renderElement.renderMaterialInstance->getID() & 0xFFFF) << 0;

		return renderPriority;
	}
}

GBufferPass::GBufferPass(RenderResourceManager& resourceManager, size_t threadCount) :
	m_resourceManager(&resourceManager)
{
	const auto& taskManager = TaskManager::instance();
	const auto maxThreadCount = taskManager.getSize();
	m_threadCount = threadCount;
	if (threadCount == 0 || threadCount > maxThreadCount)
		m_threadCount = maxThreadCount;

	Buffer::Settings bufferSettings;
	bufferSettings.usages = BufferUsage::Uniform | BufferUsage::TransferDst;
	bufferSettings.byteSize = FrameData::getLayout().getByteSize();

	m_frameDataBuffer = m_resourceManager->createBuffer(bufferSettings);

	m_frameDataDescriptorSet = Graphics::instance().getFrameLayout()->createSet();
	m_frameDataDescriptorSet->update(0, m_frameDataBuffer->getBuffer(), m_frameDataBuffer->getOffset(), m_frameDataBuffer->getSize());
}

const char* GBufferPass::getName() const noexcept
{
	return "GBuffer";
}

FrameGraphPass& GBufferPass::addToFrameGraph(FrameGraphBuilder& frameGraphBuilder, const Settings& settings)
{
	auto& pass = frameGraphBuilder.createPass(getName());

	uint32_t gbufferTextureIndex = 0;
	for (const auto& texture : settings.gbuffer)
	{
		if (settings.gbufferClearValue.size() > gbufferTextureIndex && settings.gbufferClearValue[gbufferTextureIndex].has_value())
			pass.addOutputTexture(texture, gbufferTextureIndex++, settings.gbufferClearValue[gbufferTextureIndex].value());
		else
			pass.addOutputTexture(texture, gbufferTextureIndex++);
	}

	if (settings.depthStencilClearValue.has_value())
		pass.setDepthTexture(settings.depthStencil, settings.depthStencilClearValue.value());
	else
		pass.setDepthTexture(settings.depthStencil);

	pass.enableSecondaryCommandBuffers(m_threadCount != 1);

	Settings settingsCopy = settings;

	pass.setExecutionCallback([this, settingsCopy](FrameGraphContext& context)
		{
			execute(context, settingsCopy);
		});

	return pass;
}

void GBufferPass::updateResources(CommandBuffer& commandBuffer)
{
	const auto& camera = getRenderScene().getCamera();

	FrameData surfaceFrameData;
	surfaceFrameData.cameraPosition = camera.getPosition();
	surfaceFrameData.view = camera.getViewMatrix();
	surfaceFrameData.projection = camera.getProjectionMatrix();
	surfaceFrameData.screenSize = camera.getScissor().size;

	auto data = m_resourceManager->mapBuffer(*m_frameDataBuffer);

	surfaceFrameData.copyTo(data);
}

void GBufferPass::execute(FrameGraphContext& context, const Settings& settings)
{
	const auto& renderScene = getRenderScene();

	if (!renderScene.isValid())
		return;

	auto& commandBuffer = context.getCommandBuffer();

	if (m_threadCount == 1)
	{
		drawElements(commandBuffer, 0, m_renderElements.size());
	}
	else
	{
		auto& taskManager = TaskManager::instance();
		std::vector<Ptr<Task>> tasks;
		tasks.reserve(m_threadCount);

		std::vector<Ptr<CommandBuffer>> commandBuffers;
		commandBuffers.resize(m_threadCount);

		size_t firstIndex = 0;
		size_t size = m_renderElements.size() / m_threadCount;

		for (size_t taskIndex = 0; taskIndex < m_threadCount; taskIndex++)
		{
			if (taskIndex == m_threadCount - 1)
			{
				const auto remainingSize = m_renderElements.size() - m_threadCount * size;

				size += remainingSize;
			}

			auto task = taskManager.createTask([this, &context, &commandBuffers, taskIndex, firstIndex, size](size_t threadIndex)
				{
					auto commandBuffer = context.createSecondaryCommandBuffer(threadIndex);

					drawElements(*commandBuffer, firstIndex, size);

					commandBuffer->end();

					commandBuffers[taskIndex] = commandBuffer;
				});

			tasks.push_back(task);

			firstIndex += size;
		}

		for (auto& task : tasks)
			task->wait();

		commandBuffer.executeSecondaryCommands(commandBuffers);
	}
}

void GBufferPass::beginFrame()
{
	const auto& renderScene = getRenderScene();

	if (!renderScene.isValid())
		return;

	frustumCull();

	sortElements();
}

void GBufferPass::endFrame()
{
	m_renderElements.clear();
}

void GBufferPass::frustumCull()
{
	const auto& renderObjects = getRenderScene().getRenderObjects();

	if (m_threadCount == 1)
	{
		frustumCullElements(m_renderElements, 0, renderObjects.size());
	}
	else
	{
		auto& taskManager = TaskManager::instance();

		std::vector<std::vector<RenderElement>> renderElements(m_threadCount);

		std::vector<Ptr<Task>> tasks;
		tasks.reserve(m_threadCount);

		size_t firstIndex = 0;
		size_t size = renderObjects.size() / m_threadCount;

		for (size_t taskIndex = 0; taskIndex < m_threadCount; taskIndex++)
		{
			if (taskIndex == m_threadCount - 1)
			{
				const auto remainingSize = renderObjects.size() - m_threadCount * size;

				size += remainingSize;
			}

			auto task = taskManager.createTask([this, taskIndex, firstIndex, size, &renderElements](size_t threadIndex)
				{
					frustumCullElements(renderElements[taskIndex], firstIndex, size);
				});

			tasks.push_back(task);

			firstIndex += size;
		}

		for (auto& task : tasks)
			task->wait();

		for (auto& elements : renderElements)
		{
			for (auto& renderElement : elements)
			{
				m_renderElements.emplace_back(std::move(renderElement));
			}
		}
	}
}

void GBufferPass::frustumCullElements(std::vector<RenderElement>& renderElements, size_t index, size_t count) const
{
	if (!count)
		return;

	const auto& renderObjects = getRenderScene().getRenderObjects();

	const auto& frustum = getRenderScene().getCamera().getFrustum();

	std::vector<const RenderObject*> visibleRenderObjects;
	std::vector<IntersectionType> renderObjectIntersections;

	// Keep only visible renderables using frustum culling
	visibleRenderObjects.reserve(count);
	renderObjectIntersections.reserve(count);

	size_t renderElementsSize = 0;

	for (size_t i = index; i < index + count; i++)
	{
		const auto& renderObject = *renderObjects[i];

		const auto intersectionType = getFrustumIntersection(frustum, renderObject.getRenderable().getAABB());

		if (intersectionType != IntersectionType::Outside)
		{
			visibleRenderObjects.emplace_back(&renderObject);
			renderObjectIntersections.emplace_back(intersectionType);

			renderElementsSize += renderObject.getRenderElementsSize();
		}
	}

	// Cull individual elements if needed
	renderElements.reserve(renderElementsSize);

	std::vector<RenderElement> tmpRenderElements;

	for (size_t i = 0; i < visibleRenderObjects.size(); i++)
	{
		const auto& renderObject = *visibleRenderObjects[i];

		// Renderable is fully contained : every element also is
		if (renderObjectIntersections[i] == IntersectionType::Inside)
		{
			renderObject.getRenderElements(renderElements);
		}
		// Renderable is intersecting with the frustum : test every element
		else
		{
			tmpRenderElements.clear();
			tmpRenderElements.reserve(renderObject.getRenderElementsSize());

			renderObject.getRenderElements(tmpRenderElements);

			const auto& matrix = renderObject.getRenderable().getTransform().getMatrix();

			for (auto& renderElement : tmpRenderElements)
			{
				if (getFrustumIntersection(frustum, matrix * renderElement.aabb) != IntersectionType::Outside)
					renderElements.emplace_back(std::move(renderElement));
			}
		}
	}
}

void GBufferPass::sortElements()
{
	std::sort(m_renderElements.begin(), m_renderElements.end(), [](const RenderElement& a, const RenderElement& b)
		{
			return getRenderPriority(a) < getRenderPriority(b);
		});
}

void GBufferPass::drawElements(CommandBuffer& commandBuffer, size_t index, size_t count)
{
	const auto& viewport = getRenderScene().getCamera().getViewport();
	const auto& scissor = getRenderScene().getCamera().getScissor();

	commandBuffer.setViewport(viewport);

	commandBuffer.setScissor(scissor.pos, scissor.size);

	if (!count)
		return;

	auto currentMaterialID = RenderMaterial::InvalidID;
	auto currentMaterialInstanceID = RenderMaterial::InvalidID;

	// Initialize material to the first one and bind global frame data shared across all pipelines
	{
		const auto& renderMaterialInstance = *m_renderElements[index].renderMaterialInstance;
		const auto& renderMaterial = renderMaterialInstance.getRenderMaterial();

		renderMaterial.bindTo(commandBuffer);
		renderMaterialInstance.bindTo(commandBuffer);

		currentMaterialID = renderMaterial.getID();
		currentMaterialInstanceID = renderMaterialInstance.getID();
	}

	commandBuffer.bindDescriptorSet(0, *m_frameDataDescriptorSet);

	for (size_t i = index; i < index + count; i++)
	{
		auto& renderElement = m_renderElements[i];
		
		auto& renderMaterialInstance = *renderElement.renderMaterialInstance;
		auto& renderMaterial = renderMaterialInstance.getRenderMaterial();

		auto materialID = renderMaterial.getID();
		auto materialInstanceID = renderMaterialInstance.getID();

		if (materialID != currentMaterialID)
		{
			renderMaterial.bindTo(commandBuffer);

			currentMaterialID = materialID;

			currentMaterialInstanceID = RenderMaterial::InvalidID;
		}

		if (materialInstanceID != currentMaterialInstanceID)
		{
			renderMaterialInstance.bindTo(commandBuffer);

			currentMaterialInstanceID = materialInstanceID;
		}

		if (renderElement.transformDescriptorSet)
			commandBuffer.bindDescriptorSet(renderElement.transformSetIndex, *renderElement.transformDescriptorSet);

		const auto& vertexBuffer = renderElement.vertexBuffer;
		const auto& indexBuffer = renderElement.indexBuffer;

		commandBuffer.bindVertexBuffer(*vertexBuffer->getBuffer(), 0);

		commandBuffer.bindIndexBuffer(*indexBuffer->getBuffer(), indexBuffer->getIndexType());

		commandBuffer.drawIndexed(static_cast<uint32_t>(indexBuffer->getSize()));
	}
}
