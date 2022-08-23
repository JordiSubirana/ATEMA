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

#ifndef ATEMA_MATH_FRUSTUM_HPP
#define ATEMA_MATH_FRUSTUM_HPP

#include <Atema/Math/Config.hpp>
#include <Atema/Math/Plane.hpp>
#include <Atema/Math/Matrix.hpp>
#include <Atema/Math/AABB.hpp>

namespace at
{
	template <typename T>
	class Frustum
	{
	public:
		Frustum() = default;
		Frustum(const Frustum& other) = default;
		Frustum(Frustum&& other) noexcept = default;
		Frustum(const std::array<Plane<T>, 6>& planes);
		Frustum(const Matrix4<T>& viewProjection);
		~Frustum() = default;

		void set(const std::array<Plane<T>, 6>& planes);
		void set(const Matrix4<T>& viewProjection);

		bool contains(const Vector3<T>& point) const noexcept;
		bool contains(const AABB<T>& aabb) const noexcept;

		const std::array<Plane<T>, 6>& getPlanes() const noexcept;

		Frustum& operator=(const Frustum& other) = default;
		Frustum& operator=(Frustum&& other) noexcept = default;
		
	private:
		std::array<Plane<T>, 6> m_planes;
	};

	using Frustumf = Frustum<float>;
	using Frustumd = Frustum<double>;
}

#include <Atema/Math/Frustum.inl>

#endif
