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

#ifndef ATEMA_GRAPHICS_MATERIAL_HPP
#define ATEMA_GRAPHICS_MATERIAL_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/MaterialParameters.hpp>
#include <Atema/Renderer/GraphicsPipeline.hpp>
#include <Atema/Graphics/ShaderData.hpp>
#include <Atema/Core/Signal.hpp>
#include <Atema/Graphics/MaterialData.hpp>

namespace at
{
	class CommandBuffer;
	class UberShader;
	class MaterialInstance;

	class ATEMA_GRAPHICS_API Material
	{
	public:
		Material() = delete;
		Material(const UberShader& uberShader, const MaterialData& metaData = {});
		Material(const Material& other) = delete;
		Material(Material&& other) noexcept = default;
		~Material();

		const UberShader& getUberShader() const noexcept;
		const MaterialData& getMetaData() const noexcept;

		void setParameter(const std::string& name, const ShaderData& shaderData);
		void setParameter(const std::string& name, const Ptr<Image>& image, const Ptr<Sampler>& sampler);

		const MaterialParameters& getParameters() const noexcept;

		Material& operator=(const Material& other) = delete;
		Material& operator=(Material&& other) noexcept = default;

		Signal<> onDestroy;
		Signal<> onParameterUpdated;

	private:
		const UberShader* m_uberShader;
		MaterialData m_metaData;

		MaterialParameters m_parameters;
	};

	class ATEMA_GRAPHICS_API MaterialInstance
	{
		friend class Material;

	public:
		MaterialInstance() = delete;
		MaterialInstance(Ptr<Material> material);
		MaterialInstance(const MaterialInstance& other) = delete;
		MaterialInstance(MaterialInstance&& other) noexcept = default;
		~MaterialInstance();

		const Ptr<Material>& getMaterial() const noexcept;

		void setParameter(const std::string& name, const ShaderData& shaderData);
		void setParameter(const std::string& name, const Ptr<Image>& image, const Ptr<Sampler>& sampler);

		const MaterialParameters& getParameters() const noexcept;

		MaterialInstance& operator=(const MaterialInstance& other) = delete;
		MaterialInstance& operator=(MaterialInstance&& other) noexcept = default;

		Signal<> onDestroy;
		Signal<> onParameterUpdated;

	private:
		Ptr<Material> m_material;

		MaterialParameters m_parameters;
	};
}

#endif
