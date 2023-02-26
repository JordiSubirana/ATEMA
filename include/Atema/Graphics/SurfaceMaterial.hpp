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

#ifndef ATEMA_GRAPHICS_SURFACEMATERIAL_HPP
#define ATEMA_GRAPHICS_SURFACEMATERIAL_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Core/IdManager.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/MaterialParameters.hpp>
#include <Atema/Renderer/Image.hpp>

namespace at
{
	class CommandBuffer;
	class DescriptorSet;
	class DescriptorSetLayout;
	class GraphicsPipeline;
	class Shader;
	class SurfaceMaterialInstance;
	
	struct ATEMA_GRAPHICS_API SurfaceMaterialData
	{
		SurfaceMaterialData();
		SurfaceMaterialData(const SurfaceMaterialData& other) = default;
		SurfaceMaterialData(SurfaceMaterialData&& other) noexcept;
		~SurfaceMaterialData();

		Ptr<Image> colorMap;
		Ptr<Image> normalMap;
		Ptr<Image> ambientOcclusionMap;
		Ptr<Image> heightMap;
		Ptr<Image> emissiveMap;
		Ptr<Image> metalnessMap;
		Ptr<Image> roughnessMap;
		Ptr<Image> alphaMaskMap;

		Vector4f color = Vector4f(1.0f, 1.0f, 1.0f, 1.0f);
		Vector3f emissive = Vector3f(0.0f, 0.0f, 0.0f);
		float metalness = 0.0f;
		float roughness = 0.0f;
	};

	template <>
	struct HashOverload<SurfaceMaterialData>
	{
		template <typename Hasher>
		static constexpr auto hash(const SurfaceMaterialData& data)
		{
			// For example here we are combining 2 members
			typename Hasher::HashType hash = 0;

			Hasher::hashCombine(hash, data.colorMap);
			Hasher::hashCombine(hash, data.normalMap);
			Hasher::hashCombine(hash, data.ambientOcclusionMap);
			Hasher::hashCombine(hash, data.heightMap);
			Hasher::hashCombine(hash, data.emissiveMap);
			Hasher::hashCombine(hash, data.metalnessMap);
			Hasher::hashCombine(hash, data.roughnessMap);
			Hasher::hashCombine(hash, data.alphaMaskMap);

			Hasher::hashCombine(hash, color);
			Hasher::hashCombine(hash, normal);
			Hasher::hashCombine(hash, ambientOcclusion);
			Hasher::hashCombine(hash, height);
			Hasher::hashCombine(hash, emissive);
			Hasher::hashCombine(hash, metalness);
			Hasher::hashCombine(hash, roughness);
			Hasher::hashCombine(hash, alphaMask);

			return hash;
		}
	};

	class ATEMA_GRAPHICS_API SurfaceMaterial
	{
		friend class SurfaceMaterialInstance;

	public:
		using ID = uint16_t;
		static constexpr ID InvalidID = std::numeric_limits<ID>::max();

		// Set index for per-frame data
		static constexpr uint32_t FrameSetIndex = 0;
		// Set index for per-object data
		static constexpr uint32_t ObjectSetIndex = 1;
		// Set index for global material data shared across all material instances
		static constexpr uint32_t MaterialSetIndex = 2;
		// Set index for per-instance material data
		static constexpr uint32_t InstanceSetIndex = 3;

		struct Settings
		{
			// Graphics instance used to store & retrieve resources
			Graphics& graphics = Graphics::instance();

			// Vertex input (for vertex shader)
			// Meshes used with this material must have a matching format
			std::vector<VertexInput> vertexInput;
			
			Ptr<Shader> vertexShader;
			Ptr<Shader> fragmentShader;

			// Global material layout used across all material instances
			Ptr<DescriptorSetLayout> layout;
			// Material instance layout
			Ptr<DescriptorSetLayout> instanceLayout;
		};

		SurfaceMaterial() = delete;
		SurfaceMaterial(const Settings& settings);
		SurfaceMaterial(const SurfaceMaterial& other) = default;
		SurfaceMaterial(SurfaceMaterial&& other) noexcept = default;
		~SurfaceMaterial();

		ID getID() const noexcept;

		// Creates a default SurfaceMaterial with built-in shaders
		// The user must then create an SurfaceMaterialInstance and initialize it with the material data
		// instanceLayoutPageSize is an hint of which descriptor set layout to use for instance set allocation
		// graphics is the Graphics instance used to store & retrieve resources
		static Ptr<SurfaceMaterial> createDefault(const SurfaceMaterialData& materialData, uint32_t instanceLayoutPageSize = 0, Graphics& graphics = Graphics::instance());

		// Binds the graphics pipeline and the descriptor set (if any) to a command buffer
		// The user is reponsible for binding per-frame & per-object descriptor sets
		void bindTo(CommandBuffer& commandBuffer);

		const Ptr<GraphicsPipeline>& getGraphicsPipeline() const noexcept;
		const Ptr<DescriptorSetLayout>& getLayout() const noexcept;
		const Ptr<DescriptorSetLayout>& getInstanceLayout() const noexcept;
		// Returns the set containing material global bindings
		const Ptr<DescriptorSet>& getDescriptorSet() const noexcept;
		// Optional references to all resources used by the material
		// This member is here to help managing resources lifetime
		// Updating those parameters won't update the descriptor set
		MaterialParameters& getParameters() noexcept;
		// Optional references to all resources used by the material
		// This member is here to help managing resources lifetime
		// Updating those parameters won't update the descriptor set
		const MaterialParameters& getParameters() const noexcept;

		SurfaceMaterial& operator=(const SurfaceMaterial& other) = default;
		SurfaceMaterial& operator=(SurfaceMaterial&& other) noexcept = default;

	private:
		ID getInstanceId();
		void releaseInstanceId(ID id);

		Ptr<GraphicsPipeline> m_graphicsPipeline;
		Ptr<DescriptorSetLayout> m_layout;
		Ptr<DescriptorSetLayout> m_instanceLayout;
		Ptr<DescriptorSet> m_descriptorSet;
		MaterialParameters m_parameters;
		ID m_id;
		IdManager<ID> m_instanceIdManager;
	};

	class ATEMA_GRAPHICS_API SurfaceMaterialInstance
	{
	public:
		SurfaceMaterialInstance() = delete;
		SurfaceMaterialInstance(const Ptr<SurfaceMaterial>& material);
		SurfaceMaterialInstance(const SurfaceMaterialInstance& other) = default;
		SurfaceMaterialInstance(SurfaceMaterialInstance&& other) noexcept = default;
		~SurfaceMaterialInstance();

		SurfaceMaterial::ID getID() const noexcept;

		// Creates a default SurfaceMaterialInstance from SurfaceMaterial with built-in shaders
		// graphics is the Graphics instance used to store & retrieve resources
		static Ptr<SurfaceMaterialInstance> createDefault(const Ptr<SurfaceMaterial>& material, const SurfaceMaterialData& materialData, Graphics& graphics = Graphics::instance());

		// Binds the descriptor set (if any) to a command buffer
		// The user is reponsible for binding surface material, per-frame & per-object descriptor sets
		void bindTo(CommandBuffer& commandBuffer);

		const Ptr<SurfaceMaterial>& getMaterial() const noexcept;
		// Returns the set containing material instance bindings
		const Ptr<DescriptorSet>& getDescriptorSet() const noexcept;
		// Optional references to all resources used by the material
		// This member is here to help managing resources lifetime
		// Updating those parameters won't update the descriptor set
		MaterialParameters& getParameters() noexcept;
		// Optional references to all resources used by the material
		// This member is here to help managing resources lifetime
		// Updating those parameters won't update the descriptor set
		const MaterialParameters& getParameters() const noexcept;

		SurfaceMaterialInstance& operator=(const SurfaceMaterialInstance& other) = default;
		SurfaceMaterialInstance& operator=(SurfaceMaterialInstance&& other) noexcept = default;

	private:
		Ptr<SurfaceMaterial> m_material;
		Ptr<DescriptorSet> m_descriptorSet;

		MaterialParameters m_parameters;

		SurfaceMaterial::ID m_id;
	};
}

#endif
