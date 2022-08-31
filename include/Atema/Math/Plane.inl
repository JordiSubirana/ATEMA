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

#ifndef ATEMA_MATH_PLANE_INL
#define ATEMA_MATH_PLANE_INL

#include <Atema/Math/Plane.hpp>

namespace at
{
	template <typename T>
	Plane<T>::Plane() :
		m_normal(static_cast<T>(0), static_cast<T>(0), static_cast<T>(1)),
		m_distance(static_cast<T>(0))
	{
	}

	template <typename T>
	Plane<T>::Plane(const Vector3<T>& point1, const Vector3<T>& point2, const Vector3<T>& point3)
	{
		set(point1, point2, point3);
	}

	template <typename T>
	Plane<T>::Plane(const Vector3<T>& normal, const Vector3<T>& point)
	{
		set(normal, point);
	}

	template <typename T>
	Plane<T>::Plane(const Vector3<T>& normal, T distanceToOrigin)
	{
		set(normal, distanceToOrigin);
	}

	template <typename T>
	void Plane<T>::set(const Vector3<T>& point1, const Vector3<T>& point2, const Vector3<T>& point3)
	{
		auto normal = cross(point2 - point1, point3 - point1);
		normal.normalize();

		set(normal, point1);
	}

	template <typename T>
	void Plane<T>::set(const Vector3<T>& normal, const Vector3<T>& point)
	{
		set(normal, normal.dot(point));
	}

	template <typename T>
	void Plane<T>::set(const Vector3<T>& normal, T distanceToOrigin)
	{
		m_normal = normal;
		m_distance = distanceToOrigin;
	}

	template <typename T>
	const Vector3<T>& Plane<T>::getNormal() const
	{
		return m_normal;
	}

	template <typename T>
	T Plane<T>::getDistanceToOrigin() const
	{
		return m_distance;
	}

	template <typename T>
	T Plane<T>::getDistance(const Vector3<T>& point) const
	{
		return std::abs(getSignedDistance(point));
	}

	template <typename T>
	T Plane<T>::getSignedDistance(const Vector3<T>& point) const
	{
		return m_normal.dot(point) - m_distance;
	}

	template <typename T>
	Vector3<T> Plane<T>::getProjection(const Vector3<T>& point) const
	{
		return point - m_normal * getSignedDistance(point);
	}
}

#endif
