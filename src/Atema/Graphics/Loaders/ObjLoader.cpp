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

#include <Atema/Graphics/Loaders/ObjLoader.hpp>
#include <Atema/Graphics/Model.hpp>
#include <Atema/Graphics/Loaders/ModelLoader.hpp>
#include <Atema/Graphics/VertexFormat.hpp>
#include <Atema/Renderer/Renderer.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

#include <unordered_set>

using namespace at;

namespace
{
	std::unordered_set<std::string_view> extensions =
	{
		".obj"
	};
}

Ptr<Model> ObjLoader::load(const std::filesystem::path& path, const ModelLoader::Settings& settings)
{
	if (!isExtensionSupported(path.extension()))
		return nullptr;

	const auto& format = *settings.getVertexFormat();

	auto model = std::make_shared<Model>();

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str()))
		ATEMA_ERROR(warn + err);

	// Settings override
	ModelLoader::Settings newSettings = settings;
	
	// If there is a vertex transformation required, we can do it right now on the positions & normals
	// If we don't do this, ModelLoader will apply the transform to position/normal/tangent/bitangent
	// But since we are computing tangent/bitangent from position, we would just add a lot of computing for nothing
	if (newSettings.vertexTransformation.has_value())
		newSettings.vertexTransformation.reset();

	// If the user didn't provide a command buffer, we will create one
	// This way we can use it to load all the meshes at once
	std::list<Ptr<Buffer>> stagingBuffers;
	if (!settings.commandBuffer)
	{
		auto commandPool = Renderer::instance().getCommandPool(QueueType::Graphics);

		newSettings.commandBuffer = commandPool->createBuffer({ true });

		newSettings.commandBuffer->begin();

		newSettings.stagingBuffers = &stagingBuffers;
	}

	const bool hasNormal = format.hasComponent(VertexComponentType::Normal);
	const bool hasTangent = format.hasComponent(VertexComponentType::Tangent);
	const bool hasBitangent = format.hasComponent(VertexComponentType::Bitangent);

	for (const auto& shape : shapes)
	{
		const size_t vertexCount = shape.mesh.indices.size();

		std::vector<ModelLoader::StaticVertex> vertices;
		vertices.reserve(vertexCount);

		// Generate identity index data
		std::vector<uint32_t> indices(vertexCount);
		for (uint32_t i = 0; i < vertexCount; i++)
			indices[i] = i;

		// Generate vertex data
		for (const auto& index : shape.mesh.indices)
		{
			auto& vertex = vertices.emplace_back();

			// Get vertex position
			vertex.position =
			{
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			// Get vertex texcoords if it exists
			if (index.texcoord_index >= 0)
			{
				vertex.texCoords =
				{
					attrib.texcoords[2 * index.texcoord_index + 0],
					attrib.texcoords[2 * index.texcoord_index + 1]
				};
			}

			// Get vertex normal if it exists
			if (index.normal_index >= 0)
			{
				vertex.normal =
				{
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};
			}
		}

		// Transform vertices if needed
		// We will generate tangents & bitangents from position
		// So there is no need to make the ModelLoader to transform everything later
		// Just transform the basic data so everything will be correctly computed
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
		}

		// Generate tangeants & bitangeants if needed
		if (hasTangent || hasBitangent)
			ModelLoader::generateTangents(vertices, indices);
		// Otherwise, just ensure there are no duplicate vertices
		else
			ModelLoader::removeDuplicates(vertices, indices);

		model->addMesh(ModelLoader::loadMesh(vertices, indices, newSettings));
	}

	if (!settings.commandBuffer)
	{
		newSettings.commandBuffer->end();

		Renderer::instance().submitAndWait({ newSettings.commandBuffer });
	}

	return model;
}

bool ObjLoader::isExtensionSupported(const std::filesystem::path& extension)
{
	return extensions.count(extension.string()) > 0;
}
