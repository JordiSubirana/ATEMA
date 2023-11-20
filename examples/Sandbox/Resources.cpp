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

#include <Atema/Graphics/Loaders/ObjLoader.hpp>
#include <Atema/Graphics/MaterialData.hpp>
#include <Atema/Graphics/VertexFormat.hpp>

using namespace at;

namespace
{
	struct MaterialTextureParameter
	{
		const char* name;
		std::string imageSuffix;
	};
}

std::filesystem::path ResourcePath::resources() noexcept
{
	return "../../examples/Sandbox/Resources";
}

std::filesystem::path ResourcePath::fonts() noexcept
{
	return resources() / "Fonts";
}

std::filesystem::path ResourcePath::models() noexcept
{
	return resources() / "Models";
}

std::filesystem::path ResourcePath::shaders() noexcept
{
	return resources() / "Shaders";
}

std::filesystem::path ResourcePath::textures() noexcept
{
	return resources() / "Textures";
}

at::Ptr<at::Model> ResourceLoader::loadModel(const ModelSettings& modelSettings)
{
	const auto vertexFormat = VertexFormat::create(DefaultVertexFormat::XYZ_UV_NTB);

	ModelLoader::Settings loaderSettings(vertexFormat);
	loaderSettings.flipTexCoords = true;
	loaderSettings.vertexTransformation = modelSettings.vertexTransformation;
	loaderSettings.textureDir = ResourcePath::textures();

	auto model = ObjLoader::load(modelSettings.modelPath, loaderSettings);

	if (modelSettings.overrideMaterial)
	{
		for (auto& mesh : model->getMeshes())
			mesh->setMaterialID(0);

		*model->getMaterialData()[0] = *loadMaterialData(modelSettings.modelTexturePath, modelSettings.modelTextureExtension);
	}

	for (auto& materialData : model->getMaterialData())
	{
		//model->addMaterialInstance(DefaultMaterials::getPhongInstance(*materialData));
		model->addMaterialInstance(DefaultMaterials::getPBRInstance(*materialData));
	}

	return model;
}

at::Ptr<at::MaterialData> ResourceLoader::loadMaterialData(const std::filesystem::path& path, const std::string& extension)
{
	static const std::vector<MaterialTextureParameter> materialParameters =
	{
		{ MaterialData::BaseColorMap, "Color" },
		{ MaterialData::NormalMap, "Normal" },
		{ MaterialData::AmbientOcclusionMap, "AO" },
		{ MaterialData::HeightMap, "Height" },
		{ MaterialData::EmissiveColorMap, "Emissive" },
		{ MaterialData::MetalnessMap, "Metalness" },
		{ MaterialData::RoughnessMap, "Roughness" },
		{ MaterialData::AlphaMaskMap, "AlphaMask" }
	};

	auto materialData = std::make_shared<at::MaterialData>();

	for (auto& materialParameter : materialParameters)
	{
		const auto fullPath = path.string() + "_" + materialParameter.imageSuffix + "." + extension;

		if (std::filesystem::exists(fullPath))
			materialData->set(materialParameter.name, MaterialData::Texture(fullPath));
	}

	return materialData;
}
