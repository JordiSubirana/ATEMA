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

#include <Atema/Renderer/BufferLayout.hpp>
#include <Atema/Graphics/Loaders/ObjLoader.hpp>
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/VertexFormat.hpp>
#include <Atema/Graphics/SurfaceMaterial.hpp>
#include <Atema/Graphics/Loaders/DefaultImageLoader.hpp>

using namespace at;

namespace
{
	const auto materialShaderPath = shaderPath / "GBufferOptions.atsl";

	struct MaterialParam
	{
		static constexpr size_t InvalidOffset = std::numeric_limits<size_t>::max();

		std::string name;
		Ptr<Image>* imagePtr;
		ConstantValue defaultValue;
		bool textureOnly = false;
		size_t offset = InvalidOffset;
	};

	BufferElementType getElementType(const ConstantValue& constantValue)
	{
		if (constantValue.is<bool>())
			return BufferElementType::Bool;
		else if (constantValue.is<int32_t>())
			return BufferElementType::Int;
		else if (constantValue.is<uint32_t>())
			return BufferElementType::UInt;
		else if (constantValue.is<float>())
			return BufferElementType::Float;
		else if (constantValue.is<Vector2i>())
			return BufferElementType::Int2;
		else if (constantValue.is<Vector2u>())
			return BufferElementType::UInt2;
		else if (constantValue.is<Vector2f>())
			return BufferElementType::Float2;
		else if (constantValue.is<Vector3i>())
			return BufferElementType::Int3;
		else if (constantValue.is<Vector3u>())
			return BufferElementType::UInt3;
		else if (constantValue.is<Vector3f>())
			return BufferElementType::Float3;
		else if (constantValue.is<Vector4i>())
			return BufferElementType::Int4;
		else if (constantValue.is<Vector4u>())
			return BufferElementType::UInt4;
		else if (constantValue.is<Vector4f>())
			return BufferElementType::Float4;

		ATEMA_ERROR("Invalid type");

		return BufferElementType::Int;
	}

	void writeBufferElement(void* ptr, size_t byteOffset, const ConstantValue& constantValue)
	{
		if (constantValue.is<bool>())
			mapMemory<bool>(ptr, byteOffset) = constantValue.get<bool>();
		else if (constantValue.is<int32_t>())
			mapMemory<int32_t>(ptr, byteOffset) = constantValue.get<int32_t>();
		else if (constantValue.is<uint32_t>())
			mapMemory<uint32_t>(ptr, byteOffset) = constantValue.get<uint32_t>();
		else if (constantValue.is<float>())
			mapMemory<float>(ptr, byteOffset) = constantValue.get<float>();
		else if (constantValue.is<Vector2i>())
			mapMemory<Vector2i>(ptr, byteOffset) = constantValue.get<Vector2i>();
		else if (constantValue.is<Vector2u>())
			mapMemory<Vector2u>(ptr, byteOffset) = constantValue.get<Vector2u>();
		else if (constantValue.is<Vector2f>())
			mapMemory<Vector2f>(ptr, byteOffset) = constantValue.get<Vector2f>();
		else if (constantValue.is<Vector3i>())
			mapMemory<Vector3i>(ptr, byteOffset) = constantValue.get<Vector3i>();
		else if (constantValue.is<Vector3u>())
			mapMemory<Vector3u>(ptr, byteOffset) = constantValue.get<Vector3u>();
		else if (constantValue.is<Vector3f>())
			mapMemory<Vector3f>(ptr, byteOffset) = constantValue.get<Vector3f>();
		else if (constantValue.is<Vector4i>())
			mapMemory<Vector4i>(ptr, byteOffset) = constantValue.get<Vector4i>();
		else if (constantValue.is<Vector4u>())
			mapMemory<Vector4u>(ptr, byteOffset) = constantValue.get<Vector4u>();
		else if (constantValue.is<Vector4f>())
			mapMemory<Vector4f>(ptr, byteOffset) = constantValue.get<Vector4f>();
		else
			ATEMA_ERROR("Invalid type");
	}
}

ModelData::ModelData(const std::filesystem::path& path)
{
	const auto vertexFormat = VertexFormat::create(DefaultVertexFormat::XYZ_UV_NTB);

	ModelLoader::Settings settings(vertexFormat);
	settings.flipTexCoords = true;
	settings.vertexTransformation = modelTransform.getMatrix();
	settings.textureDir = rscPath / "Textures";

	model = ObjLoader::load(path, settings);
}

at::Ptr<at::SurfaceMaterialData> loadMaterialData(const std::filesystem::path& path, const std::string& extension)
{
	/*struct MaterialData
	{
		[optional(MaterialColorBinding < 0)]
		vec4f color;

		[optional(MaterialNormalBinding < 0)]
		vec3f normal;

		[optional(MaterialAOBinding < 0)]
		float ao;

		[optional(MaterialHeightBinding < 0)]
		float height;

		[optional(MaterialEmissiveBinding < 0)]
		vec3f emissive;

		[optional(MaterialMetalnessBinding < 0)]
		float metalness;

		[optional(MaterialRoughnessBinding < 0)]
		float roughness;
	}//*/

	auto materialData = std::make_shared<SurfaceMaterialData>();

	std::vector<MaterialParam> materialParameters =
	{
		{ "Color", &materialData->colorMap, ConstantValue(Vector4f(1.0f, 1.0f, 1.0f, 1.0f)) },
		{ "Normal", &materialData->normalMap, ConstantValue(Vector3f(0.5f, 0.5f, 1.0f)) },
		{ "AO", &materialData->ambientOcclusionMap, ConstantValue(1.0f)},
		{ "Height", &materialData->heightMap, ConstantValue(1.0f) },
		{ "Emissive", &materialData->emissiveMap, ConstantValue(Vector3f(0.0f, 0.0f, 0.0f)) },
		{ "Metalness", &materialData->metalnessMap, ConstantValue(0.0f) },
		{ "Roughness", &materialData->roughnessMap, ConstantValue(0.0f) },
		{ "AlphaMask", &materialData->alphaMaskMap, ConstantValue(1.0f), true },
	};

	for (auto& materialParameter : materialParameters)
	{
		const auto& name = materialParameter.name;
		auto& texture = *materialParameter.imagePtr;

		const auto fullPath = path.string() + "_" + name + "." + extension;

		if (std::filesystem::exists(fullPath))
			texture = DefaultImageLoader::load(fullPath, {});
	}

	return materialData;
}
