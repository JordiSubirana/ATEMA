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

#include <Atema/Graphics/IndexBuffer.hpp>
#include <Atema/Graphics/Mesh.hpp>
#include <Atema/Graphics/Loaders/ModelLoader.hpp>
#include <Atema/Graphics/VertexFormat.hpp>
#include <Atema/Graphics/VertexBuffer.hpp>
#include <Atema/Math/Math.hpp>
#include <Atema/Renderer/Renderer.hpp>

using namespace at;

namespace
{
	void vec2fToRG32f(MemoryMapper& src, MemoryMapper& dst, size_t size)
	{
		for (size_t i = 0; i < size; i++)
			dst.map<Vector2f>(i) = src.map<Vector2f>(i);
	}

	void vec2fToRG64f(MemoryMapper& src, MemoryMapper& dst, size_t size)
	{
		for (size_t i = 0; i < size; i++)
		{
			const auto& vec = src.map<Vector2f>(i);
			dst.map<Vector2d>(i) = { vec.x, vec.y };
		}
	}

	void vec3fToRGB32f (MemoryMapper& src, MemoryMapper& dst, size_t size)
	{
		for (size_t i = 0; i < size; i++)
			dst.map<Vector3f>(i) = src.map<Vector3f>(i);
	}

	void vec3fToRGBA32f(MemoryMapper& src, MemoryMapper& dst, size_t size, float w)
	{
		for (size_t i = 0; i < size; i++)
		{
			const auto& vec = src.map<Vector3f>(i);
			dst.map<Vector4f>(i) = { vec.x, vec.y, vec.z, w };
		}
	}

	void vec3fToRGB64f(MemoryMapper& src, MemoryMapper& dst, size_t size)
	{
		for (size_t i = 0; i < size; i++)
		{
			const auto& vec = src.map<Vector3f>(i);
			dst.map<Vector3d>(i) = { vec.x, vec.y, vec.z };
		}
	}

	void vec3fToRGBA64f(MemoryMapper& src, MemoryMapper& dst, size_t size, double w)
	{
		for (size_t i = 0; i < size; i++)
		{
			const auto& vec = src.map<Vector3f>(i);
			dst.map<Vector4d>(i) = { static_cast<double>(vec.x), static_cast<double>(vec.y), static_cast<double>(vec.z), w };
		}
	}

	void copyComponent(VertexInputFormat format, MemoryMapper& src, MemoryMapper& dst, size_t size, double w = 0.0)
	{
		switch (format)
		{
			case VertexInputFormat::RG32_SFLOAT:
			{
				vec2fToRG32f(src, dst, size);
				break;
			}
			case VertexInputFormat::RGB32_SFLOAT:
			{
				vec3fToRGB32f(src, dst, size);
				break;
			}
			case VertexInputFormat::RGBA32_SFLOAT:
			{
				vec3fToRGBA32f(src, dst, size, static_cast<float>(w));
				break;
			}
			case VertexInputFormat::RG64_SFLOAT:
			{
				vec2fToRG64f(src, dst, size);
				break;
			}
			case VertexInputFormat::RGB64_SFLOAT:
			{
				vec3fToRGB64f(src, dst, size);
				break;
			}
			case VertexInputFormat::RGBA64_SFLOAT:
			{
				vec3fToRGBA64f(src, dst, size, w);
				break;
			}
			default:
			{
				ATEMA_ERROR("Invalid component format");
			}
		}
	}
}

//-----
// ModelLoader::StaticVertex
bool ModelLoader::StaticVertex::operator==(const StaticVertex& other) const
{
	// Since position / texCoords / normal are not computed but loaded, we can use == operator
	// For tangent / bitangent, we use equals because they may have been externally computed so there may be small errors
	return position == other.position &&
		texCoords == other.texCoords &&
		normal == other.normal &&
		tangent.equals(other.tangent) &&
		bitangent.equals(other.bitangent);
}

//-----
// ModelLoader::Settings
ModelLoader::Settings::Settings(const Ptr<const VertexFormat>& vertexFormat) :
	m_vertexFormat(vertexFormat),
	stagingBuffers(nullptr)
{
	ensureVertexFormatValid();
}

void ModelLoader::Settings::ensureVertexFormatValid() const
{
	if (m_vertexFormat && m_vertexFormat->getByteSize() > 0)
	{
		if (m_vertexFormat->hasComponent(VertexComponentType::Position))
		{
			const auto& component = m_vertexFormat->getComponent(VertexComponentType::Position);
			ATEMA_ASSERT(component.isFloatingPoint() && component.getElementSize() >= 3, "Position component must have at least 3 floating point elements");
		}
		else
		{
			ATEMA_ERROR("Position component is required");
		}

		if (m_vertexFormat->hasComponent(VertexComponentType::TexCoords))
		{
			const auto& component = m_vertexFormat->getComponent(VertexComponentType::TexCoords);
			ATEMA_ASSERT(component.isFloatingPoint() && component.getElementSize() == 3, "TexCoords component must have 2 floating point elements");
		}

		if (m_vertexFormat->hasComponent(VertexComponentType::Normal))
		{
			const auto& component = m_vertexFormat->getComponent(VertexComponentType::Normal);
			ATEMA_ASSERT(component.isFloatingPoint() && component.getElementSize() >= 3, "Normal component must have at least 3 floating point elements");
		}

		if (m_vertexFormat->hasComponent(VertexComponentType::Tangent))
		{
			const auto& component = m_vertexFormat->getComponent(VertexComponentType::Tangent);
			ATEMA_ASSERT(component.isFloatingPoint() && component.getElementSize() >= 3, "Tangent component must have at least 3 floating point elements");
		}

		if (m_vertexFormat->hasComponent(VertexComponentType::Bitangent))
		{
			const auto& component = m_vertexFormat->getComponent(VertexComponentType::Bitangent);
			ATEMA_ASSERT(component.isFloatingPoint() && component.getElementSize() >= 3, "Bitangent component must have at least 3 floating point elements");
		}
	}
	else
	{
		ATEMA_ERROR("No vertex format specified");
	}
}

const Ptr<const VertexFormat>& ModelLoader::Settings::getVertexFormat() const
{
	return m_vertexFormat;
}

//-----
// ModelLoader
void ModelLoader::generateTangents(std::vector<StaticVertex>& vertices, std::vector<uint32_t>& indices)
{
	std::vector<StaticVertex> newVertices;
	std::vector<uint32_t> newIndices;

	const auto triangleCount = indices.size() / 3;

	uint32_t newIndex = 0;
	for (size_t triangleIndex = 0; triangleIndex < triangleCount; triangleIndex++)
	{
		const auto firstVertexIndex = triangleIndex * 3;

		auto& v1 = vertices[indices[firstVertexIndex]];
		auto& v2 = vertices[indices[firstVertexIndex + 1]];
		auto& v3 = vertices[indices[firstVertexIndex + 2]];

		auto edge1 = v2.position - v1.position;
		auto edge2 = v3.position - v1.position;

		auto deltaUV1 = v2.texCoords - v1.texCoords;
		auto deltaUV2 = v3.texCoords - v1.texCoords;

		Matrix3x2f edgeMatrix;
		edgeMatrix[0] = { edge1.x, edge2.x };
		edgeMatrix[1] = { edge1.y, edge2.y };
		edgeMatrix[2] = { edge1.z, edge2.z };

		Matrix2f deltaUVMatrix;
		deltaUVMatrix[0] = { deltaUV1.x, deltaUV2.x };
		deltaUVMatrix[1] = { deltaUV1.y, deltaUV2.y };

		auto tbMatrix = deltaUVMatrix.inverse() * edgeMatrix;

		Vector3f tangent = { tbMatrix[0].x, tbMatrix[1].x, tbMatrix[2].x };
		Vector3f bitangent = { tbMatrix[0].y, tbMatrix[1].y, tbMatrix[2].y };

		tangent.normalize();
		bitangent.normalize();

		// Remove invalid triangles
		if (tangent.getSquaredNorm() < 0.1f || bitangent.getSquaredNorm() < 0.1f)
			continue;

		v1.tangent = tangent;
		v1.bitangent = bitangent;

		v2.tangent = tangent;
		v2.bitangent = bitangent;

		v3.tangent = tangent;
		v3.bitangent = bitangent;

		newIndices.emplace_back(newIndex++);
		newVertices.emplace_back(v1);
		newIndices.emplace_back(newIndex++);
		newVertices.emplace_back(v2);
		newIndices.emplace_back(newIndex++);
		newVertices.emplace_back(v3);
	}

	removeDuplicates(newVertices, newIndices);

	std::swap(vertices, newVertices);
	std::swap(indices, newIndices);
}

void ModelLoader::removeDuplicates(std::vector<StaticVertex>& vertices, std::vector<uint32_t>& indices)
{
	std::unordered_map<StaticVertex, uint32_t, DefaultStdHasher> uniqueVertices{};

	std::vector<StaticVertex> newVertices;
	std::vector<uint32_t> newIndices;

	for (auto& vertex : vertices)
	{
		if (uniqueVertices.count(vertex) == 0)
		{
			const auto vertexIndex = static_cast<uint32_t>(newVertices.size());

			newIndices.push_back(vertexIndex);
			newVertices.push_back(vertex);

			uniqueVertices[vertex] = vertexIndex;
		}
		else
		{
			newIndices.push_back(uniqueVertices[vertex]);
		}
	}

	std::swap(vertices, newVertices);
	std::swap(indices, newIndices);
}

Ptr<Mesh> ModelLoader::loadMesh(std::vector<StaticVertex>& vertices, std::vector<uint32_t>& indices, const Settings& settings)
{
	const auto& format = *settings.getVertexFormat();

	const bool hasTexCoords = format.hasComponent(VertexComponentType::TexCoords);
	const bool hasNormal = format.hasComponent(VertexComponentType::Normal);
	const bool hasTangent = format.hasComponent(VertexComponentType::Tangent);
	const bool hasBitangent = format.hasComponent(VertexComponentType::Bitangent);

	//-----
	// Process vertices & indices

	// Transform vertices if needed
	if (settings.vertexTransformation.has_value())
	{
		const auto& transform = settings.vertexTransformation.value();

		for (auto& vertex : vertices)
			vertex.position = transform.transformPosition(vertex.position);

		if (hasNormal)
		{
			for (auto& vertex : vertices)
				vertex.normal = transform.transformPosition(vertex.normal);
		}

		if (hasTangent)
		{
			for (auto& vertex : vertices)
				vertex.tangent = transform.transformPosition(vertex.tangent);
		}

		if (hasBitangent)
		{
			for (auto& vertex : vertices)
				vertex.bitangent = transform.transformPosition(vertex.bitangent);
		}
	}

	if (hasTexCoords && settings.flipTexCoords)
	{
		for (auto& vertex : vertices)
			vertex.texCoords.y = 1.0f - vertex.texCoords.y;
	}

	//-----
	// Build staging vertex buffer

	const bool vertexBufferMappable = settings.vertexBufferUsages & BufferUsage::Map;
	const auto vertexCount = vertices.size();

	VertexBuffer::Settings vertexBufferSettings;
	vertexBufferSettings.size = vertexCount;
	vertexBufferSettings.vertexFormat = settings.getVertexFormat();

	// If the buffer is mappable, the staging buffer will be the final buffer
	if (vertexBufferMappable)
		vertexBufferSettings.usages = settings.vertexBufferUsages;
	// If not, create a staging buffer
	else
		vertexBufferSettings.usages = BufferUsage::TransferSrc | BufferUsage::Map;

	auto stagingVertexBuffer = std::make_shared<VertexBuffer>(vertexBufferSettings);

	stagingVertexBuffer->map();

	MemoryMapper srcMemoryMapper(vertices.data(), sizeof(StaticVertex), offsetof(StaticVertex, position));
	auto dstMemoryMapper = stagingVertexBuffer->mapComponent(VertexComponentType::Position);
	copyComponent(format.getComponent(VertexComponentType::Position).format, srcMemoryMapper, dstMemoryMapper, vertexCount, 1.0);

	if (hasTexCoords)
	{
		srcMemoryMapper.setElementOffset(offsetof(StaticVertex, texCoords));
		dstMemoryMapper = stagingVertexBuffer->mapComponent(VertexComponentType::TexCoords);
		copyComponent(format.getComponent(VertexComponentType::TexCoords).format, srcMemoryMapper, dstMemoryMapper, vertexCount);
	}

	if (hasNormal)
	{
		srcMemoryMapper.setElementOffset(offsetof(StaticVertex, normal));
		dstMemoryMapper = stagingVertexBuffer->mapComponent(VertexComponentType::Normal);
		copyComponent(format.getComponent(VertexComponentType::Normal).format, srcMemoryMapper, dstMemoryMapper, vertexCount);
	}

	if (hasTangent)
	{
		srcMemoryMapper.setElementOffset(offsetof(StaticVertex, tangent));
		dstMemoryMapper = stagingVertexBuffer->mapComponent(VertexComponentType::Tangent);
		copyComponent(format.getComponent(VertexComponentType::Tangent).format, srcMemoryMapper, dstMemoryMapper, vertexCount);
	}

	if (hasBitangent)
	{
		srcMemoryMapper.setElementOffset(offsetof(StaticVertex, bitangent));
		dstMemoryMapper = stagingVertexBuffer->mapComponent(VertexComponentType::Bitangent);
		copyComponent(format.getComponent(VertexComponentType::Bitangent).format, srcMemoryMapper, dstMemoryMapper, vertexCount);
	}

	stagingVertexBuffer->unmap();

	//-----
	// Build staging index buffer

	const bool indexBufferMappable = settings.indexBufferUsages & BufferUsage::Map;

	IndexBuffer::Settings indexBufferSettings;
	indexBufferSettings.size = indices.size();

	// Index type defaulted to the smaller integer size
	indexBufferSettings.indexType = IndexType::U16;

	// If the user specified an index type, take this one
	if (settings.indexType.has_value())
		indexBufferSettings.indexType = settings.indexType.value();
	// The user didn't specify any type, ensure U16 is enough to index vertices, if not, take U32
	else if (vertices.size() >= std::numeric_limits<uint16_t>::max())
		indexBufferSettings.indexType = IndexType::U32;

	// If the buffer is mappable, the staging buffer will be the final buffer
	if (indexBufferMappable)
		indexBufferSettings.usages = settings.indexBufferUsages;
	// If not, create a staging buffer
	else
		indexBufferSettings.usages = BufferUsage::TransferSrc | BufferUsage::Map;

	auto stagingIndexBuffer = std::make_shared<IndexBuffer>(indexBufferSettings);

	auto bufferData = stagingIndexBuffer->map();

	if (indexBufferSettings.indexType == IndexType::U32)
	{
		memcpy(bufferData, indices.data(), indices.size() * sizeof(uint32_t));
	}
	else
	{
		MemoryMapper mapper(bufferData, sizeof(uint16_t));

		for (size_t i = 0; i < indices.size(); i++)
			mapper.map<uint16_t>(i) = static_cast<uint16_t>(indices[i]);
	}

	stagingIndexBuffer->unmap();

	//----- Build vertex & index buffers

	Ptr<VertexBuffer> vertexBuffer;
	Ptr<IndexBuffer> indexBuffer;

	if (vertexBufferMappable && indexBufferMappable)
	{
		std::swap(vertexBuffer, stagingVertexBuffer);
		std::swap(indexBuffer, stagingIndexBuffer);
	}
	else
	{
		Ptr<CommandBuffer> commandBuffer = settings.commandBuffer;

		// Command buffer not provided by the user : we create one
		if (!settings.commandBuffer)
		{
			auto commandPool = Renderer::instance().getCommandPool(QueueType::Graphics);

			commandBuffer = commandPool->createBuffer({ true });

			commandBuffer->begin();
		}
		// The command buffer is provided, so the user will be responsible to delete staging buffers once the command is executed
		else
		{
			ATEMA_ASSERT(settings.stagingBuffers, "Invalid staging buffer list");

			if (!vertexBufferMappable)
				settings.stagingBuffers->emplace_back(stagingVertexBuffer->getBuffer());

			if (!indexBufferMappable)
				settings.stagingBuffers->emplace_back(stagingIndexBuffer->getBuffer());
		}

		// Vertex buffer
		if (vertexBufferMappable)
		{
			std::swap(vertexBuffer, stagingVertexBuffer);
		}
		else
		{
			// This flag is required for transfer dst
			vertexBufferSettings.usages = settings.vertexBufferUsages | BufferUsage::TransferDst;

			vertexBuffer = std::make_shared<VertexBuffer>(vertexBufferSettings);

			commandBuffer->copyBuffer(stagingVertexBuffer->getBuffer(), vertexBuffer->getBuffer(), vertexBuffer->getByteSize());
		}

		// Index buffer
		if (indexBufferMappable)
		{
			std::swap(indexBuffer, stagingIndexBuffer);
		}
		else
		{
			// This flag is required for transfer dst
			indexBufferSettings.usages = settings.indexBufferUsages | BufferUsage::TransferDst;

			indexBuffer = std::make_shared<IndexBuffer>(indexBufferSettings);

			commandBuffer->copyBuffer(stagingIndexBuffer->getBuffer(), indexBuffer->getBuffer(), indexBuffer->getByteSize());
		}

		// We created our own command buffer, execute it right now, so we can delete staging buffers
		if (!settings.commandBuffer)
		{
			commandBuffer->end();

			Renderer::instance().submitAndWait({ commandBuffer });
		}
	}

	//-----
	// AABB

	AABBf aabb;

	for (const auto& vertex : vertices)
		aabb.extend(vertex.position);

	//-----
	// Build mesh

	auto mesh = std::make_shared<Mesh>();

	mesh->setVertexBuffer(vertexBuffer);
	mesh->setIndexBuffer(indexBuffer);
	mesh->setAABB(aabb);

	return mesh;
}
