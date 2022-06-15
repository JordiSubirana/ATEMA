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

#include "Resources.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

using namespace at;

ModelData::ModelData(const std::filesystem::path& path)
{
	auto commandPool = Renderer::instance().getCommandPool(QueueType::Graphics);

	std::vector<BasicVertex> modelVertices;
	std::vector<uint32_t> modelIndices;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str()))
	{
		ATEMA_ERROR(warn + err);
	}

	Matrix4f rotation = Matrix4f::createRotation({Math::HalfPi<float>, 0.0f, 0.0f});

	std::unordered_map<BasicVertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			BasicVertex vertex;

			vertex.position =
			{
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord =
			{
				attrib.texcoords[2 * index.texcoord_index + 0],
				attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.normal =
			{
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};

			Vector4f tmp(vertex.position.x, vertex.position.y, vertex.position.z, 1.0f);
			tmp = rotation * tmp;
			vertex.position = { tmp.x, tmp.y, tmp.z };

			tmp = { vertex.normal.x, vertex.normal.y, vertex.normal.z, 0.0f };
			tmp = rotation * tmp;
			vertex.normal = { tmp.x, tmp.y, tmp.z };
			vertex.normal.normalize();
			
			modelIndices.push_back(static_cast<uint32_t>(modelVertices.size()));
			modelVertices.push_back(vertex);
		}
	}

	// Compute tangents / bitangents
	{
		const auto triangleCount = modelIndices.size() / 3;

		for (size_t triangleIndex = 0; triangleIndex < triangleCount; triangleIndex++)
		{
			const auto firstVertexIndex = triangleIndex * 3;

			auto& v1 = modelVertices[modelIndices[firstVertexIndex]];
			auto& v2 = modelVertices[modelIndices[firstVertexIndex + 1]];
			auto& v3 = modelVertices[modelIndices[firstVertexIndex + 2]];

			auto edge1 = v2.position - v1.position;
			auto edge2 = v3.position - v1.position;

			auto deltaUV1 = v2.texCoord - v1.texCoord;
			auto deltaUV2 = v3.texCoord - v1.texCoord;

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

			float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

			tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

			bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
			bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
			bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

			tangent = (edge1 * deltaUV2.y - edge2 * deltaUV1.y) * f;
			bitangent = (edge2 * deltaUV1.x - edge1 * deltaUV2.x) * f;

			tangent.normalize();
			bitangent.normalize();

			v1.tangent = tangent;
			v1.bitangent = bitangent;

			v2.tangent = tangent;
			v2.bitangent = bitangent;

			v3.tangent = tangent;
			v3.bitangent = bitangent;

			// Flip vertical component of texcoord
			v1.texCoord.y = 1.0f - v1.texCoord.y;
			v2.texCoord.y = 1.0f - v2.texCoord.y;
			v3.texCoord.y = 1.0f - v3.texCoord.y;
		}
	}

	// Build final vertex / index buffers
	{
		std::vector<BasicVertex> vertices;
		std::vector<uint32_t> indices;

		for (auto& modelVertex : modelVertices)
		{
			bool found = false;
			uint32_t vertexIndex = 0;
			for (auto& vertex : vertices)
			{
				if (vertex == modelVertex)
				{
					indices.emplace_back(vertexIndex);
					found = true;
					break;
				}
				vertexIndex++;
			}

			if (!found)
			{
				indices.emplace_back(static_cast<uint32_t>(vertices.size()));
				vertices.emplace_back(modelVertex);
			}
		}

		std::swap(vertices, modelVertices);
		std::swap(indices, modelIndices);
	}

	// Create vertex buffer
	{
		// Fill staging buffer
		size_t bufferSize = sizeof(modelVertices[0]) * modelVertices.size();

		auto stagingBuffer = Buffer::create({ BufferUsage::Transfer, bufferSize, true });

		auto bufferData = stagingBuffer->map();

		memcpy(bufferData, static_cast<void*>(modelVertices.data()), static_cast<size_t>(bufferSize));

		stagingBuffer->unmap();

		// Create vertex buffer
		vertexBuffer = Buffer::create({ BufferUsage::Vertex, bufferSize });

		// Copy staging buffer to vertex buffer
		auto commandBuffer = commandPool->createBuffer({ true });

		commandBuffer->begin();

		commandBuffer->copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		commandBuffer->end();

		Renderer::instance().submitAndWait({ commandBuffer });
	}

	// Create index buffer
	{
		// Fill staging buffer
		size_t bufferSize = sizeof(modelIndices[0]) * modelIndices.size();

		auto stagingBuffer = Buffer::create({ BufferUsage::Transfer, bufferSize, true });

		auto bufferData = stagingBuffer->map();

		memcpy(bufferData, static_cast<void*>(modelIndices.data()), static_cast<size_t>(bufferSize));

		stagingBuffer->unmap();

		// Create vertex buffer
		indexBuffer = Buffer::create({ BufferUsage::Index, bufferSize });

		// Copy staging buffer to index buffer
		auto commandBuffer = commandPool->createBuffer({ true });

		commandBuffer->begin();

		commandBuffer->copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		commandBuffer->end();

		Renderer::instance().submitAndWait({ commandBuffer });
	}

	indexCount = static_cast<uint32_t>(modelIndices.size());
}

MaterialData::MaterialData(const std::filesystem::path& path, const std::string& extension)
{
	const std::vector<std::pair<std::string, Ptr<Image>*>> texturePairs =
	{
		{ "color", &color },
		{ "normal", &normal },
		{ "ao", &ambientOcclusion },
		{ "emissive", &emissive },
		{ "metalness", &metalness },
		{ "roughness", &roughness }
	};

	auto commandPool = Renderer::instance().getCommandPool(QueueType::Graphics);
	
	for (auto& pair : texturePairs)
	{
		const auto& suffix = pair.first;
		auto& texture = *pair.second;

		const auto fullPath = path.string() + "_" + suffix + "." + extension;

		// Load the texture data
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(fullPath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		size_t imageSize = texWidth * texHeight * 4;

		// std::max for the largest dimension, std::log2 to find how many times we can divide by 2
		// std::floor handles the case when the dimension is not power of 2, +1 to add a mip level to the original image
		auto textureMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

		if (!pixels)
			ATEMA_ERROR("Failed to load texture image");

		// Fill staging buffer
		auto stagingBuffer = Buffer::create({ BufferUsage::Transfer, imageSize, true });

		auto bufferData = stagingBuffer->map();

		memcpy(bufferData, pixels, imageSize);

		stagingBuffer->unmap();

		// Free image data
		stbi_image_free(pixels);

		// Create image
		Image::Settings imageSettings;
		//imageSettings.format = ImageFormat::RGBA8_SRGB;
		imageSettings.format = ImageFormat::RGBA8_UNORM;
		imageSettings.width = texWidth;
		imageSettings.height = texHeight;
		imageSettings.mipLevels = textureMipLevels;
		imageSettings.usages = ImageUsage::ShaderSampling | ImageUsage::TransferDst | ImageUsage::TransferSrc;

		texture = Image::create(imageSettings);

		// Copy staging buffer to index buffer
		auto commandBuffer = commandPool->createBuffer({ true });

		commandBuffer->begin();

		commandBuffer->imageBarrier(texture, ImageBarrier::InitializeTransferDst);

		commandBuffer->copyBuffer(stagingBuffer, texture, ImageLayout::TransferDst);

		commandBuffer->createMipmaps(texture, PipelineStage::FragmentShader, MemoryAccess::ShaderRead, ImageLayout::ShaderRead);

		commandBuffer->end();

		Renderer::instance().submitAndWait({ commandBuffer });
	}

	// Create sampler
	Sampler::Settings samplerSettings(SamplerFilter::Linear, true);

	sampler = Sampler::create(samplerSettings);
}
