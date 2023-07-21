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

	namespace
	{
		template <typename T>
		struct Line
		{
			Vector3<T> point;
			Vector3<T> direction;
		};

		template <typename T>
		Line<T> getIntersection(const Plane<T>& p1, const Plane<T>& p2)
		{
			Line<T> line;

			line.direction = cross(p1.getNormal(), p2.getNormal());

			line.point = -cross(line.direction, p2.getNormal()) * p1.getDistanceToOrigin() - cross(p1.getNormal(), line.direction) * p2.getDistanceToOrigin();
			line.point /= line.direction.getSquaredNorm();

			line.direction.normalize();

			return line;
		}

		template <typename T>
		Vector3<T> getIntersection(const Plane<T>& plane, const Line<T>& line)
		{
			const auto dot = plane.getNormal().dot(line.direction);

			ATEMA_ASSERT(std::fabs(dot) >= Math::Epsilon<T>, "Plane and Line don't intersect");

			T t = plane.getSignedDistance(line.point) / dot;

			return line.point - line.direction * t;
		}
	}

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

		updateVertexIndices();

		updateCorners();
	}

	template <typename T>
	void Frustum<T>::set(const Matrix4<T>& viewProjection)
	{
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

		m_planes[static_cast<size_t>(FrustumPlane::Left)].set(normal, -distance);

		// Right
		normal.x = viewProjection[0][3] - viewProjection[0][0];
		normal.y = viewProjection[1][3] - viewProjection[1][0];
		normal.z = viewProjection[2][3] - viewProjection[2][0];
		distance = viewProjection[3][3] - viewProjection[3][0];

		norm = normal.getNorm();
		normal /= norm;
		distance /= norm;

		m_planes[static_cast<size_t>(FrustumPlane::Right)].set(normal, -distance);

		// Bottom
		normal.x = viewProjection[0][3] + viewProjection[0][1];
		normal.y = viewProjection[1][3] + viewProjection[1][1];
		normal.z = viewProjection[2][3] + viewProjection[2][1];
		distance = viewProjection[3][3] + viewProjection[3][1];

		norm = normal.getNorm();
		normal /= norm;
		distance /= norm;

		m_planes[static_cast<size_t>(FrustumPlane::Bottom)].set(normal, -distance);

		// Top
		normal.x = viewProjection[0][3] - viewProjection[0][1];
		normal.y = viewProjection[1][3] - viewProjection[1][1];
		normal.z = viewProjection[2][3] - viewProjection[2][1];
		distance = viewProjection[3][3] - viewProjection[3][1];

		norm = normal.getNorm();
		normal /= norm;
		distance /= norm;

		m_planes[static_cast<size_t>(FrustumPlane::Top)].set(normal, -distance);

		// Near
#if ATEMA_CLIPSPACE_Z == ATEMA_CLIPSPACE_Z_ZERO_TO_ONE // Clip space Z : [0;1]
		normal.x = viewProjection[0][2];
		normal.y = viewProjection[1][2];
		normal.z = viewProjection[2][2];
		distance = viewProjection[3][2];
#elif ATEMA_CLIPSPACE_Z == ATEMA_CLIPSPACE_Z_MINUS_ONE_TO_ONE // Clip space Z : [-1;1]
		normal.x = viewProjection[0][3] + viewProjection[0][2];
		normal.y = viewProjection[1][3] + viewProjection[1][2];
		normal.z = viewProjection[2][3] + viewProjection[2][2];
		distance = viewProjection[3][3] + viewProjection[3][2];
#else
#error Invalid clipspace Z range
#endif

		norm = normal.getNorm();
		normal /= norm;
		distance /= norm;

		m_planes[static_cast<size_t>(FrustumPlane::Near)].set(normal, -distance);

		// Far
		normal.x = viewProjection[0][3] - viewProjection[0][2];
		normal.y = viewProjection[1][3] - viewProjection[1][2];
		normal.z = viewProjection[2][3] - viewProjection[2][2];
		distance = viewProjection[3][3] - viewProjection[3][2];

		norm = normal.getNorm();
		normal /= norm;
		distance /= norm;

		m_planes[static_cast<size_t>(FrustumPlane::Far)].set(normal, -distance);

		updateVertexIndices();

		updateCorners();
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
		return getIntersectionType(aabb) == IntersectionType::Inside;
	}

    template <typename T>
    bool Frustum<T>::contains(const Sphere<T>& sphere) const noexcept
    {
		for (const auto& plane : m_planes)
		{
			if (plane.getSignedDistance(sphere.center) < sphere.radius)
				return false;
		}

		return true;
    }

    template <typename T>
    bool Frustum<T>::intersects(const AABB<T>& aabb) const noexcept
    {
		// http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-testing-boxes-ii/
		for (size_t i = 0; i < m_planes.size(); i++)
		{
			const auto& plane = m_planes[i];
			const auto& positiveIndices = m_aabbPositiveVertexIndices[i];
			const auto values = reinterpret_cast<const T*>(&aabb.min);

			// See updatePositiveVertexIndices() for details on this optimization
			const Vector3<T> positiveVertex(values[positiveIndices.x], values[positiveIndices.y], values[positiveIndices.z]);

			if (plane.getSignedDistance(positiveVertex) < static_cast<T>(0))
				return false;
		}

		return true;
    }

    template <typename T>
    bool Frustum<T>::intersects(const Sphere<T>& sphere) const noexcept
    {
		for (const auto& plane : m_planes)
		{
			if (plane.getSignedDistance(sphere.center) < -sphere.radius)
				return false;
		}

		return true;
    }

	template<typename T>
	IntersectionType Frustum<T>::getIntersectionType(const AABB<T>& aabb) const noexcept
	{
		IntersectionType intersectionType = IntersectionType::Inside;
		
		// http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-testing-boxes-ii/
		for (size_t i = 0; i < m_planes.size(); i++)
		{
			const auto& plane = m_planes[i];
			const auto& positiveIndices = m_aabbPositiveVertexIndices[i];
			const auto& negativeIndices = m_aabbNegativeVertexIndices[i];
			const auto values = reinterpret_cast<const T*>(&aabb.min);

			// See updatePositiveVertexIndices() for details on this optimization
			const Vector3<T> positiveVertex(values[positiveIndices.x], values[positiveIndices.y], values[positiveIndices.z]);
			const Vector3<T> negativeVertex(values[negativeIndices.x], values[negativeIndices.y], values[negativeIndices.z]);

			if (plane.getSignedDistance(positiveVertex) < static_cast<T>(0))
				return IntersectionType::Outside;
			else if (plane.getSignedDistance(negativeVertex) < static_cast<T>(0))
				intersectionType = IntersectionType::Intersection;
		}

		return intersectionType;
	}

	template <typename T>
	IntersectionType Frustum<T>::getIntersectionType(const Sphere<T>& sphere) const noexcept
	{
		IntersectionType intersectionType = IntersectionType::Inside;

		for (const auto& plane : m_planes)
		{
			const auto signedDistance = plane.getSignedDistance(sphere.center);
			
			if (signedDistance < -sphere.radius)
				return IntersectionType::Outside;
			else if (signedDistance < sphere.radius)
				intersectionType = IntersectionType::Intersection;
		}

		return intersectionType;
	}

	template <typename T>
	const std::array<Plane<T>, 6>& Frustum<T>::getPlanes() const noexcept
	{
		return m_planes;
	}

	template <typename T>
	const std::array<Vector3<T>, 8>& Frustum<T>::getCorners() const noexcept
	{
		return m_corners;
	}

	template <typename T>
	const Plane<T>& Frustum<T>::getPlane(FrustumPlane plane) const
	{
		return m_planes[static_cast<size_t>(plane)];
	}

	template <typename T>
	const Vector3<T>& Frustum<T>::getCorner(FrustumCorner corner) const
	{
		return m_corners[static_cast<size_t>(corner)];
	}

	template <typename T>
	void Frustum<T>::updateCorners()
	{
		auto lb = getIntersection(m_planes[static_cast<size_t>(FrustumPlane::Left)],	m_planes[static_cast<size_t>(FrustumPlane::Bottom)]);
		auto rb = getIntersection(m_planes[static_cast<size_t>(FrustumPlane::Right)],	m_planes[static_cast<size_t>(FrustumPlane::Bottom)]);
		auto lt = getIntersection(m_planes[static_cast<size_t>(FrustumPlane::Left)],	m_planes[static_cast<size_t>(FrustumPlane::Top)]);
		auto rt = getIntersection(m_planes[static_cast<size_t>(FrustumPlane::Right)],	m_planes[static_cast<size_t>(FrustumPlane::Top)]);

		m_corners[static_cast<size_t>(FrustumCorner::NearBottomLeft)] =		getIntersection(m_planes[static_cast<size_t>(FrustumPlane::Near)], lb);
		m_corners[static_cast<size_t>(FrustumCorner::NearBottomRight)] =	getIntersection(m_planes[static_cast<size_t>(FrustumPlane::Near)], rb);
		m_corners[static_cast<size_t>(FrustumCorner::NearTopLeft)] =		getIntersection(m_planes[static_cast<size_t>(FrustumPlane::Near)], lt);
		m_corners[static_cast<size_t>(FrustumCorner::NearTopRight)] =		getIntersection(m_planes[static_cast<size_t>(FrustumPlane::Near)], rt);
		m_corners[static_cast<size_t>(FrustumCorner::FarBottomLeft)] =		getIntersection(m_planes[static_cast<size_t>(FrustumPlane::Far)], lb);
		m_corners[static_cast<size_t>(FrustumCorner::FarBottomRight)] =		getIntersection(m_planes[static_cast<size_t>(FrustumPlane::Far)], rb);
		m_corners[static_cast<size_t>(FrustumCorner::FarTopLeft)] =			getIntersection(m_planes[static_cast<size_t>(FrustumPlane::Far)], lt);
		m_corners[static_cast<size_t>(FrustumCorner::FarTopRight)] =		getIntersection(m_planes[static_cast<size_t>(FrustumPlane::Far)], rt);
	}

	template <typename T>
	void Frustum<T>::updateVertexIndices()
	{
		// We will consider the AABB to be an array of floats :
		// [min.x, min.y, min.z, max.x, max.y, max.z]
		// Here we precompute positive vertex indices for each plane from its normal
		// So when we test multiple AABBs with this frustum, we already have the correct positive vertex
		for (size_t planeIndex = 0; planeIndex < m_planes.size(); planeIndex++)
		{
			const auto& normal = m_planes[planeIndex].getNormal();
			auto& positiveIndices = m_aabbPositiveVertexIndices[planeIndex];
			auto& negativeIndices = m_aabbNegativeVertexIndices[planeIndex];

			positiveIndices = { 0, 1, 2 };
			negativeIndices = { 3, 4, 5 };

			for (size_t i = 0; i < 3; i++)
			{
				if (normal.data[i] >= static_cast<T>(0))
					std::swap(positiveIndices.data[i], negativeIndices.data[i]);
			}
		}
	}
}

#endif
