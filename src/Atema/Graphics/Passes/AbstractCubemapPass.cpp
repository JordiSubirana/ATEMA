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

#include <Atema/Core/MemoryMapper.hpp>
#include <Atema/Graphics/FrameGraphBuilder.hpp>
#include <Atema/Graphics/IndexBuffer.hpp>
#include <Atema/Graphics/Primitive.hpp>
#include <Atema/Graphics/RenderMaterial.hpp>
#include <Atema/Graphics/VertexBuffer.hpp>
#include <Atema/Graphics/Passes/AbstractCubemapPass.hpp>
#include <Atema/Renderer/BufferLayout.hpp>
#include <Atema/Renderer/DescriptorSet.hpp>
#include <Atema/Renderer/Viewport.hpp>

using namespace at;

namespace
{
	constexpr uint32_t CubeDataSetIndex = 0;
}

AbstractCubemapPass::AbstractCubemapPass() :
	AbstractRenderPass()
{
	// Plane mesh
	ModelLoader::Settings loaderSettings(VertexFormat::create(DefaultVertexFormat::XYZ));
	loaderSettings.vertexTransformation = Matrix4f::createTranslation(Vector3f(0.0f, 0.0f, -1.0f));

	m_mesh = Primitive::createPlane(loaderSettings, Vector3f(0, 0, 1), 2.0f, 2.0f, 1, 1);

	// Buffer
	Matrix4f projection = Matrix4f::createPerspective(Math::toRadians(90.0f), 1.0f, 0.1f, 10.0f);
	projection[1][1] *= -1;

	const std::array<Matrix4f, 6> models =
	{
		Matrix4f::createRotation(Vector3f(Math::toRadians(90.0f), 0.0f, 0.0f)), // Top
		Matrix4f::createRotation(Vector3f(0.0f, 0.0f, Math::toRadians(-90.0f))), // Left
		Matrix4f::createRotation(Vector3f(0.0f, 0.0f, Math::toRadians(0.0f))), // Front
		Matrix4f::createRotation(Vector3f(0.0f, 0.0f, Math::toRadians(90.0f))), // Right
		Matrix4f::createRotation(Vector3f(0.0f, 0.0f, Math::toRadians(180.0f))), // Back
		Matrix4f::createRotation(Vector3f(Math::toRadians(-90.0f), 0.0f, 0.0f)) // Bottom
	};

	BufferLayout layout;
	layout.addMatrixArray(BufferElementType::Float, 4, 4, true, 6);

	Buffer::Settings bufferSettings;
	bufferSettings.byteSize = layout.getSize();
	bufferSettings.usages = BufferUsage::Uniform | BufferUsage::Map;

	m_cubeDataBuffer = Buffer::create(bufferSettings);

	void* cubeData = m_cubeDataBuffer->map();

	MemoryMapper matrixMapper(cubeData, 0, sizeof(Matrix4f));
	for (size_t i = 0; i < 6; i++)
		matrixMapper.map<Matrix4f>(i) = projection * models[i];

	m_cubeDataBuffer->unmap();
}

void AbstractCubemapPass::initialize(Ptr<Material> material, Ptr<RenderMaterial> renderMaterial)
{
	m_material = std::move(material);
	m_renderMaterial = std::move(renderMaterial);

	m_cubeDescriptorSet = m_renderMaterial->createSet(CubeDataSetIndex);
	m_cubeDescriptorSet->update(0, *m_cubeDataBuffer);
}

RenderMaterial& AbstractCubemapPass::getRenderMaterial() const
{
	ATEMA_ASSERT(m_renderMaterial, "RenderMaterial was not initialized");

	return *m_renderMaterial;
}

size_t AbstractCubemapPass::getCubemapFaceIndex(CubemapFace face)
{
	return static_cast<size_t>(face);
}

FrameGraphPass& AbstractCubemapPass::createFrameGraphPass(FrameGraphBuilder& frameGraphBuilder, const BaseFrameGraphSettings& settings)
{
	ATEMA_ASSERT(settings.outputCubemapFaces.size() == 6, "All cubemap faces are required");

	auto& frameGraphPass = frameGraphBuilder.createPass(getName());

	uint32_t index = 0;
	for (auto& texture : settings.outputCubemapFaces)
	{
		if (settings.clearColor)
			frameGraphPass.addOutputTexture(texture, index++, settings.clearColor.value());
		else
			frameGraphPass.addOutputTexture(texture, index++);
	}

	return frameGraphPass;
}

void AbstractCubemapPass::initializeDraw(FrameGraphContext& context, const BaseFrameGraphSettings& settings)
{
	ATEMA_ASSERT(m_renderMaterial, "RenderMaterial was not initialized");

	if (settings.outputCubemapFaces.empty())
		return;

	Vector2u imageSize = context.getTexture(settings.outputCubemapFaces[0])->getSize();
	const uint32_t mipLevel = context.getImageView(settings.outputCubemapFaces[0])->getBaseMipLevel();
	imageSize.x /= 1 << mipLevel;
	imageSize.y /= 1 << mipLevel;

	auto& commandBuffer = context.getCommandBuffer();

	Viewport viewport;
	viewport.size = imageSize;

	commandBuffer.setViewport(viewport);

	commandBuffer.setScissor(Vector2i(0, 0), imageSize);

	m_renderMaterial->bindTo(commandBuffer);

	commandBuffer.bindDescriptorSet(CubeDataSetIndex, *m_cubeDescriptorSet);
}

void AbstractCubemapPass::draw(FrameGraphContext& context, const BaseFrameGraphSettings& settings)
{
	auto& commandBuffer = context.getCommandBuffer();

	const VertexBuffer& vertexBuffer = *m_mesh->getVertexBuffer();
	const IndexBuffer& indexBuffer = *m_mesh->getIndexBuffer();

	commandBuffer.bindVertexBuffer(*vertexBuffer.getBuffer(), 0);
	commandBuffer.bindIndexBuffer(*indexBuffer.getBuffer(), indexBuffer.getIndexType());

	commandBuffer.drawIndexed(static_cast<uint32_t>(indexBuffer.getSize()));
}
