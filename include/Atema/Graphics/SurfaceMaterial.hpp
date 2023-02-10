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
#include <Atema/Core/Pointer.hpp>
#include <Atema/Renderer/Image.hpp>

namespace at
{
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
		Vector3f normal = Vector3f(0.5f, 0.5f, 1.0f);
		float ambientOcclusion = 1.0f;
		float height = 1.0f;
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
}

#endif