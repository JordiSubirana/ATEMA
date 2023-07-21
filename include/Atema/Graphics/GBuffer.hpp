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

#ifndef ATEMA_GRAPHICS_GBUFFER_HPP
#define ATEMA_GRAPHICS_GBUFFER_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/LightingModel.hpp>

#include <string>

namespace at
{
	class ATEMA_GRAPHICS_API GBuffer
	{
	public:
		struct ATEMA_GRAPHICS_API Component
		{
			Component() = delete;
			Component(const std::string& name, size_t size);

			std::string name;
			size_t size;
		};

		struct ATEMA_GRAPHICS_API Texture
		{
			Texture() = delete;
			Texture(const std::string& name, ImageFormat format);

			void addComponent(size_t colorChannel, const Component& component);

			std::string name;
			ImageFormat format;
			// Each color channel is composed of one or multiple Components
			std::vector<std::vector<Component>> components;
		};

		struct TextureBinding
		{
			size_t index = 0;
			std::string bindingOptionName;
		};

		GBuffer() = default;
		// Build a GBuffer from LightingModels
		// Tries to fit all different components into the smallest GBuffer possible
		GBuffer(const std::vector<LightingModel>& lightingModels);
		GBuffer(const GBuffer& other) = default;
		GBuffer(GBuffer&& other) noexcept = default;
		~GBuffer() = default;

		void addTexture(const Texture& texture);

		const std::vector<Texture>& getTextures() const noexcept;

		void generateShaderLibraries(ShaderLibraryManager& shaderLibraryManager);

		bool isCompatible(const LightingModel& lightingModel) const;

		std::vector<TextureBinding> getTextureBindings(const std::vector<std::string>& componentNames) const;
		std::vector<TextureBinding> getTextureBindings(const LightingModel& lightingModel) const;

		GBuffer& operator=(const GBuffer& other) = default;
		GBuffer& operator=(GBuffer&& other) noexcept = default;

	private:
		std::vector<Texture> m_textures;
		std::unordered_map<std::string, size_t> m_componentToTextureIndex;
	};
}

#endif
