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

#ifndef ATEMA_MATH_SPHERE_HPP
#define ATEMA_MATH_SPHERE_HPP

#include <Atema/Math/Config.hpp>
#include <Atema/Math/Vector.hpp>

namespace at
{
	template <typename T>
	class AABB;

	template <typename T>
	class Sphere
	{
	public:
		Sphere();
		Sphere(const Vector3<T>& center, T radius);
		Sphere(const Sphere& other) = default;
		Sphere(Sphere&& other) noexcept = default;
		~Sphere() = default;

		void set(const Vector3<T>& center, T radius);

		bool contains(const Vector3<T>& point) const noexcept;
		bool contains(const AABB<T>& aabb) const noexcept;
		bool contains(const Sphere<T>& sphere) const noexcept;

		AABB<T> getAABB() const noexcept;

		Sphere& operator=(const Sphere& other) = default;
		Sphere& operator=(Sphere&& other) noexcept = default;

		Vector3<T> center;
		T radius;
	};

	using Spherei = Sphere<int>;
	using Sphereu = Sphere<unsigned>;
	using Spheref = Sphere<float>;
	using Sphered = Sphere<double>;
}

#include <Atema/Math/Sphere.inl>

#endif