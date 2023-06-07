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

#ifndef ATEMA_MATH_SPHERE_INL
#define ATEMA_MATH_SPHERE_INL

#include <Atema/Math/Config.hpp>
#include <Atema/Math/Sphere.hpp>
#include <Atema/Math/AABB.hpp>
#include <Atema/Math/Math.hpp>

namespace at
{
	template<typename T>
	Sphere<T>::Sphere() :
		center(0, 0, 0),
		radius(0)
	{
	}

	template<typename T>
	Sphere<T>::Sphere(const Vector3<T>& center, T radius) :
		center(center),
		radius(radius)
	{
	}

	template<typename T>
	void Sphere<T>::set(const Vector3<T>& center, T radius)
	{
		this->center = center;
		this->radius = radius;
	}

	template<typename T>
	bool Sphere<T>::contains(const Vector3<T>& point) const noexcept
	{
		return (point - center).getSquaredNorm() <= radius * radius;
	}

	template<typename T>
	bool Sphere<T>::contains(const AABB<T>& aabb) const noexcept
	{
		// https://stackoverflow.com/questions/28343716/sphere-intersection-test-of-aabb
		T distanceMax = static_cast<T>(0);

		for (size_t i = 0; i < 3; i++)
		{
			const auto distMin = Math::squared(center[i] - aabb.min[i]);
			const auto distMax = Math::squared(center[i] - aabb.max[i]);

			distanceMax += std::max(distMin, distMax);
		}

		return distanceMax <= radius * radius;
	}

	template<typename T>
	bool Sphere<T>::contains(const Sphere<T>& sphere) const noexcept
	{
		const auto centerDistance = (center - sphere.center).getNorm();
		
		return (centerDistance + sphere.Radius) <= radius;
	}

	template<typename T>
	AABB<T> Sphere<T>::getAABB() const noexcept
	{
		const T halfRadius = radius / static_cast<T>(2);
		const Vector3<T> halfVector(halfRadius, halfRadius, halfRadius);
		
		return AABB<T>(center - halfVector, center + halfVector);
	}
}

#endif