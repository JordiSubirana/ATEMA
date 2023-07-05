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

#include <Atema/Graphics/Passes/ShadowPass.hpp>
#include <Atema/Graphics/FrameGraphBuilder.hpp>
#include <Atema/Graphics/RenderData.hpp>
#include <Atema/Graphics/VertexTypes.hpp>
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/SurfaceMaterial.hpp>
#include <Atema/Graphics/FrameGraphContext.hpp>
#include <Atema/Renderer/BufferLayout.hpp>
#include <Atema/Core/Utils.hpp>
#include <Atema/Core/TaskManager.hpp>
#include <Atema/Graphics/DirectionalLight.hpp>

using namespace at;

namespace
{
	constexpr uint32_t ShadowSetIndex = 0;
	constexpr uint32_t ObjectSetIndex = 1;
	
	struct ShadowLayoutData
	{
		ShadowLayoutData(StructLayout structLayout) : bufferLayout(structLayout)
		{
			/*struct ShadowData
			{
				mat4f viewProj;
			}*/

			viewProjectionOffset = bufferLayout.addMatrix(BufferElementType::Float, 4, 4);

		}

		BufferLayout bufferLayout;

		size_t viewProjectionOffset;
	};

	struct ShadowData
	{
		Matrix4f viewProjection;

		void copyTo(void* destData, StructLayout structLayout = StructLayout::Default)
		{
			ShadowLayoutData layoutData(structLayout);

			mapMemory<Matrix4f>(destData, layoutData.viewProjectionOffset) = viewProjection;
		}
	};

	inline IntersectionType getFrustumIntersection(const Frustumf& frustum, const AABBf& aabb)
	{
		const auto sphere = aabb.getBoundingSphere();

		const auto sphereIntersection = frustum.getIntersectionType(sphere);

		if (sphereIntersection != IntersectionType::Intersection)
			return sphereIntersection;

		return frustum.getIntersectionType(aabb);
	}

	constexpr char* ShaderName = "AtemaShadowPass";

	const char ShaderCode[] = R"(
struct ObjectData
{
	mat4f model;
}

struct ShadowData
{
	mat4f viewProjection;
}

external
{
	[set(0), binding(0)] ShadowData shadowData;
	
	[set(1), binding(0)] ObjectData objectData;
}

[stage(vertex)]
input
{
	[location(0)] vec3f inPosition;
	[location(1)] vec3f inNormal;
	[location(2)] vec3f inTangent;
	[location(3)] vec3f inBitangent;
	[location(4)] vec2f inTexCoords;
}

[entry(vertex)]
void main()
{
	vec4f worldPos = objectData.model * vec4f(inPosition, 1.0);
	
	vec4f screenPosition = shadowData.viewProjection * worldPos;
	
	setVertexPosition(screenPosition);
}

[stage(fragment)]
output
{
	[location(0)] vec4f outColor;
}

[entry(fragment)]
void main()
{
	outColor = vec4f(0.0, 0.0, 0.0, 1.0);
}
)";
}

ShadowPass::ShadowPass(size_t threadCount)
{
	const auto& taskManager = TaskManager::instance();
	const auto maxThreadCount = taskManager.getSize();
	m_threadCount = threadCount;
	if (threadCount == 0 || threadCount > maxThreadCount)
		m_threadCount = maxThreadCount;

	auto& graphics = Graphics::instance();

	if (!graphics.uberShaderExists(ShaderName))
		graphics.setUberShader(ShaderName, ShaderCode);

	DescriptorSetLayout::Settings descriptorSetLayoutSettings;
	descriptorSetLayoutSettings.bindings =
	{
		{ DescriptorType::UniformBuffer, 0, 1, ShaderStage::Vertex }
	};
	descriptorSetLayoutSettings.pageSize = Renderer::FramesInFlight;

	m_setLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);

	GraphicsPipeline::Settings pipelineSettings;
	pipelineSettings.vertexShader = graphics.getShader(*graphics.getUberShaderFromString(std::string(ShaderName), AstShaderStage::Vertex));
	pipelineSettings.fragmentShader = graphics.getShader(*graphics.getUberShaderFromString(std::string(ShaderName), AstShaderStage::Fragment));
	pipelineSettings.descriptorSetLayouts = { m_setLayout, graphics.getObjectLayout()};
	//TODO: Change vertex input depending on what mesh is drawn
	pipelineSettings.state.vertexInput.inputs = Vertex_XYZ_UV_NTB::getVertexInput();
	pipelineSettings.state.rasterization.depthClamp = true;

	m_pipeline = GraphicsPipeline::create(pipelineSettings);

	ShadowLayoutData layoutData(StructLayout::Default);

	Buffer::Settings bufferSettings;
	bufferSettings.usages = BufferUsage::Uniform | BufferUsage::Map;
	bufferSettings.byteSize = layoutData.bufferLayout.getSize();

	for (auto& frameData : m_frameDatas)
	{
		frameData.buffer = Buffer::create(bufferSettings);
		frameData.descriptorSet = m_setLayout->createSet();
		frameData.descriptorSet->update(0, *frameData.buffer);
	}
}

const char* ShadowPass::getName() const noexcept
{
	return "Shadow";
}

FrameGraphPass& ShadowPass::addToFrameGraph(FrameGraphBuilder& frameGraphBuilder, const Settings& settings)
{
	auto& pass = frameGraphBuilder.createPass(getName());

	if (settings.shadowMapClearValue.has_value())
		pass.setDepthTexture(settings.shadowMap, settings.shadowMapClearValue.value());
	else
		pass.setDepthTexture(settings.shadowMap);

	pass.enableSecondaryCommandBuffers(m_threadCount != 1);

	Settings settingsCopy = settings;

	pass.setExecutionCallback([this, settingsCopy](FrameGraphContext& context)
		{
			execute(context, settingsCopy);
		});

	return pass;
}

void ShadowPass::updateResources(RenderFrame& renderFrame, CommandBuffer& commandBuffer)
{
	auto& buffer = m_frameDatas[renderFrame.getFrameIndex()].buffer;

	auto data = buffer->map();

	mapMemory<Matrix4f>(data, 0) = m_viewProjection;

	buffer->unmap();
}

void ShadowPass::setViewProjection(const Matrix4f& viewProjection)
{
	m_viewProjection = viewProjection;
}

void ShadowPass::setFrustum(const Frustumf& frustum)
{
	m_frustum = frustum;
}

void ShadowPass::execute(FrameGraphContext& context, const Settings& settings)
{
	const auto& renderData = getRenderData();

	if (!renderData.isValid())
		return;

	const auto frameIndex = context.getFrameIndex();

	auto& frameData = m_frameDatas[frameIndex];

	auto& commandBuffer = context.getCommandBuffer();

	const auto shadowMapSize = settings.shadowMapSize;

	if (m_threadCount == 1)
	{
		drawElements(commandBuffer, frameData, 0, m_renderElements.size(), shadowMapSize);
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

			auto task = taskManager.createTask([this, &context, &frameData, &commandBuffers, taskIndex, firstIndex, size, shadowMapSize](size_t threadIndex)
				{
					auto commandBuffer = context.createSecondaryCommandBuffer(threadIndex);

					drawElements(*commandBuffer, frameData, firstIndex, size, shadowMapSize);

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

void ShadowPass::beginFrame()
{
	const auto& renderData = getRenderData();

	if (!renderData.isValid())
		return;

	frustumCull();
}

void ShadowPass::endFrame()
{
	m_renderElements.clear();
}

void ShadowPass::frustumCull()
{
	const auto& renderables = getRenderData().getRenderables();

	if (m_threadCount == 1)
	{
		frustumCullElements(m_renderElements, 0, renderables.size());
	}
	else
	{
		auto& taskManager = TaskManager::instance();

		std::vector<std::vector<RenderElement>> renderElements(m_threadCount);

		std::vector<Ptr<Task>> tasks;
		tasks.reserve(m_threadCount);

		size_t firstIndex = 0;
		size_t size = renderables.size() / m_threadCount;

		for (size_t taskIndex = 0; taskIndex < m_threadCount; taskIndex++)
		{
			if (taskIndex == m_threadCount - 1)
			{
				const auto remainingSize = renderables.size() - m_threadCount * size;

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

void ShadowPass::frustumCullElements(std::vector<RenderElement>& renderElements, size_t index, size_t count) const
{
	if (!count)
		return;

	const auto& renderables = getRenderData().getRenderables();

	std::vector<const Renderable*> visibleRenderables;
	std::vector<IntersectionType> renderableIntersections;

	// Keep only visible renderables using frustum culling
	visibleRenderables.reserve(count);
	renderableIntersections.reserve(count);

	size_t renderElementsSize = 0;

	for (size_t i = index; i < index + count; i++)
	{
		auto& renderable = *renderables[i];

		if (!renderable.castShadows())
			continue;

		const auto intersectionType = getFrustumIntersection(m_frustum, renderable.getAABB());

		if (intersectionType != IntersectionType::Outside)
		{
			visibleRenderables.emplace_back(&renderable);
			renderableIntersections.emplace_back(intersectionType);

			renderElementsSize += renderable.getRenderElementsSize();
		}
	}

	renderElements.reserve(renderElementsSize);

	for (size_t i = 0; i < visibleRenderables.size(); i++)
	{
		const auto& renderable = *visibleRenderables[i];

		renderable.getRenderElements(renderElements);
	}
}

void ShadowPass::drawElements(CommandBuffer& commandBuffer, FrameData& frameData, size_t index, size_t count, uint32_t shadowMapSize)
{
	if (!count)
		return;

	Viewport viewport;
	viewport.size = { shadowMapSize, shadowMapSize };

	commandBuffer.bindPipeline(*m_pipeline);

	commandBuffer.setViewport(viewport);

	commandBuffer.setScissor(Vector2i(), { shadowMapSize, shadowMapSize });

	commandBuffer.bindDescriptorSet(ShadowSetIndex, *frameData.descriptorSet);

	for (size_t i = index; i < index + count; i++)
	{
		const auto& renderElement = m_renderElements[i];
		const auto binding = renderElement.objectBinding;

		if (binding)
			commandBuffer.bindDescriptorSet(ObjectSetIndex, *binding->descriptorSet, binding->dynamicBufferOffsets.data(), binding->dynamicBufferOffsets.size());

		commandBuffer.bindVertexBuffer(*renderElement.vertexBuffer->getBuffer(), 0);

		commandBuffer.bindIndexBuffer(*renderElement.indexBuffer->getBuffer(), renderElement.indexBuffer->getIndexType());

		commandBuffer.drawIndexed(renderElement.indexBuffer->getSize());
	}
}
