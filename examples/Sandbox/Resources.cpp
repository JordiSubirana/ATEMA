/*
	Copyright 2021 Jordi SUBIRANA

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
	auto commandPool = Renderer::instance().getDefaultCommandPool();

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

	std::unordered_map<BasicVertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			BasicVertex vertex;

			vertex.pos =
			{
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			//vertex.texCoord =
			//{
			//	attrib.texcoords[2 * index.texcoord_index + 0],
			//	attrib.texcoords[2 * index.texcoord_index + 1]
			//};

			vertex.texCoord =
			{
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1] // Flip vertical component of texcoord
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			// This method may duplicate vertices and we don't want that
			//m_modelVertices.push_back(vertex);
			//m_modelIndices.push_back(m_modelIndices.size());

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(modelVertices.size());
				modelVertices.push_back(vertex);
			}

			modelIndices.push_back(uniqueVertices[vertex]);
		}
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

		auto fence = Fence::create({});

		Renderer::instance().submit(
			{ commandBuffer },
			{},
			{},
			{},
			fence);

		// Wait for the command to be done
		fence->wait();
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

		auto fence = Fence::create({});

		Renderer::instance().submit(
			{ commandBuffer },
			{},
			{},
			{},
			fence);

		// Wait for the command to be done
		fence->wait();
	}

	indexCount = modelIndices.size();
}

MaterialData::MaterialData(const std::filesystem::path& path)
{
	auto commandPool = Renderer::instance().getDefaultCommandPool();

	// Load the texture data
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(path.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

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
	imageSettings.format = ImageFormat::RGBA8_SRGB;
	imageSettings.width = texWidth;
	imageSettings.height = texHeight;
	imageSettings.mipLevels = textureMipLevels;
	imageSettings.usages = ImageUsage::ShaderInput | ImageUsage::TransferDst | ImageUsage::TransferSrc;

	texture = Image::create(imageSettings);

	// Copy staging buffer to index buffer
	auto commandBuffer = commandPool->createBuffer({ true });

	commandBuffer->begin();

	commandBuffer->setImageLayout(texture, ImageLayout::TransferDst);

	commandBuffer->copyBuffer(stagingBuffer, texture);

	commandBuffer->createMipmaps(texture);

	commandBuffer->setImageLayout(texture, ImageLayout::ShaderInput);

	commandBuffer->end();

	auto fence = Fence::create({});

	Renderer::instance().submit(
		{ commandBuffer },
		{},
		{},
		{},
		fence);

	// Wait for the command to be done
	fence->wait();

	// Create sampler
	Sampler::Settings samplerSettings(SamplerFilter::Linear, true);

	sampler = Sampler::create(samplerSettings);
}
