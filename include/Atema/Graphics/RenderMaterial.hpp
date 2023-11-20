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

#ifndef ATEMA_GRAPHICS_RENDERMATERIAL_HPP
#define ATEMA_GRAPHICS_RENDERMATERIAL_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/MaterialParameters.hpp>
#include <Atema/Renderer/GraphicsPipeline.hpp>
#include <Atema/Core/IdManager.hpp>
#include <Atema/Graphics/RenderResource.hpp>
#include <Atema/Graphics/ShaderData.hpp>
#include <Atema/Core/Signal.hpp>

#include <limits>
#include <Atema/Shader/ShaderLibraryManager.hpp>
#include <Atema/Shader/UberShader.hpp>

namespace at
{
	class Material;
	class MaterialInstance;
	class CommandBuffer;
	class RenderMaterialInstance;

	class ATEMA_GRAPHICS_API RenderMaterial : public RenderResource
	{
	public:
		using ID = uint16_t;
		static constexpr ID InvalidID = std::numeric_limits<ID>::max();
		static constexpr uint32_t InvalidBindingIndex = std::numeric_limits<uint32_t>::max();

		struct Settings
		{
			Material* material = nullptr;
			ID id = InvalidID;
			std::vector<UberShader::Option> uberShaderOptions;
			const ShaderLibraryManager* shaderLibraryManager = nullptr;
			GraphicsPipeline::State pipelineState;
		};

		struct Binding
		{
			uint32_t set = InvalidBindingIndex;
			uint32_t binding = InvalidBindingIndex;
		};

		RenderMaterial() = delete;
		RenderMaterial(RenderResourceManager& resourceManager, const Settings& settings);
		RenderMaterial(const RenderMaterial& other) = delete;
		RenderMaterial(RenderMaterial&& other) noexcept = default;
		~RenderMaterial();

		ID getID() const noexcept;

		void bindTo(CommandBuffer& commandBuffer) const;

		Ptr<RenderMaterialInstance> createInstance(const MaterialInstance& materialInstance);

		bool hasBinding(const std::string& name) const;
		const Binding& getBinding(const std::string& name) const;
		size_t getDescriptorSetLayoutCount() const noexcept;

		Ptr<DescriptorSet> createSet(size_t setIndex) const;

		RenderMaterial& operator=(const RenderMaterial& other) = delete;
		RenderMaterial& operator=(RenderMaterial&& other) noexcept = default;

		Signal<> onDestroy;

	protected:
		void updateResources() override;

	private:
		Material* m_material;

		ID m_id;

		std::vector<Ptr<DescriptorSetLayout>> m_descriptorSetLayouts;
		Ptr<GraphicsPipeline> m_pipeline;

		std::map<std::string, Binding> m_bindings;

		MaterialParameters m_parameters;
		MaterialParameters m_newParameters;
		std::vector<Ptr<DescriptorSet>> m_descriptorSets;
		bool m_needUpdate;
		std::unordered_map<std::string, UPtr<ShaderData>> m_shaderDatas;

		IdManager<ID> m_instanceIdManager;

		ConnectionGuard m_connectionGuard;
	};

	class ATEMA_GRAPHICS_API RenderMaterialInstance : public RenderResource
	{
		friend class RenderMaterial;

	public:
		RenderMaterialInstance() = delete;
		RenderMaterialInstance(const RenderMaterialInstance& other) = delete;
		RenderMaterialInstance(RenderMaterialInstance&& other) noexcept = default;
		~RenderMaterialInstance();

		RenderMaterial::ID getID() const noexcept;

		const RenderMaterial& getRenderMaterial() const noexcept;

		// The user needs to bind the RenderMaterial before calling this method
		void bindTo(CommandBuffer& commandBuffer) const;

		RenderMaterialInstance& operator=(const RenderMaterialInstance& other) = delete;
		RenderMaterialInstance& operator=(RenderMaterialInstance&& other) noexcept = default;

		Signal<> onDestroy;

	protected:
		void updateResources() override;

	private:
		RenderMaterialInstance(const RenderMaterial& renderMaterial, const MaterialInstance& materialInstance, RenderMaterial::ID id);
		
		const RenderMaterial* m_renderMaterial;
		const MaterialInstance* m_materialInstance;
		RenderMaterial::ID m_id;

		MaterialParameters m_parameters;
		MaterialParameters m_newParameters;
		std::vector<Ptr<DescriptorSet>> m_descriptorSets;
		bool m_needUpdate;
		std::unordered_map<std::string, UPtr<ShaderData>> m_shaderDatas;

		ConnectionGuard m_connectionGuard;
	};
}

#endif
