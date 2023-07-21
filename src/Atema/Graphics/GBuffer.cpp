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

#include <Atema/Graphics/GBuffer.hpp>
#include <Atema/Renderer/Utils.hpp>
#include <Atema/Renderer/Renderer.hpp>
#include <Atema/Shader/Atsl/AtslParser.hpp>
#include <Atema/Shader/Atsl/AtslToAstConverter.hpp>

#include <set>

using namespace at;

namespace
{
	static constexpr size_t ComponentCount = 4;

	std::string componentToString(ImageComponentType componentType)
	{
#define ATEMA_IMAGE_COMPONENT_TO_STRING(atComponent) case ImageComponentType::atComponent : return #atComponent

		switch (componentType)
		{
			ATEMA_IMAGE_COMPONENT_TO_STRING(UNORM8);
			ATEMA_IMAGE_COMPONENT_TO_STRING(SNORM8);
			ATEMA_IMAGE_COMPONENT_TO_STRING(USCALED8);
			ATEMA_IMAGE_COMPONENT_TO_STRING(SSCALED8);
			ATEMA_IMAGE_COMPONENT_TO_STRING(UINT8);
			ATEMA_IMAGE_COMPONENT_TO_STRING(SINT8);
			ATEMA_IMAGE_COMPONENT_TO_STRING(SRGB8);
			ATEMA_IMAGE_COMPONENT_TO_STRING(UNORM16);
			ATEMA_IMAGE_COMPONENT_TO_STRING(SNORM16);
			ATEMA_IMAGE_COMPONENT_TO_STRING(USCALED16);
			ATEMA_IMAGE_COMPONENT_TO_STRING(SSCALED16);
			ATEMA_IMAGE_COMPONENT_TO_STRING(UINT16);
			ATEMA_IMAGE_COMPONENT_TO_STRING(SINT16);
			ATEMA_IMAGE_COMPONENT_TO_STRING(SFLOAT16);
			ATEMA_IMAGE_COMPONENT_TO_STRING(UINT32);
			ATEMA_IMAGE_COMPONENT_TO_STRING(SINT32);
			ATEMA_IMAGE_COMPONENT_TO_STRING(SFLOAT32);
			ATEMA_IMAGE_COMPONENT_TO_STRING(UINT64);
			ATEMA_IMAGE_COMPONENT_TO_STRING(SINT64);
			ATEMA_IMAGE_COMPONENT_TO_STRING(SFLOAT64);
			default:
			{
				ATEMA_ERROR("Invalid component type");
			}
		}

#undef ATEMA_IMAGE_COMPONENT_TO_STRING

		return "";
	}

	struct GBufferTexture
	{
		GBufferTexture() :
			format(ImageFormat::RGBA32_SFLOAT),
			componentType(ImageComponentType::SFLOAT32),
			componentCount(0)
		{
			
		}

		GBufferTexture(const LightingModel::Parameter& parameter) :
			name(parameter.name),
			format(parameter.format),
			componentType(getComponentType(parameter.format)),
			componentCount(getComponentCount(parameter.format))
		{
		}

		bool dependsOn(const GBufferTexture& other) const
		{
			return dependencies.find(&other) != dependencies.end();
		}

		std::string name;
		ImageFormat format;
		ImageComponentType componentType;
		size_t componentCount;
		std::unordered_set<const GBufferTexture*> dependencies;
	};

	struct PhysicalTexture
	{
		PhysicalTexture() = delete;
		PhysicalTexture(ImageComponentType componentType) :
			componentType(componentType)
		{
			gbufferComponents.resize(ComponentCount);
		}

		bool canBeAdded(const GBufferTexture& texture, size_t& index) const
		{
			bool dependencyFound = false;
			index = 0;
			while (index <= ComponentCount - texture.componentCount)
			{
				size_t dependencyIndex = 0;
				dependencyFound = false;
				for (size_t offset = 0; offset < texture.componentCount; offset++)
				{
					for (const auto& existingTexture : components[index + offset])
					{
						if (texture.dependsOn(*existingTexture))
						{
							dependencyFound = true;
							dependencyIndex = index + offset;
							break;
						}
					}

					if (dependencyFound)
						break;
				}

				if (dependencyFound)
					index = dependencyIndex + 1;
				else
					break;
			}

			return !dependencyFound;
		}

		void add(const GBufferTexture& texture, size_t index)
		{
			for (size_t i = 0; i < texture.componentCount; i++)
				components[index + i].emplace_back(&texture);

			gbufferComponents[index].emplace_back(texture.name, texture.componentCount);
		}

		ImageFormat getImageFormat() const
		{
			// Get the smallest format possible
			size_t componentCount = 0;
			for (size_t i = 0; i < components.size(); i++)
			{
				if (components[i].empty())
					break;

				componentCount++;
			}

			ImageFormat format = getImageColorFormat(componentType, componentCount);

			// Ensure the format is valid, if not, increase the component count
			while (componentCount <= ComponentCount)
			{
				format = getImageColorFormat(componentType, componentCount);

				const auto usages = Renderer::instance().getImageFormatOptimalUsages(format);

				if (usages & ImageUsage::RenderTarget && usages & ImageUsage::ShaderSampling)
					break;

				// The component type is not supported, component count has no impact
				if (componentCount == ComponentCount)
					ATEMA_ERROR("Unsupported ImageFormat for GBuffer attachment");

				componentCount++;
			}
			
			return format;
		}

		ImageComponentType componentType;
		std::array<std::vector<const GBufferTexture*>, ComponentCount> components;
		std::vector<std::vector<GBuffer::Component>> gbufferComponents;
	};

	struct GBufferBuilder
	{
		GBufferBuilder() = delete;
		GBufferBuilder(const std::vector<LightingModel>& lightingModels)
		{
			createTextures(lightingModels);

			sortTextures();

			createPhysicalTextures();
		}

		void createTextures(const std::vector<LightingModel>& lightingModels)
		{
			for (const auto& lightingModel : lightingModels)
			{
				// Create textures for the current LightingModel if they don't exist
				for (const auto& parameter : lightingModel.parameters)
				{
					const auto it = m_textures.find(parameter.name);

					if (it == m_textures.end())
						m_textures.emplace(parameter.name, GBufferTexture(parameter));
					else if (it->second.format != parameter.format)
						ATEMA_ERROR("Parameters with the same name should have the same ImageFormat");
				}

				// Build dependencies once all LightingModel textures were created
				for (const auto& parameter : lightingModel.parameters)
				{
					GBufferTexture& texture = m_textures.find(parameter.name)->second;
					
					// Build dependencies
					for (const auto& dependency : lightingModel.parameters)
					{
						if (&parameter == &dependency)
							continue;

						ATEMA_ASSERT(parameter.name != dependency.name, "LightingModel parameters must have different names");

						texture.dependencies.emplace(&m_textures[dependency.name]);
					}
				}
			}
		}

		void sortTextures()
		{
			for (auto& [textureName, texture] : m_textures)
				m_sortedTextures[texture.componentType].emplace_back(&texture);

			for (auto& [componentType, textures] : m_sortedTextures)
			{
				std::sort(textures.begin(), textures.end(), [](const GBufferTexture* texture1, const GBufferTexture* texture2)
					{
						if (texture1->componentCount == texture2->componentCount)
							return texture1->name < texture2->name;

						return texture1->componentCount > texture2->componentCount;
					});
			}
		}

		void createPhysicalTextures()
		{
			for (auto& [componentType, sortedTextures] : m_sortedTextures)
			{
				for (auto& texture : sortedTextures)
				{
					auto& physicalTextures = m_physicalTextures[componentType];

					PhysicalTexture* currentPhysicalTexture = nullptr;
					size_t currentIndex = 0;

					// Get a compatible physical texture and try to reduce the remaining size as much as possible
					size_t currentRemainingSize = ComponentCount;
					for (auto& physicalTexture : physicalTextures)
					{
						size_t index = 0;
						if (physicalTexture.canBeAdded(*texture, index))
						{
							const size_t remainingSize = ComponentCount - index - texture->componentCount;

							if (!currentPhysicalTexture || remainingSize < currentRemainingSize)
							{
								currentPhysicalTexture = &physicalTexture;
								currentIndex = index;
								currentRemainingSize = remainingSize;
							}
						}
					}

					// If we couldn't find a compatible physical texture, create one
					if (!currentPhysicalTexture)
					{
						currentPhysicalTexture = &physicalTextures.emplace_back(texture->componentType);
						currentIndex = 0;
					}

					// Finally add the texture to the physical texture
					currentPhysicalTexture->add(*texture, currentIndex);
				}
			}
		}

		std::unordered_map<std::string, GBufferTexture> m_textures;
		std::unordered_map<ImageComponentType, std::vector<GBufferTexture*>> m_sortedTextures;
		std::unordered_map<ImageComponentType, std::vector<PhysicalTexture>> m_physicalTextures;
	};

	std::string getSuffixStr(ImageComponentType componentType)
	{
		switch (componentType)
		{
			case ImageComponentType::UNORM8:
			case ImageComponentType::SNORM8:
			case ImageComponentType::USCALED8:
			case ImageComponentType::SSCALED8:
			case ImageComponentType::SRGB8:
			case ImageComponentType::UNORM16:
			case ImageComponentType::SNORM16:
			case ImageComponentType::USCALED16:
			case ImageComponentType::SSCALED16:
			case ImageComponentType::SFLOAT16:
			case ImageComponentType::SFLOAT32:
				return "f";
			case ImageComponentType::SFLOAT64:
				return "d";
			case ImageComponentType::UINT8:
			case ImageComponentType::UINT16:
			case ImageComponentType::UINT32:
			case ImageComponentType::UINT64:
				return "u";
			case ImageComponentType::SINT8:
			case ImageComponentType::SINT16:
			case ImageComponentType::SINT32:
			case ImageComponentType::SINT64:
				return "i";
			default:
			{
				ATEMA_ERROR("Invalid ImageComponentType");
			}
		}

		return "";
	}

	std::string getTypeStr(ImageComponentType componentType, size_t componentCount)
	{
		if (componentCount > 1)
		{
			return std::string("vec") + std::to_string(componentCount) + getSuffixStr(componentType);
		}
		else
		{
			switch (componentType)
			{
				case ImageComponentType::UNORM8:
				case ImageComponentType::SNORM8:
				case ImageComponentType::USCALED8:
				case ImageComponentType::SSCALED8:
				case ImageComponentType::SRGB8:
				case ImageComponentType::UNORM16:
				case ImageComponentType::SNORM16:
				case ImageComponentType::USCALED16:
				case ImageComponentType::SSCALED16:
				case ImageComponentType::SFLOAT16:
				case ImageComponentType::SFLOAT32:
					return "float";
				case ImageComponentType::SFLOAT64:
					return "double";
				case ImageComponentType::UINT8:
				case ImageComponentType::UINT16:
				case ImageComponentType::UINT32:
				case ImageComponentType::UINT64:
					return "uint";
				case ImageComponentType::SINT8:
				case ImageComponentType::SINT16:
				case ImageComponentType::SINT32:
				case ImageComponentType::SINT64:
					return "int";
				default:
				{
					ATEMA_ERROR("Invalid ImageComponentType");
				}
			}
		}
		

		return "";
	}

	std::string getTypeStr(ImageFormat format)
	{
		const auto componentType = getComponentType(format);
		const auto componentCount = getComponentCount(format);

		return getTypeStr(componentType, componentCount);
	}

	std::string getTextureFilterStr(size_t colorChannel, size_t componentCount)
	{
		static const std::string filters[ComponentCount] = { "r", "g", "b", "a" };

		std::string filterStr = ".";

		for (size_t offset = 0; offset < componentCount; offset++)
			filterStr += filters[colorChannel + offset];

		return filterStr;
	}

	const std::string gbufferOptionsLibName = "Options";
	const std::string gbufferWriteOptionsLib = R"(
option
{
	int GBufferWriteLocation = 0;
})";

	const std::string gbufferReadOptionsLib = R"(
option
{
	int GBufferReadSet = 0;
	int GBufferReadBinding = 0;
})";

	std::string getReadOptionName(const GBuffer::Texture& texture)
	{
		return "GBufferRead" + texture.name + "Offset";
	}

	std::string getTextureWriteShader(size_t bindingOffset, const GBuffer::Texture& texture)
	{
		std::string libStr = "include Atema.GBufferWrite.Options;\n";
		libStr += "[stage(fragment)]\noutput\n{\t";
		libStr += "[location(GBufferWriteLocation +" + std::to_string(bindingOffset) + ")] ";
		libStr += getTypeStr(texture.format) + " " + texture.name + ";\n}";

		return libStr;
	}

	std::string getTextureReadShader(size_t bindingOffset, const GBuffer::Texture& texture)
	{
		const std::string readOptionName = getReadOptionName(texture);

		std::string libStr = "include Atema.GBufferRead.Options;\n";
		libStr += "option\n{\n\tint " + readOptionName + " = " + std::to_string(bindingOffset) + "; \n }\n";
		libStr += "external\n{\n\t[set(GBufferReadSet), binding(GBufferReadBinding + " + readOptionName + ")] ";
		libStr += "sampler2D" + getSuffixStr(getComponentType(texture.format)) + " " + texture.name + ";\n}\n";

		return libStr;
	}

	std::string getAliasWriteShader(const std::string& textureName, size_t colorChannel, ImageComponentType componentType, const GBuffer::Component& alias)
	{
		std::string libStr = "include Atema.GBufferWrite." + textureName + ";\n";
		libStr += "void GBufferWrite" + alias.name + "(" + getTypeStr(componentType, alias.size) + " value)\n";
		libStr += "{\n\t" + textureName + getTextureFilterStr(colorChannel, alias.size) + " = value;\n}";

		return libStr;
	}

	std::string getAliasReadShader(const std::string& textureName, size_t colorChannel, ImageComponentType componentType, const GBuffer::Component& alias)
	{
		std::string libStr = "include Atema.GBufferRead." + textureName + ";\n";
		libStr += getTypeStr(componentType, alias.size) + " GBufferRead" + alias.name + "(vec2f uv)\n";
		libStr += "{\n\treturn sample(" + textureName + ", uv)" + getTextureFilterStr(colorChannel, alias.size) + ";\n}";

		return libStr;
	}
}

// GBuffer::Component
GBuffer::Component::Component(const std::string& name, size_t size) :
	name(name),
	size(size)
{
	ATEMA_ASSERT(!name.empty(), "Invalid GBuffer component name");
	ATEMA_ASSERT(size > 0 && size <= ComponentCount, "Invalid GBuffer component size");
}

// GBuffer::Texture
GBuffer::Texture::Texture(const std::string& name, ImageFormat format) :
	name(name),
	format(format)
{
	ATEMA_ASSERT(!name.empty(), "Invalid GBuffer texture name");

	components.resize(getComponentCount(format));
}

void GBuffer::Texture::addComponent(size_t colorChannel, const Component& component)
{
	ATEMA_ASSERT(colorChannel + component.size <= components.size(), "GBuffer component does not fit into the texture");

	components[colorChannel].emplace_back(component);
}

// GBuffer
GBuffer::GBuffer(const std::vector<LightingModel>& lightingModels)
{
	GBufferBuilder builder(lightingModels);

	size_t textureIndex = 0;
	for (const auto& [componentType, physicalTextures] : builder.m_physicalTextures)
	{
		std::string textureSuffix = std::string("_") + componentToString(componentType);

		for (const auto& physicalTexture : physicalTextures)
		{
			std::string textureName = "GBufferTexture" + std::to_string(textureIndex++) + textureSuffix;

			GBuffer::Texture texture(textureName, physicalTexture.getImageFormat());

			for (size_t colorChannel = 0; colorChannel < physicalTexture.gbufferComponents.size(); colorChannel++)
			{
				for (const auto& gbufferComponent : physicalTexture.gbufferComponents[colorChannel])
					texture.addComponent(colorChannel, gbufferComponent);
			}

			addTexture(texture);
		}
	}
}

void GBuffer::addTexture(const Texture& texture)
{
	const size_t textureIndex = m_textures.size();

	m_textures.emplace_back(texture);

	for (const auto& components : texture.components)
	{
		for (const auto& component : components)
		{
			if (m_componentToTextureIndex.find(component.name) != m_componentToTextureIndex.end())
				ATEMA_ERROR("Component '" + std::string(component.name) + "' already exists in another texture");

			m_componentToTextureIndex[component.name] = textureIndex;
		}
	}
}

const std::vector<GBuffer::Texture>& GBuffer::getTextures() const noexcept
{
	return m_textures;
}

void GBuffer::generateShaderLibraries(ShaderLibraryManager& shaderLibraryManager)
{
	std::map<std::string, std::string> shaderLibraries;

	std::string writeLibName = "Atema.GBufferWrite";
	std::string readLibName = "Atema.GBufferRead";

	shaderLibraries[writeLibName + "." + gbufferOptionsLibName] = gbufferWriteOptionsLib;
	shaderLibraries[readLibName + "." + gbufferOptionsLibName] = gbufferReadOptionsLib;

	size_t bindingOffset = 0;
	for (const auto& texture : m_textures)
	{
		const auto componentType = getComponentType(texture.format);

		shaderLibraries[writeLibName + "." + texture.name] = getTextureWriteShader(bindingOffset, texture);
		shaderLibraries[readLibName + "." + texture.name] = getTextureReadShader(bindingOffset++, texture);

		for (size_t colorChannel = 0; colorChannel < texture.components.size(); colorChannel++)
		{
			for (const auto& component : texture.components[colorChannel])
			{
				const std::string writeAliasLibName = writeLibName + "." + component.name;
				const std::string readAliasLibName = readLibName + "." + component.name;

				shaderLibraries[writeAliasLibName] = getAliasWriteShader(texture.name, colorChannel, componentType, component);
				shaderLibraries[readAliasLibName] = getAliasReadShader(texture.name, colorChannel, componentType, component);

				shaderLibraries[writeLibName] += "include " + writeAliasLibName + ";\n";
				shaderLibraries[readLibName] += "include " + readAliasLibName + ";\n";
			}
		}
	}

	for (const auto& [libName, lib] : shaderLibraries)
	{
		AtslParser parser;

		AtslToAstConverter converter;

		auto ast = converter.createAst(parser.createTokens(lib));

		shaderLibraryManager.setLibrary(libName, std::move(ast));
	}
}

bool GBuffer::isCompatible(const LightingModel& lightingModel) const
{
	for (const auto& parameter : lightingModel.parameters)
	{
		const auto it = m_componentToTextureIndex.find(parameter.name);

		// If the component does not exist, the LightingModel is not compatible with the GBuffer
		if (it == m_componentToTextureIndex.end())
			return false;

		const auto& texture = m_textures[it->second];

		// The component must be of the same type
		if (getComponentType(texture.format) != getComponentType(parameter.format))
			return false;

		// The component must be of the same size
		for (const auto& components : texture.components)
		{
			for (const auto& component : components)
			{
				if (component.name != parameter.name)
					continue;

				if (component.size != getComponentCount(parameter.format))
					return false;
			}
		}
	}

	return true;
}

std::vector<GBuffer::TextureBinding> GBuffer::getTextureBindings(const std::vector<std::string>& componentNames) const
{
	std::map<size_t, GBuffer::TextureBinding> sortedTextureBindings;
	std::vector<GBuffer::TextureBinding> textureBindings;

	for (const auto& componentName : componentNames)
	{
		const auto it = m_componentToTextureIndex.find(componentName);

		ATEMA_ASSERT(it != m_componentToTextureIndex.end(), "The requested GBuffer component does not exist");

		if (sortedTextureBindings.find(it->second) == sortedTextureBindings.end())
		{
			const auto& texture = m_textures[it->second];

			auto& textureBinding = sortedTextureBindings[it->second];
			textureBinding.index = it->second;
			textureBinding.bindingOptionName = getReadOptionName(texture);
		}
	}

	textureBindings.reserve(sortedTextureBindings.size());
	for (auto& [index, binding] : sortedTextureBindings)
		textureBindings.emplace_back(std::move(binding));

	return textureBindings;
}

std::vector<GBuffer::TextureBinding> GBuffer::getTextureBindings(const LightingModel& lightingModel) const
{
	std::vector<std::string> parameterNames;
	parameterNames.reserve(lightingModel.parameters.size());

	for (const auto& parameter : lightingModel.parameters)
		parameterNames.emplace_back(parameter.name);

	return getTextureBindings(parameterNames);
}
