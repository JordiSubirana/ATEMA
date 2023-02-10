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
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/Mesh.hpp>
#include <Atema/Graphics/Model.hpp>
#include <Atema/Graphics/SurfaceMaterial.hpp>
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

	tinyobj::ObjReader objReader;

	objReader.ParseFromFile(path.string());

	if (!objReader.Valid())
		ATEMA_ERROR(objReader.Warning() + objReader.Error());

	const auto& attrib = objReader.GetAttrib();
	const auto& shapes = objReader.GetShapes();
	const auto& materials = objReader.GetMaterials();

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

	// Load materials
	const auto textureDir = settings.textureDir.empty() ? path.parent_path() : settings.textureDir;

	for (const auto& mat : materials)
	{
		auto materialData = std::make_shared<SurfaceMaterialData>();

		materialData->color = { mat.diffuse[0], mat.diffuse[1], mat.diffuse[2], 1.0f };
		materialData->emissive = { mat.emission[0], mat.emission[1], mat.emission[2] };
		materialData->metalness = { mat.metallic };
		materialData->roughness = { mat.roughness };

		if (!mat.diffuse_texname.empty())
			materialData->colorMap = Graphics::instance().getImage(textureDir / mat.diffuse_texname);

		if (!mat.normal_texname.empty())
			materialData->normalMap = Graphics::instance().getImage(textureDir / mat.normal_texname);

		if (!mat.displacement_texname.empty())
			materialData->heightMap = Graphics::instance().getImage(textureDir / mat.displacement_texname);

		if (!mat.emissive_texname.empty())
			materialData->emissiveMap = Graphics::instance().getImage(textureDir / mat.emissive_texname);

		if (!mat.metallic_texname.empty())
			materialData->metalnessMap = Graphics::instance().getImage(textureDir / mat.metallic_texname);

		if (!mat.roughness_texname.empty())
			materialData->roughnessMap = Graphics::instance().getImage(textureDir / mat.roughness_texname);

		if (!mat.alpha_texname.empty())
			materialData->alphaMaskMap = Graphics::instance().getImage(textureDir / mat.alpha_texname);

		model->addMaterial(materialData);
	}

	// Load meshes
	const bool hasNormal = format.hasComponent(VertexComponentType::Normal);
	const bool hasTangent = format.hasComponent(VertexComponentType::Tangent);
	const bool hasBitangent = format.hasComponent(VertexComponentType::Bitangent);

	for (const auto& shape : shapes)
	{
		size_t currentVertexCount = 0;
		int currentMaterialID = shape.mesh.material_ids[0];

		std::vector<size_t> meshVertexCounts;
		std::vector<size_t> meshMaterialIDs;

		for (size_t i = 0; i < shape.mesh.material_ids.size(); i++)
		{
			const auto materialID = shape.mesh.material_ids[i];

			if (materialID != currentMaterialID)
			{
				meshVertexCounts.emplace_back(currentVertexCount);
				meshMaterialIDs.emplace_back(currentMaterialID);

				currentMaterialID = materialID;
				currentVertexCount = 0;
			}

			currentVertexCount += 3;
		}

		meshVertexCounts.emplace_back(currentVertexCount);
		meshMaterialIDs.emplace_back(shape.mesh.material_ids.back());

		size_t firstIndex = 0;
		for (size_t m = 0; m < meshVertexCounts.size(); m++)
		{
			const size_t vertexCount = meshVertexCounts[m];
			const auto materialID = meshMaterialIDs[m];

			std::vector<ModelLoader::StaticVertex> vertices;
			vertices.reserve(vertexCount);

			// Generate identity index data
			std::vector<uint32_t> indices(vertexCount);
			for (uint32_t i = 0; i < vertexCount; i++)
				indices[i] = i;

			// Generate vertex data
			const auto lastIndex = firstIndex + vertexCount;
			for (size_t i = firstIndex; i < lastIndex; i++)
			{
				const auto& index = shape.mesh.indices[i];
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
					{
						vertex.normal = transform.transformVector(vertex.normal);
						vertex.normal.normalize();
					}
				}
			}
			
			// Generate tangeants & bitangeants if needed
			if (hasTangent || hasBitangent)
				ModelLoader::generateTangents(vertices, indices);
			// Otherwise, just ensure there are no duplicate vertices
			else
				ModelLoader::removeDuplicates(vertices, indices);
			
			auto mesh = ModelLoader::loadMesh(vertices, indices, newSettings);
			mesh->setMaterialID(materialID);

			model->addMesh(std::move(mesh));

			firstIndex += vertexCount;
		}
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
