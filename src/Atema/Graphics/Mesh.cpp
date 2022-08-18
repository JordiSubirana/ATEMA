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

using namespace at;

Mesh::Mesh(Mesh&& other) noexcept
{
	operator=(std::move(other));
}

void Mesh::setVertexBuffer(const Ptr<VertexBuffer>& buffer)
{
	m_vertexBuffer = buffer;
}

void Mesh::setIndexBuffer(const Ptr<IndexBuffer>& buffer)
{
	m_indexBuffer = buffer;
}

void Mesh::setAABB(const AABBf& aabb)
{
	m_aabb = aabb;
}

const Ptr<VertexBuffer>& Mesh::getVertexBuffer() const noexcept
{
	return m_vertexBuffer;
}

const Ptr<IndexBuffer>& Mesh::getIndexBuffer() const noexcept
{
	return m_indexBuffer;
}

const AABBf& Mesh::getAABB() const noexcept
{
	return m_aabb;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
	m_vertexBuffer = std::move(other.m_vertexBuffer);
	m_indexBuffer = std::move(other.m_indexBuffer);

	m_aabb = std::move(other.m_aabb);

	return *this;
}
