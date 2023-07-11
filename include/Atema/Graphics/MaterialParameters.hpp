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

#ifndef ATEMA_GRAPHICS_MATERIALPARAMETERS_HPP
#define ATEMA_GRAPHICS_MATERIALPARAMETERS_HPP

#include <Atema/Core/Pointer.hpp>
#include <Atema/Core/Variant.hpp>
#include <Atema/Graphics/Config.hpp>
#include <Atema/Renderer/Buffer.hpp>
#include <Atema/Renderer/Image.hpp>
#include <Atema/Renderer/Sampler.hpp>

#include <vector>
#include <map>

namespace at
{
	class DescriptorSet;

	struct ATEMA_GRAPHICS_API MaterialParameter
	{
		struct Texture
		{
			Texture() = default;
			Texture(const Ptr<Image>& image, const Ptr<Sampler>& sampler);
			Texture(const Texture& other) = default;
			Texture(Texture&& other) noexcept = default;
			~Texture() = default;

			Texture& operator=(const Texture& other) = default;
			Texture& operator=(Texture&& other) noexcept = default;

			Ptr<Image> image;
			Ptr<Sampler> sampler;
		};

		struct Buffer
		{
			Buffer() = default;
			Buffer(const Ptr<at::Buffer>& buffer);
			Buffer(const Buffer& other) = default;
			Buffer(Buffer&& other) noexcept = default;
			~Buffer() = default;

			Buffer& operator=(const Buffer& other) = default;
			Buffer& operator=(Buffer&& other) noexcept = default;

			Ptr<at::Buffer> buffer;
		};

		MaterialParameter();
		MaterialParameter(const std::string& name);
		MaterialParameter(const std::string& name, const Ptr<Image>& image, const Ptr<Sampler>& sampler);
		MaterialParameter(const std::string& name, const Ptr<at::Buffer>& buffer);
		MaterialParameter(const MaterialParameter& other);
		MaterialParameter(MaterialParameter&& other) noexcept = default;
		~MaterialParameter();

		void updateDescriptorSet(DescriptorSet& descriptorSet, uint32_t bindingIndex) const;

		MaterialParameter& operator=(const MaterialParameter& other);
		MaterialParameter& operator=(MaterialParameter&& other) noexcept = default;

		std::string name;
		Variant<Texture, Buffer> value;
	};

	class ATEMA_GRAPHICS_API MaterialParameters
	{
	public:
		MaterialParameters();
		MaterialParameters(const MaterialParameters& other) = default;
		MaterialParameters(MaterialParameters&& other) = default;
		~MaterialParameters();

		bool exists(const std::string& parameterName) const noexcept;

		void set(const MaterialParameter& parameter);
		void set(MaterialParameter&& parameter);

		template <typename ... Args>
		void set(Args&&... args)
		{
			emplace({ std::forward<Args>(args)... });
		}

		MaterialParameter& get(const std::string& parameterName);
		const MaterialParameter& get(const std::string& parameterName) const;
		const std::vector<MaterialParameter>& getParameters() const;

		Hash getHash() const;

		MaterialParameters& operator=(const MaterialParameters& other) = default;
		MaterialParameters& operator=(MaterialParameters&& other) noexcept = default;
		
	private:
		void emplace(const MaterialParameter& parameter);
		void emplace(MaterialParameter&& parameter);

		std::vector<MaterialParameter> m_parameters;
		std::map<std::string, MaterialParameter*> m_orderedParameters;
	};

	template <>
	struct HashOverload<MaterialParameter>
	{
		template <typename Hasher>
		static constexpr auto hash(const MaterialParameter& object)
		{
			// For example here we are combining 2 members
			typename Hasher::HashType hash = 0;

			Hasher::hashCombine(hash, object.name);

			if (object.value.is<MaterialParameter::Texture>())
			{
				const auto& texture = object.value.get<MaterialParameter::Texture>();

				Hasher::hashCombine(hash, texture.image.get());
				Hasher::hashCombine(hash, texture.sampler.get());
			}
			else
			{
				const auto& buffer = object.value.get<MaterialParameter::Buffer>();

				Hasher::hashCombine(hash, buffer.buffer.get());
			}

			return hash;
		}
	};

	template <>
	struct HashOverload<MaterialParameters>
	{
		template <typename Hasher>
		static constexpr auto hash(const MaterialParameters& object)
		{
			return Hasher::hash(object.getHash());
		}
	};
}

#endif
