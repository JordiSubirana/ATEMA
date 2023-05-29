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

#ifndef ATEMA_GRAPHICS_MESH_HPP
#define ATEMA_GRAPHICS_MESH_HPP

#include <Atema/Core/Pointer.hpp>
#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/VertexFormat.hpp>
#include <Atema/Math/AABB.hpp>

namespace at
{
	class VertexBuffer;
	class IndexBuffer;

	class ATEMA_GRAPHICS_API Mesh
	{
	public:
		Mesh();
		Mesh(const Mesh& other) = default;
		Mesh(Mesh&& other) noexcept;
		~Mesh() = default;
		
		void setVertexBuffer(const Ptr<VertexBuffer>& vertexBuffer);
		void setIndexBuffer(const Ptr<IndexBuffer>& indexBuffer);
		void setAABB(const AABBf& aabb);
		// The ID refers to the material index in the parent model
		void setMaterialID(size_t materialID);

		const Ptr<VertexBuffer>& getVertexBuffer() const noexcept;
		const Ptr<IndexBuffer>& getIndexBuffer() const noexcept;
		const AABBf& getAABB() const noexcept;
		// The ID refers to the material index in the parent mesh
		size_t getMaterialID() const noexcept;
		size_t getTriangleCount() const noexcept;

		Mesh& operator=(const Mesh& other) = default;
		Mesh& operator=(Mesh&& other) noexcept;
		
	private:
		Ptr<VertexBuffer> m_vertexBuffer;
		Ptr<IndexBuffer> m_indexBuffer;

		AABBf m_aabb;

		size_t m_materialID;

		size_t m_triangleCount;
	};
}

#endif
