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

#ifndef ATEMA_GRAPHICS_GRAPHICS_HPP
#define ATEMA_GRAPHICS_GRAPHICS_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Shader/UberShader.hpp>
#include <Atema/Renderer/Sampler.hpp>
#include <Atema/Renderer/Shader.hpp>
#include <Atema/Renderer/GraphicsPipeline.hpp>
#include <Atema/Graphics/Loaders/ImageLoader.hpp>
#include <Atema/Shader/ShaderLibraryManager.hpp>

namespace at
{
	struct SurfaceMaterialData;
	class SurfaceMaterial;
	class SurfaceMaterialInstance;
	class VertexBuffer;
	
	// Class managing all resources needed to do rendering
	class ATEMA_GRAPHICS_API Graphics
	{
	public:
		Graphics();
		~Graphics();

		// Default Graphics instance
		static Graphics& instance();

		// When a resource is not used for a number of iterations, it is removed
		// Default value is 0, meaning unused resources will be removed at each call of clearUnused()
		void setMaxUnusedCounter(uint32_t counter);

		// Updates the internal counter of each resource depending if it is in use
		// Then remove unused ones, depending on the max unused counter
		void clearUnused();

		// Forces the deletion of every resource no matter it is in use or not
		// If the user owns a resource at this time (saving a Ptr to the resource) it will not be deleted
		void clear();

		// Initializes ShaderLibraryManager with built-in shader libraries
		void initializeShaderLibraries(ShaderLibraryManager& libraryManager);

		// Default quad mesh (x & y : [-1,1], z : 0)
		Ptr<VertexBuffer> getQuadMesh();

		// Saves a UberShader an associates it to an identifier
		void setUberShader(const std::string& identifier, const std::string& shaderCode);
		bool uberShaderExists(const std::string& identifier) const;
		// Returns a UberShader previously saved
		// See setUberShader
		Ptr<UberShader> getUberShader(const std::string& identifier);
		// Returns a UberShader loaded from a file
		// The UberShader won't pass through a preprocessor stage, use another overload with empty options to get a preprocessed shader
		Ptr<UberShader> getUberShader(const std::filesystem::path& path);
		// Returns a UberShader instance created from a base UberShader and some options
		Ptr<UberShader> getUberShader(const UberShader& baseUberShader, const std::vector<UberShader::Option>& options);
		// Returns a UberShader instance representing only one shader stage of a base UberShader
		Ptr<UberShader> getUberShader(const UberShader& baseUberShader, AstShaderStage shaderStage);
		// Convenience method to get a specific UberShader using other overloads
		Ptr<UberShader> getUberShader(const std::filesystem::path& path, AstShaderStage shaderStage, const std::vector<UberShader::Option>& options = {});
		Ptr<UberShader> getUberShaderFromString(const std::string& identifier, AstShaderStage shaderStage, const std::vector<UberShader::Option>& options = {});

		// Returns a Shader module created from a UberShader
		Ptr<Shader> getShader(const UberShader& uberShader);

		// Returns a DescriptorSetLayout
		Ptr<DescriptorSetLayout> getDescriptorSetLayout(const DescriptorSetLayout::Settings& settings);

		// Returns the default GraphicsPipeline::Settings for a given set of shaders
		// Everything is set to the default value, except for :
		// - GraphicsPipeline::Settings::vertexShader
		// - GraphicsPipeline::Settings::fragmentShader
		// - GraphicsPipeline::Settings::vertexInput (set according to vertexShader's input parameters)
		//	- All binding indices are set to 0
		//	- The user must override these indices to bind multiple buffers
		// - GraphicsPipeline::Settings::descriptorSetLayouts (set according to shaders' external parameters)
		//	- DescriptorType is set to CombinedImageSampler for sample types
		//	- DescriptorType is set to UniformBuffer for primitive/vector/matrix/struct types
		//	- The user may change the sampler type or buffer type for specific cases
		//	- See Renderer/Utils.hpp for more details
		Ptr<GraphicsPipeline::Settings> getGraphicsPipelineSettings(const UberShader& vertexShader, const UberShader& fragmentShader);

		Ptr<GraphicsPipeline> getGraphicsPipeline(const GraphicsPipeline::Settings& settings);
		
		Ptr<Image> getImage(const std::filesystem::path& path, const ImageLoader::Settings& settings = {});

		Ptr<Sampler> getSampler(const Sampler::Settings& settings);

		// instanceLayoutPageSize is an hint of which descriptor set layout to use for instance set allocation
		Ptr<SurfaceMaterial> getSurfaceMaterial(const SurfaceMaterialData& materialData, uint32_t instanceLayoutPageSize = 0);

		Ptr<SurfaceMaterialInstance> getSurfaceMaterialInstance(const Ptr<SurfaceMaterial>& material, const SurfaceMaterialData& materialData);

	private:
		struct UberInstanceSettings
		{
			UberInstanceSettings() = delete;
			UberInstanceSettings(const UberShader* uberShader, const std::vector<UberShader::Option>& options);

			static StdHash hash(const UberInstanceSettings& settings);

			const UberShader* uberShader;
			const std::vector<UberShader::Option>& options;
		};

		struct UberStageSettings
		{
			UberStageSettings() = delete;
			UberStageSettings(const UberShader* uberShader, AstShaderStage shaderStage);

			const UberShader* uberShader;
			AstShaderStage shaderStage;
		};

		struct GraphicsPipelineSettings
		{
			const UberShader* vertexShader;
			const UberShader* fragmentShader;
		};

		struct ImageSettings
		{
			ImageSettings() = delete;
			ImageSettings(const std::filesystem::path& path, const ImageLoader::Settings& settings);

			static StdHash hash(const ImageSettings& settings);

			const std::filesystem::path& path;
			const ImageLoader::Settings& settings;
		};

		struct DefaultSurfaceMaterialSettings
		{
			DefaultSurfaceMaterialSettings() = delete;
			DefaultSurfaceMaterialSettings(const SurfaceMaterialData& materialData, uint32_t instanceLayoutPageSize);

			static StdHash hash(const DefaultSurfaceMaterialSettings& settings);

			const SurfaceMaterialData& materialData;
			uint32_t instanceLayoutPageSize;
		};

		struct DefaultSurfaceInstanceSettings
		{
			DefaultSurfaceInstanceSettings() = delete;
			DefaultSurfaceInstanceSettings(const Ptr<SurfaceMaterial>& material, const SurfaceMaterialData& materialData);

			const Ptr<SurfaceMaterial>& material;
			const SurfaceMaterialData& materialData;
		};

		Ptr<UberShader> loadUberShader(const std::filesystem::path& path);
		Ptr<UberShader> loadUberInstance(const UberInstanceSettings& settings);
		Ptr<UberShader> loadUberStage(const UberStageSettings& settings);
		Ptr<Shader> loadShader(const UberShader& uberShader);
		static Ptr<DescriptorSetLayout> loadDescriptorSetLayout(const DescriptorSetLayout::Settings& settings);
		Ptr<GraphicsPipeline::Settings> loadGraphicsPipelineSettings(const GraphicsPipelineSettings& settings);
		static Ptr<GraphicsPipeline> loadGraphicsPipeline(const GraphicsPipeline::Settings& settings);
		static Ptr<Image> loadImage(const ImageSettings& settings);
		static Ptr<Sampler> loadSampler(const Sampler::Settings& settings);
		Ptr<SurfaceMaterial> loadSurfaceMaterial(const DefaultSurfaceMaterialSettings& settings);
		Ptr<SurfaceMaterialInstance> loadSurfaceMaterialInstance(const DefaultSurfaceInstanceSettings& settings);
		
		std::vector<AbstractResourceManager*> m_resourceManagers;

		Ptr<VertexBuffer> m_quadMesh;

		ResourceManager<UberShader, std::string> m_uberShaderIdManager;
		ResourceManager<UberShader> m_uberShaderManager;
		ResourceManager<UberShader, UberInstanceSettings> m_uberShaderOptionsManager;
		ResourceManager<UberShader, UberStageSettings> m_uberShaderStageManager;
		ResourceManager<Shader, const UberShader*> m_shaderManager;
		ResourceManager<DescriptorSetLayout, DescriptorSetLayout::Settings> m_descriptorSetLayoutManager;
		ResourceManager<GraphicsPipeline::Settings, GraphicsPipelineSettings> m_graphicsPipelineSettingsManager;
		ResourceManager<GraphicsPipeline, GraphicsPipeline::Settings> m_graphicsPipelineManager;
		ResourceManager<Image, ImageSettings> m_imageManager;
		ResourceManager<Sampler, Sampler::Settings> m_samplerManager;
		ResourceManager<SurfaceMaterial, DefaultSurfaceMaterialSettings> m_defaultSurfaceMaterialSettingsManager;
		ResourceManager<SurfaceMaterialInstance, DefaultSurfaceInstanceSettings> m_defaultSurfaceInstanceSettingsManager;

		std::unordered_map<const UberShader*, WPtr<UberShader>> m_uberShaders;
		std::unordered_map<Shader*, Ptr<UberShader>> m_shaderToUber;
	};
}

#endif
