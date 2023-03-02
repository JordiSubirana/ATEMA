/*
	Copyright 2022 Jordi SUBIRANA

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

#include <Atema/Graphics/DebugRenderer.hpp>
#include <Atema/Graphics/FrameGraphContext.hpp>
#include <Atema/Graphics/VertexBuffer.hpp>
#include <Atema/Renderer/BufferLayout.hpp>
#include <Atema/Renderer/CommandBuffer.hpp>
#include <Atema/Renderer/GraphicsPipeline.hpp>
#include <Atema/Renderer/Renderer.hpp>
#include <Atema/Renderer/Shader.hpp>
#include <Atema/Shader/Ast/Enums.hpp>

using namespace at;

namespace
{
	const char* debugVS = R"(
	struct FrameData
	{
		mat4f viewProj;
	}

	external
	{
		[set(0), binding(0)] FrameData frameData;
	}

	[stage(vertex)]
	input
	{
		[location(0)] vec3f inPosition;
		[location(1)] vec3f inColor;
	}

	[stage(vertex)]
	output
	{
		[location(0)] vec3f outColor;
	}

	[entry(vertex)]
	void main()
	{
		vec4f screenPosition = frameData.viewProj * vec4f(inPosition, 1.0);

		setVertexPosition(screenPosition);

		outColor = inColor;
	})";

	const char* debugFS = R"(
	[stage(fragment)]
	input
	{
		[location(0)] vec3f inColor;
	}

	[stage(fragment)]
	output
	{
		[location(0)] vec4f outColor;
	}

	[entry(fragment)]
	void main()
	{
		outColor = vec4f(inColor, 1.0);
	})";

	Vector3f getColor(const Color& color)
	{
		return { color.r, color.g, color.b };
	}
}

DebugRenderer::DebugRenderer()
{
	// Vertex format
	m_vertexFormat = VertexFormat::create(DefaultVertexFormat::XYZ_RGB);

	// Uniform buffer
	BufferLayout bufferLayout(StructLayout::Default);
	bufferLayout.addMatrix(BufferElementType::Float, 4, 4);

	Buffer::Settings bufferSettings;
	bufferSettings.usages = BufferUsage::Uniform | BufferUsage::Map;
	bufferSettings.byteSize = bufferLayout.getSize();

	m_uniformBuffer = Buffer::create(bufferSettings);

	// Pipeline state
	GraphicsPipeline::Settings pipelineSettings;
	auto& pipelineState = pipelineSettings.state;
	
	for (const auto& component : m_vertexFormat->getComponents())
		pipelineState.vertexInput.inputs.emplace_back(component);

	pipelineState.inputAssembly.topology = PrimitiveTopology::LineList;

	//pipelineState.rasterization.polygonMode = PolygonMode::Line;

	pipelineState.depth.test = true;
	pipelineState.depth.write = false;

	// Descriptor set layout
	DescriptorSetLayout::Settings descriptorSetLayoutSettings;
	descriptorSetLayoutSettings.bindings =
	{
		{ DescriptorType::UniformBuffer, 0, 1, ShaderStage::Vertex }
	};
	descriptorSetLayoutSettings.pageSize = 1;

	m_descriptorSetLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);

	pipelineSettings.descriptorSetLayouts = { m_descriptorSetLayout };

	// Descriptor set
	m_descriptorSet = m_descriptorSetLayout->createSet();
	m_descriptorSet->update(0, m_uniformBuffer);

	// Load shaders
	Shader::Settings shaderSettings;
	shaderSettings.shaderLanguage = ShaderLanguage::Atsl;

	shaderSettings.shaderData = debugVS;
	shaderSettings.shaderDataSize = std::char_traits<char>::length(debugVS);
	pipelineSettings.vertexShader = Shader::create(shaderSettings);

	shaderSettings.shaderData = debugFS;
	shaderSettings.shaderDataSize = std::char_traits<char>::length(debugFS);
	pipelineSettings.fragmentShader = Shader::create(shaderSettings);

	m_graphicsPipeline = GraphicsPipeline::create(pipelineSettings);
}

DebugRenderer::~DebugRenderer()
{
	Renderer::instance().waitForIdle();

	m_vertexBuffers.clear();
	m_obsoleteVertexBuffers.clear();
}

void DebugRenderer::clear()
{
	for (auto& vertexBuffer : m_vertexBuffers)
		m_obsoleteVertexBuffers.emplace_back(std::move(vertexBuffer));

	m_vertexBuffers.clear();
}

void DebugRenderer::drawLine(const Vector3f& point1, const Vector3f& point2, const Color& color)
{
	const auto c = getColor(color);

	m_vertices.emplace_back(point1, c);
	m_vertices.emplace_back(point2, c);
}

void DebugRenderer::draw(const AABBf& aabb, const Color& color)
{
	const auto c = getColor(color);

	m_vertices.emplace_back(Vector3f(aabb.min.x, aabb.min.y, aabb.min.z), c);
	m_vertices.emplace_back(Vector3f(aabb.min.x, aabb.min.y, aabb.max.z), c);
	m_vertices.emplace_back(Vector3f(aabb.min.x, aabb.min.y, aabb.min.z), c);
	m_vertices.emplace_back(Vector3f(aabb.min.x, aabb.max.y, aabb.min.z), c);
	m_vertices.emplace_back(Vector3f(aabb.min.x, aabb.min.y, aabb.min.z), c);
	m_vertices.emplace_back(Vector3f(aabb.max.x, aabb.min.y, aabb.min.z), c);

	m_vertices.emplace_back(Vector3f(aabb.min.x, aabb.max.y, aabb.max.z), c);
	m_vertices.emplace_back(Vector3f(aabb.min.x, aabb.max.y, aabb.min.z), c);
	m_vertices.emplace_back(Vector3f(aabb.min.x, aabb.max.y, aabb.max.z), c);
	m_vertices.emplace_back(Vector3f(aabb.min.x, aabb.min.y, aabb.max.z), c);
	m_vertices.emplace_back(Vector3f(aabb.min.x, aabb.max.y, aabb.max.z), c);
	m_vertices.emplace_back(Vector3f(aabb.max.x, aabb.max.y, aabb.max.z), c);

	m_vertices.emplace_back(Vector3f(aabb.max.x, aabb.min.y, aabb.max.z), c);
	m_vertices.emplace_back(Vector3f(aabb.max.x, aabb.min.y, aabb.min.z), c);
	m_vertices.emplace_back(Vector3f(aabb.max.x, aabb.min.y, aabb.max.z), c);
	m_vertices.emplace_back(Vector3f(aabb.max.x, aabb.max.y, aabb.max.z), c);
	m_vertices.emplace_back(Vector3f(aabb.max.x, aabb.min.y, aabb.max.z), c);
	m_vertices.emplace_back(Vector3f(aabb.min.x, aabb.min.y, aabb.max.z), c);

	m_vertices.emplace_back(Vector3f(aabb.max.x, aabb.max.y, aabb.min.z), c);
	m_vertices.emplace_back(Vector3f(aabb.max.x, aabb.max.y, aabb.max.z), c);
	m_vertices.emplace_back(Vector3f(aabb.max.x, aabb.max.y, aabb.min.z), c);
	m_vertices.emplace_back(Vector3f(aabb.max.x, aabb.min.y, aabb.min.z), c);
	m_vertices.emplace_back(Vector3f(aabb.max.x, aabb.max.y, aabb.min.z), c);
	m_vertices.emplace_back(Vector3f(aabb.min.x, aabb.max.y, aabb.min.z), c);
}

void DebugRenderer::draw(const Frustumf& frustum, const Color& color)
{
	const auto& nbl = frustum.getCorner(FrustumCorner::NearBottomLeft);
	const auto& nbr = frustum.getCorner(FrustumCorner::NearBottomRight);
	const auto& ntl = frustum.getCorner(FrustumCorner::NearTopLeft);
	const auto& ntr = frustum.getCorner(FrustumCorner::NearTopRight);
	const auto& fbl = frustum.getCorner(FrustumCorner::FarBottomLeft);
	const auto& fbr = frustum.getCorner(FrustumCorner::FarBottomRight);
	const auto& ftl = frustum.getCorner(FrustumCorner::FarTopLeft);
	const auto& ftr = frustum.getCorner(FrustumCorner::FarTopRight);

	const auto c = getColor(color);

	m_vertices.emplace_back(nbl, c);
	m_vertices.emplace_back(nbr, c);
	m_vertices.emplace_back(nbr, c);
	m_vertices.emplace_back(ntr, c);
	m_vertices.emplace_back(ntr, c);
	m_vertices.emplace_back(ntl, c);
	m_vertices.emplace_back(ntl, c);
	m_vertices.emplace_back(nbl, c);

	m_vertices.emplace_back(fbl, c);
	m_vertices.emplace_back(fbr, c);
	m_vertices.emplace_back(fbr, c);
	m_vertices.emplace_back(ftr, c);
	m_vertices.emplace_back(ftr, c);
	m_vertices.emplace_back(ftl, c);
	m_vertices.emplace_back(ftl, c);
	m_vertices.emplace_back(fbl, c);

	m_vertices.emplace_back(nbl, c);
	m_vertices.emplace_back(fbl, c);
	m_vertices.emplace_back(nbr, c);
	m_vertices.emplace_back(fbr, c);
	m_vertices.emplace_back(ntr, c);
	m_vertices.emplace_back(ftr, c);
	m_vertices.emplace_back(ntl, c);
	m_vertices.emplace_back(ftl, c);
}

void DebugRenderer::render(FrameGraphContext& frameGraphContext, CommandBuffer& commandBuffer, const Matrix4f& viewProjection)
{
	if (!m_vertices.empty())
	{
		VertexBuffer::Settings settings;
		settings.usages = BufferUsage::Map;
		settings.vertexFormat = m_vertexFormat;
		settings.size = m_vertices.size();

		auto vertexBuffer = std::make_shared<VertexBuffer>(settings);

		memcpy(vertexBuffer->map(), m_vertices.data(), m_vertices.size() * sizeof(Vertex));

		vertexBuffer->unmap();

		m_vertexBuffers.emplace_back(std::move(vertexBuffer));

		m_vertices.clear();
	}

	if (m_vertexBuffers.empty())
		return;

	auto data = m_uniformBuffer->map();

	mapMemory<Matrix4f>(data, 0) = viewProjection;

	m_uniformBuffer->unmap();

	commandBuffer.bindPipeline(*m_graphicsPipeline);

	commandBuffer.bindDescriptorSet(0, *m_descriptorSet);

	for (const auto& vertexBuffer : m_vertexBuffers)
	{
		commandBuffer.bindVertexBuffer(*vertexBuffer->getBuffer(), 0);

		commandBuffer.draw(static_cast<uint32_t>(vertexBuffer->getSize()));
	}

	for (auto& vertexBuffer : m_obsoleteVertexBuffers)
		frameGraphContext.destroyAfterUse(std::move(vertexBuffer));

	m_obsoleteVertexBuffers.clear();
}
