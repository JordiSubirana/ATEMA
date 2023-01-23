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

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

using namespace at;

namespace
{
	const auto materialShaderPath = shaderPath / "GBufferOptions.atsl";

	struct MaterialParameter
	{
		static constexpr size_t InvalidOffset = std::numeric_limits<size_t>::max();

		std::string name;
		Ptr<Image>* imagePtr;
		ConstantValue defaultValue;
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
	const auto vertexFormat = VertexFormat::create(DefaultVertexFormat::XYZ_UV_Normal_Tangent_Bitangent);

	ModelLoader::Settings settings(vertexFormat);
	settings.flipTexCoords = true;
	settings.vertexTransformation = modelTransform.getMatrix();

	model = ObjLoader::load(path, settings);
}

MaterialData::MaterialData(const std::filesystem::path& path, const std::string& extension)
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

	std::vector<MaterialParameter> materialParameters =
	{
		{ "Color", &color, ConstantValue(Vector4f(1.0f, 1.0f, 1.0f, 1.0f)) },
		{ "Normal", &normal, ConstantValue(Vector3f(0.5f, 0.5f, 1.0f)) },
		{ "AO", &ambientOcclusion, ConstantValue(1.0f)},
		{ "Height", &height, ConstantValue(1.0f) },
		{ "Emissive", &emissive, ConstantValue(Vector3f(0.0f, 0.0f, 0.0f)) },
		{ "Metalness", &metalness, ConstantValue(0.0f) },
		{ "Roughness", &roughness, ConstantValue(0.0f) }
	};

	BufferLayout bufferLayout(StructLayout::Default);
	std::vector<size_t> bufferOffsets;

	auto commandPool = Renderer::instance().getCommandPool(QueueType::Graphics);

	size_t textureCount = 0;
	for (auto& materialParameter : materialParameters)
	{
		const auto& name = materialParameter.name;
		auto& texture = *materialParameter.imagePtr;

		const auto fullPath = path.string() + "_" + name + "." + extension;

		// Load the texture data
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(fullPath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		// The texture was successfully loaded : create a corresponding GPU texture
		if (pixels)
		{
			textureCount++;

			size_t imageSize = texWidth * texHeight * 4;

			// std::max for the largest dimension, std::log2 to find how many times we can divide by 2
			// std::floor handles the case when the dimension is not power of 2, +1 to add a mip level to the original image
			auto textureMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

			// Fill staging buffer
			auto stagingBuffer = Buffer::create({ BufferUsage::TransferSrc | BufferUsage::Map, imageSize });

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
		// The texture does not exist : use the default value instead
		else
		{
			const auto elementOffset = bufferLayout.add(getElementType(materialParameter.defaultValue));

			bufferOffsets.emplace_back(elementOffset);
		}
	}

	// Create sampler
	Sampler::Settings samplerSettings(SamplerFilter::Linear, true);

	sampler = Sampler::create(samplerSettings);

	// Create bindings
	DescriptorSetLayout::Settings descriptorSetLayoutSettings;
	descriptorSetLayoutSettings.pageSize = 1;

	uint32_t bindingIndex = 0;

	// Check if we have some elements in the buffer
	if (bufferLayout.getSize() > 0)
	{
		bindingIndex++;
		
		descriptorSetLayoutSettings.bindings.emplace_back(DescriptorType::UniformBuffer, 0, 1, ShaderStage::Fragment);

		Buffer::Settings bufferSettings;
		bufferSettings.usages = BufferUsage::Uniform | BufferUsage::Map;
		bufferSettings.byteSize = bufferLayout.getSize();

		uniformBuffer = Buffer::create(bufferSettings);
	}

	for (size_t i = 0; i < textureCount; i++)
		descriptorSetLayoutSettings.bindings.emplace_back(DescriptorType::CombinedImageSampler, bindingIndex++, 1, ShaderStage::Fragment);

	descriptorSetLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);

	// Create descriptor set
	descriptorSet = descriptorSetLayout->createSet();

	bindingIndex = 0;

	void* bufferData = nullptr;

	if (uniformBuffer)
	{
		descriptorSet->update(bindingIndex++, uniformBuffer);

		bufferData = uniformBuffer->map();
	}

	// Initialize AST
	std::vector<UberShader::Option> shaderOptions;

	size_t bufferElementIndex = 0;

	for (auto& materialParameter : materialParameters)
	{
		auto& texture = *materialParameter.imagePtr;

		if (texture)
		{
			shaderOptions.emplace_back("Material" + materialParameter.name + "Binding", bindingIndex);

			descriptorSet->update(bindingIndex++, texture->getView(), sampler);
		}
		else
		{
			writeBufferElement(bufferData, bufferOffsets[bufferElementIndex++], materialParameter.defaultValue);
		}
	}

	if (uniformBuffer)
		uniformBuffer->unmap();

	auto& graphics = Graphics::instance();

	vertexShader = graphics.getShader(*graphics.getUberShader(materialShaderPath, AstShaderStage::Vertex, shaderOptions));
	fragmentShader = graphics.getShader(*graphics.getUberShader(materialShaderPath, AstShaderStage::Fragment, shaderOptions));
}
