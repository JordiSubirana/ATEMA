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

#ifndef ATEMA_MATH_PLANE_HPP
#define ATEMA_MATH_PLANE_HPP

#include <Atema/Math/Config.hpp>
#include <Atema/Math/Vector.hpp>

namespace at
{
	template <typename T>
	class Plane
	{
	public:
		Plane();
		Plane(const Plane& other) = default;
		Plane(Plane&& other) noexcept = default;
		Plane(const Vector3<T>& point1, const Vector3<T>& point2, const Vector3<T>& point3);
		Plane(const Vector3<T>& normal, const Vector3<T>& point);
		Plane(const Vector3<T>& normal, T distanceToOrigin);
		~Plane() = default;

		void set(const Vector3<T>& point1, const Vector3<T>& point2, const Vector3<T>& point3);
		void set(const Vector3<T>& normal, const Vector3<T>& point);
		void set(const Vector3<T>& normal, T distanceToOrigin);

		const Vector3<T>& getNormal() const;
		T getDistanceToOrigin() const;

		T getDistance(const Vector3<T>& point) const;
		T getSignedDistance(const Vector3<T>& point) const;
		Vector3<T> getProjection(const Vector3<T>& point) const;

		Plane& operator=(const Plane& other) = default;
		Plane& operator=(Plane&& other) noexcept = default;
		
	private:
		Vector3<T> m_normal;
		T m_distance;
	};

	using Planef = Plane<float>;
	using Planed = Plane<double>;
}

#include <Atema/Math/Plane.inl>

#endif
