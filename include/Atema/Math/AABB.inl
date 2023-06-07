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

#ifndef ATEMA_MATH_AABB_INL
#define ATEMA_MATH_AABB_INL

#include <Atema/Math/AABB.hpp>
#include <Atema/Math/Math.hpp>
#include <Atema/Math/Sphere.hpp>

namespace at
{
	template <typename T>
	AABB<T>::AABB() :
		min(std::numeric_limits<T>::max()),
		max(std::numeric_limits<T>::lowest())
	{
	}

	template <typename T>
	AABB<T>::AABB(const AABB& other)
	{
		operator=(other);
	}

	template <typename T>
	AABB<T>::AABB(AABB&& other) noexcept
	{
		operator=(std::move(other));
	}

	template <typename T>
	AABB<T>::AABB(const Vector3<T>& point) :
		min(point),
		max(point)
	{
	}

	template <typename T>
	AABB<T>::AABB(const Vector3<T>& min, const Vector3<T>& max) :
		AABB()
	{
		extend(min);
		extend(max);
	}

	template <typename T>
	AABB<T>::AABB(const std::vector<Vector3<T>>& points) :
		AABB()
	{
		extend(points);
	}

	template <typename T>
	AABB<T>::~AABB()
	{
	}

	template <typename T>
	void AABB<T>::extend(const Vector3<T>& point)
	{
		min.x = std::min(min.x, point.x);
		min.y = std::min(min.y, point.y);
		min.z = std::min(min.z, point.z);

		max.x = std::max(max.x, point.x);
		max.y = std::max(max.y, point.y);
		max.z = std::max(max.z, point.z);
	}

	template <typename T>
	void AABB<T>::extend(const std::vector<Vector3<T>>& points)
	{
		for (const auto& point : points)
			extend(point);
	}

	template <typename T>
	void AABB<T>::extend(const AABB& aabb)
	{
		extend(aabb.min);
		extend(aabb.max);
	}

	template <typename T>
	bool AABB<T>::contains(const Vector3<T>& point) const noexcept
	{
		return Math::isInside(point.x, min.x, max.x) && Math::isInside(point.y, min.y, max.y) && Math::isInside(point.z, min.z, max.z);
	}

	template <typename T>
	bool AABB<T>::contains(const AABB<T>& aabb) const noexcept
	{
		return contains(aabb.min) && contains(aabb.max);
	}

	template <typename T>
	Vector3<T> AABB<T>::getCenter() const noexcept
	{
		return (min + max) / static_cast<T>(2);
	}

	template <typename T>
	Vector3<T> AABB<T>::getSize() const noexcept
	{
		return max - min;
	}

	template <typename T>
	Vector3<T> AABB<T>::getPositiveVertex(const Vector3<T>& normal) const noexcept
	{
		Vector3<T> vertex = min;

		if (normal.x >= static_cast<T>(0))
			vertex.x = max.x;

		if (normal.y >= static_cast<T>(0))
			vertex.y = max.y;

		if (normal.z >= static_cast<T>(0))
			vertex.z = max.z;

		return vertex;
	}

	template <typename T>
	Vector3<T> AABB<T>::getNegativeVertex(const Vector3<T>& normal) const noexcept
	{
		Vector3<T> vertex = max;

		if (normal.x >= static_cast<T>(0))
			vertex.x = min.x;

		if (normal.y >= static_cast<T>(0))
			vertex.y = min.y;

		if (normal.z >= static_cast<T>(0))
			vertex.z = min.z;

		return vertex;
	}

	template <typename T>
	Sphere<T> AABB<T>::getBoundingSphere() const noexcept
	{
		Sphere<T> sphere;

		sphere.center = getCenter();
		sphere.radius = (max - min).getNorm() / static_cast<T>(2);

		return sphere;
	}

	template <typename T>
	AABB<T>& AABB<T>::operator=(const AABB& other)
	{
		min = other.min;
		max = other.max;

		return *this;
	}

	template <typename T>
	AABB<T>& AABB<T>::operator=(AABB&& other) noexcept
	{
		min = std::move(other.min);
		max = std::move(other.max);

		return *this;
	}

	template <typename T>
	AABB<T> operator*(const Matrix3<T>& matrix, const AABB<T>& aabb)
	{
		const std::vector<Vector3<T>> corners =
		{
			{ aabb.min.x, aabb.min.y, aabb.min.z },
			{ aabb.min.x, aabb.min.y, aabb.max.z },
			{ aabb.min.x, aabb.max.y, aabb.min.z },
			{ aabb.min.x, aabb.max.y, aabb.max.z },
			{ aabb.max.x, aabb.min.y, aabb.min.z },
			{ aabb.max.x, aabb.min.y, aabb.max.z },
			{ aabb.max.x, aabb.max.y, aabb.min.z },
			{ aabb.max.x, aabb.max.y, aabb.max.z }
		};

		AABB<T> newAABB;

		for (const auto& corner : corners)
			newAABB.extend(matrix * corner);

		return newAABB;
	}

	template <typename T>
	AABB<T> operator*(const Matrix4<T>& matrix, const AABB<T>& aabb)
	{
		AABB<T> newAABB(matrix.transformPosition({ aabb.min.x, aabb.min.y, aabb.min.z }));
		newAABB.extend(matrix.transformPosition({ aabb.min.x, aabb.min.y, aabb.max.z }));
		newAABB.extend(matrix.transformPosition({ aabb.min.x, aabb.max.y, aabb.min.z }));
		newAABB.extend(matrix.transformPosition({ aabb.min.x, aabb.max.y, aabb.max.z }));
		newAABB.extend(matrix.transformPosition({ aabb.max.x, aabb.min.y, aabb.min.z }));
		newAABB.extend(matrix.transformPosition({ aabb.max.x, aabb.min.y, aabb.max.z }));
		newAABB.extend(matrix.transformPosition({ aabb.max.x, aabb.max.y, aabb.min.z }));
		newAABB.extend(matrix.transformPosition({ aabb.max.x, aabb.max.y, aabb.max.z }));

		return newAABB;
	}
}

#endif
