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

#include <Atema/Graphics/Mesh.hpp>
#include <Atema/Graphics/Model.hpp>

using namespace at;

Model::Model() :
	m_aabbValid(false)
{
}

void Model::addMesh(const Ptr<Mesh>& mesh)
{
	m_meshes.emplace_back(mesh);

	m_aabbValid = false;
}

void Model::addMaterial(const Ptr<SurfaceMaterialData>& material)
{
	m_materials.emplace_back(material);
}

const AABBf& Model::updateAABB()
{
	m_aabbValid = false;

	return getAABB();
}

const std::vector<Ptr<Mesh>>& Model::getMeshes() const noexcept
{
	return m_meshes;
}

const std::vector<Ptr<SurfaceMaterialData>>& Model::getMaterials() const noexcept
{
	return m_materials;
}

const AABBf& Model::getAABB() const
{
	if (!m_aabbValid)
	{
		m_aabb = AABBf();

		for (const auto& mesh : m_meshes)
			m_aabb.extend(mesh->getAABB());

		m_aabbValid = true;
	}

	return m_aabb;
}
