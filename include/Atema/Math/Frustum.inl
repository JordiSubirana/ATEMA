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

#ifndef ATEMA_MATH_FRUSTUM_INL
#define ATEMA_MATH_FRUSTUM_INL

#include <Atema/Math/Frustum.hpp>

namespace at
{
	template <typename T>
	Frustum<T>::Frustum(const std::array<Plane<T>, 6>& planes)
	{
		set(planes);
	}

	template <typename T>
	Frustum<T>::Frustum(const Matrix4<T>& projection)
	{
		set(projection);
	}

	template <typename T>
	void Frustum<T>::set(const std::array<Plane<T>, 6>& planes)
	{
		m_planes = planes;

		updatePositiveVertexIndices();
	}

	template <typename T>
	void Frustum<T>::set(const Matrix4<T>& viewProjection)
	{
		size_t planeIndex = 0;

		Vector3<T> normal;
		T norm;
		T distance;

		// Left
		normal.x = viewProjection[0][3] + viewProjection[0][0];
		normal.y = viewProjection[1][3] + viewProjection[1][0];
		normal.z = viewProjection[2][3] + viewProjection[2][0];
		distance = viewProjection[3][3] + viewProjection[3][0];

		norm = normal.getNorm();
		normal /= norm;
		distance /= norm;

		m_planes[planeIndex++].set(normal, -distance);

		// Right
		normal.x = viewProjection[0][3] - viewProjection[0][0];
		normal.y = viewProjection[1][3] - viewProjection[1][0];
		normal.z = viewProjection[2][3] - viewProjection[2][0];
		distance = viewProjection[3][3] - viewProjection[3][0];

		norm = normal.getNorm();
		normal /= norm;
		distance /= norm;

		m_planes[planeIndex++].set(normal, -distance);

		// Bottom
		normal.x = viewProjection[0][3] + viewProjection[0][1];
		normal.y = viewProjection[1][3] + viewProjection[1][1];
		normal.z = viewProjection[2][3] + viewProjection[2][1];
		distance = viewProjection[3][3] + viewProjection[3][1];

		norm = normal.getNorm();
		normal /= norm;
		distance /= norm;

		m_planes[planeIndex++].set(normal, -distance);

		// Top
		normal.x = viewProjection[0][3] - viewProjection[0][1];
		normal.y = viewProjection[1][3] - viewProjection[1][1];
		normal.z = viewProjection[2][3] - viewProjection[2][1];
		distance = viewProjection[3][3] - viewProjection[3][1];

		norm = normal.getNorm();
		normal /= norm;
		distance /= norm;

		m_planes[planeIndex++].set(normal, -distance);

		// Near
		normal.x = viewProjection[0][3] + viewProjection[0][2];
		normal.y = viewProjection[1][3] + viewProjection[1][2];
		normal.z = viewProjection[2][3] + viewProjection[2][2];
		distance = viewProjection[3][3] + viewProjection[3][2];

		norm = normal.getNorm();
		normal /= norm;
		distance /= norm;

		m_planes[planeIndex++].set(normal, -distance);

		// Far
		normal.x = viewProjection[0][3] - viewProjection[0][2];
		normal.y = viewProjection[1][3] - viewProjection[1][2];
		normal.z = viewProjection[2][3] - viewProjection[2][2];
		distance = viewProjection[3][3] - viewProjection[3][2];

		norm = normal.getNorm();
		normal /= norm;
		distance /= norm;

		m_planes[planeIndex].set(normal, -distance);

		updatePositiveVertexIndices();
	}

	template <typename T>
	bool Frustum<T>::contains(const Vector3<T>& point) const noexcept
	{
		for (const auto& plane : m_planes)
		{
			if (plane.getSignedDistance(point) < 0)
				return false;
		}

		return true;
	}

	template <typename T>
	bool Frustum<T>::contains(const AABB<T>& aabb) const noexcept
	{
		// http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-testing-boxes-ii/
		for (size_t i = 0; i < m_planes.size(); i++)
		{
			const auto& plane = m_planes[i];
			const auto& indices = m_aabbPositiveVertexIndices[i];
			const auto values = reinterpret_cast<const T*>(&aabb.min);

			// See updatePositiveVertexIndices() for details on this optimization
			Vector3<T> positiveVertex;
			positiveVertex.x = values[indices.x];
			positiveVertex.y = values[indices.y];
			positiveVertex.z = values[indices.z];

			if (plane.getSignedDistance(positiveVertex) < 0)
				return false;
		}

		return true;
	}

	template <typename T>
	const std::array<Plane<T>, 6>& Frustum<T>::getPlanes() const noexcept
	{
		return m_planes;
	}

	template <typename T>
	void Frustum<T>::updatePositiveVertexIndices()
	{
		// We will consider the AABB to be an array of floats :
		// [min.x, min.y, min.z, max.x, max.y, max.z]
		// Here we precompute positive vertex indices for each plane from its normal
		// So when we test multiple AABBs with this frustum, we already have the correct positive vertex
		for (size_t i = 0; i < m_planes.size(); i++)
		{
			const auto& normal = m_planes[i].getNormal();
			auto& indices = m_aabbPositiveVertexIndices[i];

			indices = { 0, 1, 2 };

			if (normal.x >= static_cast<T>(0))
				indices.x = 3;

			if (normal.y >= static_cast<T>(0))
				indices.y = 4;

			if (normal.z >= static_cast<T>(0))
				indices.z = 5;
		}
	}
}

#endif
