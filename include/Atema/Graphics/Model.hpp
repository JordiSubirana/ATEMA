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

#ifndef ATEMA_GRAPHICS_MODEL_HPP
#define ATEMA_GRAPHICS_MODEL_HPP

#include <Atema/Core/Pointer.hpp>
#include <Atema/Graphics/Config.hpp>
#include <Atema/Math/AABB.hpp>
#include <Atema/Core/Signal.hpp>

#include <vector>

namespace at
{
	class MaterialInstance;
	class Mesh;
	class MaterialData;

	class ATEMA_GRAPHICS_API Model
	{
	public:
		Model();
		Model(const Model& other) = default;
		Model(Model&& other) noexcept = default;
		~Model() = default;

		void addMesh(const Ptr<Mesh>& mesh);
		void addMaterialData(const Ptr<MaterialData>& materialData);
		void addMaterialInstance(Ptr<MaterialInstance> materialInstance);

		const AABBf& updateAABB();

		const std::vector<Ptr<Mesh>>& getMeshes() const noexcept;
		const std::vector<Ptr<MaterialData>>& getMaterialData() const noexcept;
		const std::vector<Ptr<MaterialInstance>>& getMaterialInstances() const noexcept;
		const AABBf& getAABB() const;
		size_t getTriangleCount() const noexcept;

		Model& operator=(const Model& other) = default;
		Model& operator=(Model&& other) noexcept = default;

		Signal<> onGeometryUpdate;

	private:
		std::vector<Ptr<Mesh>> m_meshes;
		std::vector<Ptr<MaterialData>> m_materialData;
		std::vector<Ptr<MaterialInstance>> m_materialInstances;

		size_t m_triangleCount;

		mutable bool m_aabbValid;
		mutable AABBf m_aabb;
	};
}

#endif
