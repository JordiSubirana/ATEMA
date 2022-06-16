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

#ifndef ATEMA_MATH_AABB_HPP
#define ATEMA_MATH_AABB_HPP

#include <Atema/Math/Config.hpp>
#include <Atema/Math/Vector.hpp>
#include <Atema/Math/Matrix.hpp>

#include <vector>

namespace at
{
	template <typename T>
	class AABB
	{
	public:
		AABB();
		AABB(const AABB& other);
		AABB(AABB&& other) noexcept;
		AABB(const Vector3<T>& point);
		AABB(const Vector3<T>& min, const Vector3<T>& max);
		AABB(const std::vector<Vector3<T>>& points);
		~AABB();

		void extend(const Vector3<T>& point);
		void extend(const std::vector<Vector3<T>>& points);
		void extend(const AABB& aabb);

		bool contains(const Vector3<T>& point) const noexcept;
		bool contains(const AABB<T>& aabb) const noexcept;

		Vector3<T> getCenter() const noexcept;

		AABB& operator=(const AABB& other);
		AABB& operator=(AABB&& other) noexcept;

		Vector3<T> min;
		Vector3<T> max;
	};

	using AABBi = AABB<int>;
	using AABBu = AABB<unsigned>;
	using AABBf = AABB<float>;
	using AABBd = AABB<double>;

	template <typename T>
	AABB<T> operator*(const Matrix3<T>& matrix, const AABB<T>& aabb);

	template <typename T>
	AABB<T> operator*(const Matrix4<T>& matrix, const AABB<T>& aabb);
}

#include <Atema/Math/AABB.inl>

#endif
