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

#ifndef ATEMA_GRAPHICS_MATERIALDATA_HPP
#define ATEMA_GRAPHICS_MATERIALDATA_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Core/Variant.hpp>
#include <Atema/Renderer/Color.hpp>

#include <filesystem>
#include <unordered_map>

#define ATEMA_DEFINE_MATERIAL_DATA(atMatName) static constexpr const char* atMatName = #atMatName;

namespace at
{
	class ATEMA_GRAPHICS_API MaterialData
	{
	public:
		ATEMA_DEFINE_MATERIAL_DATA(AlphaMaskMap)
		ATEMA_DEFINE_MATERIAL_DATA(AlphaMaskThreshold)
		ATEMA_DEFINE_MATERIAL_DATA(AmbientOcclusionMap)
		ATEMA_DEFINE_MATERIAL_DATA(BaseColor)
		ATEMA_DEFINE_MATERIAL_DATA(BaseColorMap)
		ATEMA_DEFINE_MATERIAL_DATA(EmissiveColor)
		ATEMA_DEFINE_MATERIAL_DATA(EmissiveColorMap)
		ATEMA_DEFINE_MATERIAL_DATA(HeightMap)
		ATEMA_DEFINE_MATERIAL_DATA(LightingModel)
		ATEMA_DEFINE_MATERIAL_DATA(Metalness)
		ATEMA_DEFINE_MATERIAL_DATA(MetalnessMap)
		ATEMA_DEFINE_MATERIAL_DATA(NormalMap)
		ATEMA_DEFINE_MATERIAL_DATA(Roughness)
		ATEMA_DEFINE_MATERIAL_DATA(RoughnessMap)
		ATEMA_DEFINE_MATERIAL_DATA(Shininess)
		ATEMA_DEFINE_MATERIAL_DATA(SpecularColor)
		ATEMA_DEFINE_MATERIAL_DATA(SpecularColorMap)

		struct ATEMA_GRAPHICS_API Texture
		{
			Texture(const std::filesystem::path& path);

			std::filesystem::path path;
		};

		using Value = Variant<int32_t, uint32_t, float, double, Color, std::string, MaterialData::Texture>;

		struct Parameter
		{
			Parameter() = default;
			Parameter(const std::string& name, const Value& value);
			Parameter(const Parameter& other);
			Parameter(Parameter&& other) noexcept = default;
			~Parameter() = default;

			Parameter& operator=(const Parameter& other) = default;
			Parameter& operator=(Parameter&& other) noexcept = default;

			std::string name;
			Value value;
		};

		MaterialData() = default;
		MaterialData(const MaterialData& other) = default;
		MaterialData(MaterialData&& other) noexcept = default;
		~MaterialData() = default;

		void set(const Parameter& parameter);
		void set(const std::string& name, const Value& value);

		bool exists(const std::string& name) const noexcept;

		Value& getValue(const std::string& name);
		const Value& getValue(const std::string& name) const;

		const std::vector<Parameter>& getParameters() const noexcept;

		MaterialData& operator=(const MaterialData& other) = default;
		MaterialData& operator=(MaterialData&& other) noexcept = default;

	private:
		std::unordered_map<std::string, size_t> m_indices;
		std::vector<Parameter> m_parameters;
	};
}

#endif
