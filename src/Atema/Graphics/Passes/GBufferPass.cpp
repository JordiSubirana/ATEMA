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
#include <Atema/Graphics/RenderData.hpp>
#include <Atema/Graphics/FrameGraphContext.hpp>
#include <Atema/Graphics/Camera.hpp>
#include <Atema/Graphics/SurfaceMaterial.hpp>
#include <Atema/Graphics/VertexBuffer.hpp>
#include <Atema/Graphics/IndexBuffer.hpp>
#include <Atema/Core/TaskManager.hpp>

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
		if (!renderElement.materialInstance || !renderElement.materialInstance->getMaterial())
			return 0;

		uint64_t renderPriority = 0;
		renderPriority |= static_cast<uint64_t>(renderElement.materialInstance->getMaterial()->getID() & 0xFFFF) << 16;
		renderPriority |= static_cast<uint64_t>(renderElement.materialInstance->getID() & 0xFFFF) << 0;

		return renderPriority;
	}
}

GBufferPass::GBufferPass() :
	AbstractRenderPass()
{
	const auto bufferLayout = SurfaceMaterial::FrameData::getBufferLayout();

	Buffer::Settings bufferSettings;
	bufferSettings.usages = BufferUsage::Uniform | BufferUsage::Map;
	bufferSettings.byteSize = bufferLayout.getSize();

	auto frameLayout = SurfaceMaterial::getFrameLayout();

	for (auto& frameData : m_frameDatas)
	{
		frameData.buffer = Buffer::create(bufferSettings);
		frameData.descriptorSet = frameLayout->createSet();
		frameData.descriptorSet->update(0, *frameData.buffer);
	}
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

	pass.enableSecondaryCommandBuffers(settings.threadCount != 1);

	Settings settingsCopy = settings;

	pass.setExecutionCallback([this, settingsCopy](FrameGraphContext& context)
		{
			execute(context, settingsCopy);
		});

	return pass;
}

void GBufferPass::execute(FrameGraphContext& context, const Settings& settings)
{
	const auto& renderData = getRenderData();

	//if (!renderData.isValid() || m_renderElements.empty())
	if (!renderData.isValid())
		return;

	const auto frameIndex = context.getFrameIndex();

	auto& frameData = m_frameDatas[frameIndex];

	updateFrameData(frameData);

	auto& commandBuffer = context.getCommandBuffer();

	if (settings.threadCount == 1)
	{
		drawElements(commandBuffer, frameData, 0, m_renderElements.size());
	}
	else
	{
		auto& taskManager = TaskManager::instance();
		const auto maxThreadCount = taskManager.getSize();
		size_t threadCount = settings.threadCount;
		if (threadCount == 0 || threadCount > maxThreadCount)
			threadCount = maxThreadCount;

		std::vector<Ptr<Task>> tasks;
		tasks.reserve(threadCount);

		std::vector<Ptr<CommandBuffer>> commandBuffers;
		commandBuffers.resize(threadCount);

		size_t firstIndex = 0;
		size_t size = m_renderElements.size() / threadCount;

		for (size_t taskIndex = 0; taskIndex < threadCount; taskIndex++)
		{
			if (taskIndex == threadCount - 1)
			{
				const auto remainingSize = m_renderElements.size() - threadCount * size;

				size += remainingSize;
			}

			auto task = taskManager.createTask([this, &context, &frameData, &commandBuffers, taskIndex, firstIndex, size](size_t threadIndex)
				{
					auto commandBuffer = context.createSecondaryCommandBuffer(threadIndex);

					drawElements(*commandBuffer, frameData, firstIndex, size);

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

void GBufferPass::doBeginFrame()
{
	const auto& renderData = getRenderData();

	if (!renderData.isValid())
		return;

	frustumCull();

	sortElements();
}

void GBufferPass::doEndFrame()
{
	m_renderables.clear();
	m_renderableIntersections.clear();
	m_renderElements.clear();
}

void GBufferPass::frustumCull()
{
	const auto& renderData = getRenderData();
	const auto& camera = renderData.getCamera();
	const auto& renderables = renderData.getRenderables();

	const auto& frustum = camera.getFrustum();

	// Keep only visible renderables using frustum culling
	m_renderables.reserve(renderables.size());
	m_renderableIntersections.reserve(renderables.size());

	size_t renderElementsSize = 0;

	for (const auto& renderable : renderables)
	{
		const auto intersectionType = getFrustumIntersection(frustum, renderable->getAABB());

		if (intersectionType != IntersectionType::Outside)
		{
			m_renderables.emplace_back(renderable);
			m_renderableIntersections.emplace_back(intersectionType);

			renderElementsSize += renderable->getRenderElementsSize();
		}
	}

	// Cull individual elements if needed
	m_renderElements.reserve(renderElementsSize);

	std::vector<RenderElement> renderElements;

	for (size_t i = 0; i < m_renderables.size(); i++)
	{
		const auto& renderable = m_renderables[i];

		// Renderable is fully contained : every element also is
		if (m_renderableIntersections[i] == IntersectionType::Inside)
		{
			renderable->getRenderElements(m_renderElements);
		}
		// Renderable is intersecting with the frustum : test every element
		else
		{
			renderElements.clear();
			renderElements.reserve(renderable->getRenderElementsSize());

			renderable->getRenderElements(renderElements);

			const auto& matrix = renderable->getTransform().getMatrix();

			for (auto& renderElement : renderElements)
			{
				if (getFrustumIntersection(frustum, matrix * renderElement.aabb) != IntersectionType::Outside)
					m_renderElements.emplace_back(std::move(renderElement));
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

void GBufferPass::updateFrameData(FrameData& frameData)
{
	auto& buffer = frameData.buffer;
	const auto& camera = getRenderData().getCamera();

	SurfaceMaterial::FrameData surfaceFrameData;
	surfaceFrameData.cameraPosition = camera.getPosition();
	surfaceFrameData.view = camera.getViewMatrix();
	surfaceFrameData.projection = camera.getProjectionMatrix();
	
	auto data = buffer->map();

	surfaceFrameData.copyTo(data);

	buffer->unmap();
}

void GBufferPass::drawElements(CommandBuffer& commandBuffer, FrameData& frameData, size_t index, size_t count)
{
	const auto& viewport = getRenderData().getCamera().getViewport();
	const auto& scissor = getRenderData().getCamera().getScissor();

	commandBuffer.setViewport(viewport);

	commandBuffer.setScissor(scissor.pos, scissor.size);

	if (!count)
		return;

	auto currentMaterialID = SurfaceMaterial::InvalidID;
	auto currentMaterialInstanceID = SurfaceMaterial::InvalidID;

	// Initialize material to the first one and bind global frame data shared across all pipelines
	{
		auto& materialInstance = m_renderElements[index].materialInstance;
		auto& material = materialInstance->getMaterial();

		material->bindTo(commandBuffer);
		materialInstance->bindTo(commandBuffer);

		currentMaterialID = material->getID();
		currentMaterialInstanceID = materialInstance->getID();
	}

	commandBuffer.bindDescriptorSet(SurfaceMaterial::FrameSetIndex, *frameData.descriptorSet);

	for (size_t i = index; i < index + count; i++)
	{
		auto& renderElement = m_renderElements[i];
		
		auto& materialInstance = renderElement.materialInstance;
		auto& material = materialInstance->getMaterial();

		auto materialID = material->getID();
		auto materialInstanceID = materialInstance->getID();

		if (materialID != currentMaterialID)
		{
			material->bindTo(commandBuffer);

			currentMaterialID = materialID;

			currentMaterialInstanceID = SurfaceMaterial::InvalidID;
		}

		if (materialInstanceID != currentMaterialInstanceID)
		{
			materialInstance->bindTo(commandBuffer);

			currentMaterialInstanceID = materialInstanceID;
		}

		if (renderElement.objectBinding)
			renderElement.objectBinding->bindTo(commandBuffer);

		const auto& vertexBuffer = renderElement.vertexBuffer;
		const auto& indexBuffer = renderElement.indexBuffer;

		commandBuffer.bindVertexBuffer(*vertexBuffer->getBuffer(), 0);

		commandBuffer.bindIndexBuffer(*indexBuffer->getBuffer(), indexBuffer->getIndexType());

		commandBuffer.drawIndexed(indexBuffer->getSize());
	}
}
