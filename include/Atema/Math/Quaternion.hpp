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

#ifndef ATEMA_MATH_QUATERNION_HPP
#define ATEMA_MATH_QUATERNION_HPP

#include <Atema/Math/Config.hpp>
#include <Atema/Math/Vector.hpp>

namespace at
{
	template <typename T>
	class Quaternion
	{
	public:
		Quaternion();
		Quaternion(T w, T x, T y, T z);
		// From Euler angles
		Quaternion(const Vector3<T>& eulerAngles);
		Quaternion(T angle, const Vector3<T>& axis);
		// Quaternion defining the rotation between 2 vectors
		Quaternion(const Vector3<T>& src, const Vector3<T>& dst);
		Quaternion(const Quaternion& other) = default;
		Quaternion(Quaternion&& other) noexcept = default;
		~Quaternion() = default;

		static Quaternion createIdentity();
		// coefficient must be in the range [0,1]
		static Quaternion lerp(const Quaternion& q1, const Quaternion& q2, T coefficient);
		static Quaternion slerp(const Quaternion& q1, const Quaternion& q2, T coefficient);

		T dot(const Quaternion& other) const;

		T getNorm() const;
		T getSquaredNorm() const;

		Quaternion& normalize();
		Quaternion& conjugate();
		Quaternion& inverse();

		Quaternion getNormalized() const;
		Quaternion getConjugate() const;
		Quaternion getInverse() const;

		Vector3<T> toEulerAngles() const;

		bool equals(const Quaternion& other, T epsilon = Math::Epsilon<T>) const;

		// Convenience method applying Q * V * Qconjugate
		Vector3<T> rotateVector(const Vector3<T>& vector) const;

		Quaternion operator+(const Quaternion& other) const;
		Quaternion operator-(const Quaternion& other) const;
		Quaternion operator*(const Quaternion& other) const;
		Quaternion operator*(T value) const;
		Quaternion operator/(const Quaternion& other) const;

		Quaternion& operator+=(const Quaternion& other);
		Quaternion& operator-=(const Quaternion& other);
		Quaternion& operator*=(const Quaternion& other);
		Quaternion& operator*=(T value);
		Quaternion& operator/=(const Quaternion& other);

		Quaternion operator+() const;
		Quaternion operator-() const;
		
		Quaternion& operator=(const Quaternion& other) = default;
		Quaternion& operator=(Quaternion&& other) noexcept = default;

		T w, x, y, z;
	};

	using Quaternionf = Quaternion<float>;
	using Quaterniond = Quaternion<double>;
}

#include <Atema/Math/Quaternion.inl>

#endif
