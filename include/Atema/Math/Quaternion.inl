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

#ifndef ATEMA_MATH_QUATERNION_INL
#define ATEMA_MATH_QUATERNION_INL

#include <Atema/Math/Quaternion.hpp>

namespace at
{
	template <typename T>
	Quaternion<T>::Quaternion() :
		Quaternion(static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0))
	{
	}

	template <typename T>
	Quaternion<T>::Quaternion(T w, T x, T y, T z) :
		w(w),
		x(x),
		y(y),
		z(z)
	{
	}

	template <typename T>
	Quaternion<T>::Quaternion(const Vector3<T>& eulerAngles)
	{
		const T cx = std::cos(eulerAngles.x / static_cast<T>(2));
		const T sx = std::sin(eulerAngles.x / static_cast<T>(2));

		const T cy = std::cos(eulerAngles.y / static_cast<T>(2));
		const T sy = std::sin(eulerAngles.y / static_cast<T>(2));

		const T cz = std::cos(eulerAngles.z / static_cast<T>(2));
		const T sz = std::sin(eulerAngles.z / static_cast<T>(2));

		w = cx * cy * cz + sx * sy * sz;
		x = sx * cy * cz - cx * sy * sz;
		y = cx * sy * cz + sx * cy * sz;
		z = cx * cy * sz - sx * sy * cz;
	}

	template <typename T>
	Quaternion<T>::Quaternion(T angle, const Vector3<T>& axis)
	{
		const T halfAngleSin = std::sin(angle / static_cast<T>(2));

		w = std::cos(angle / static_cast<T>(2));
		x = halfAngleSin * axis.x;
		y = halfAngleSin * axis.y;
		z = halfAngleSin * axis.z;

		normalize();
	}

	template <typename T>
	Quaternion<T>::Quaternion(const Vector3<T>& src, const Vector3<T>& dst)
	{
		// https://math.stackexchange.com/questions/4520571/how-to-get-a-rotation-quaternion-from-two-vectors

		const Vector3<T> srcNormalized = src.getNormalized();
		const Vector3<T> dstNormalized = dst.getNormalized();
		const Vector3<T> rotationAxis = cross(srcNormalized, dstNormalized).normalize();

		const T dot = srcNormalized.dot(dstNormalized);

		const T halfAngleCos = std::sqrt((static_cast<T>(1) + dot) / static_cast<T>(2));
		const T halfAngleSin = std::sqrt((static_cast<T>(1) - dot) / static_cast<T>(2));

		w = halfAngleCos;
		x = halfAngleSin * rotationAxis.x;
		y = halfAngleSin * rotationAxis.y;
		z = halfAngleSin * rotationAxis.z;
	}

	template <typename T>
	Quaternion<T> Quaternion<T>::createIdentity()
	{
		return Quaternion(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0));
	}

	template <typename T>
	Quaternion<T> Quaternion<T>::lerp(const Quaternion& q1, const Quaternion& q2, T coefficient)
	{
		Quaternion q;

		q.w = Math::lerp(q1.w, q2.w, coefficient);
		q.x = Math::lerp(q1.x, q2.x, coefficient);
		q.y = Math::lerp(q1.y, q2.y, coefficient);
		q.z = Math::lerp(q1.z, q2.z, coefficient);

		q.normalize();

		return q;
	}

	template <typename T>
	Quaternion<T> Quaternion<T>::slerp(const Quaternion& q1, const Quaternion& q2, T coefficient)
	{
		T cosTheta = q1.dot(q2);

		Quaternion<T> dst;

		// Take the shortest path
		if (cosTheta < static_cast<T>(0))
		{
			dst = -q2;
			cosTheta = -cosTheta;
		}
		else
		{
			dst = q2;
		}

		// Spherical interpolation
		T t1, t2;
		if (cosTheta < static_cast<T>(1) - Math::Epsilon<T>)
		{
			const T theta = std::acos(cosTheta);
			const T oneOverSinTheta = static_cast<T>(1) / std::sin(theta);

			t1 = std::sin((static_cast<T>(1) - coefficient) * theta) * oneOverSinTheta;
			t2 = std::sin(coefficient * theta) * oneOverSinTheta;
		}
		// Fallback on linear interpolation when dividing by zero
		else
		{
			t1 = static_cast<T>(1) - coefficient;
			t2 = coefficient;
		}

		return q1 * t1 + dst * t2;
	}

	template <typename T>
	T Quaternion<T>::dot(const Quaternion& other) const
	{
		return w * other.w + x * other.x + y * other.y + z * other.z;
	}

	template <typename T>
	T Quaternion<T>::getNorm() const
	{
		return std::sqrt(getSquaredNorm());
	}

	template <typename T>
	T Quaternion<T>::getSquaredNorm() const
	{
		return dot(*this);
	}

	template <typename T>
	Quaternion<T>& Quaternion<T>::normalize()
	{
		const T norm = getNorm();

		ATEMA_ASSERT(!Math::equals(norm, static_cast<T>(0)), "Quaternion norm must be different from 0");

		const T inverseNorm = static_cast<T>(1) / norm;

		w *= inverseNorm;
		x *= inverseNorm;
		y *= inverseNorm;
		z *= inverseNorm;

		return *this;
	}

	template <typename T>
	Quaternion<T>& Quaternion<T>::conjugate()
	{
		x *= static_cast<T>(-1);
		y *= static_cast<T>(-1);
		z *= static_cast<T>(-1);

		return *this;
	}

	template <typename T>
	Quaternion<T>& Quaternion<T>::inverse()
	{
		const T norm = getNorm();

		if (!Math::equals(norm, static_cast<T>(0)))
		{
			const T inverseNorm = static_cast<T>(1) / norm;

			w *= inverseNorm;
			x *= -inverseNorm;
			y *= -inverseNorm;
			z *= -inverseNorm;
		}

		return *this;
	}

	template <typename T>
	Quaternion<T> Quaternion<T>::getNormalized() const
	{
		Quaternion q(*this);
		q.normalize();

		return q;
	}

	template <typename T>
	Quaternion<T> Quaternion<T>::getConjugate() const
	{
		Quaternion q(*this);
		q.conjugate();

		return q;
	}

	template <typename T>
	Quaternion<T> Quaternion<T>::getInverse() const
	{
		Quaternion q(*this);
		q.inverse();

		return q;
	}

	template <typename T>
	Vector3<T> Quaternion<T>::toEulerAngles() const
	{
		const T T1 = static_cast<T>(1);
		const T T2 = static_cast<T>(2);

		Vector3<T> eulerAngles;

		const T sxcy = T2 * (w * x + y * z);
		const T cxcy = T1 - T2 * (x * x + y * y);
		eulerAngles.x = std::atan2(sxcy, cxcy);

		const T sy = std::sqrt(T1 + T2 * (w * y - x * z));
		const T cy = std::sqrt(T1 - T2 * (w * y - x * z));
		eulerAngles.y = T2 * std::atan2(sy, cy) - Math::Pi<T> / T2;

		const T szcy = T2 * (w * z + x * y);
		const T czcy = T1 - T2 * (y * y + z * z);
		eulerAngles.z = std::atan2(szcy, czcy);

		return eulerAngles;
	}

	template <typename T>
	bool Quaternion<T>::equals(const Quaternion& other, T epsilon) const
	{
		if (!Math::equals(w, other.w, epsilon))
			return false;

		if (!Math::equals(x, other.x, epsilon))
			return false;

		if (!Math::equals(y, other.y, epsilon))
			return false;

		if (!Math::equals(z, other.z, epsilon))
			return false;

		return true;
	}

	template <typename T>
	Vector3<T> Quaternion<T>::rotateVector(const Vector3<T>& vector)
	{
		Quaternion result = operator*(Quaternion(0, vector.x, vector.y, vector.z) * getConjugate());

		return Vector3<T>(result.x, result.y, result.z);
	}

	template <typename T>
	Quaternion<T> Quaternion<T>::operator+(const Quaternion& other) const
	{
		Quaternion q;

		q.w = w + other.w;
		q.x = x + other.x;
		q.y = y + other.y;
		q.z = z + other.z;

		return q;
	}

	template <typename T>
	Quaternion<T> Quaternion<T>::operator-(const Quaternion& other) const
	{
		Quaternion q;

		q.w = w - other.w;
		q.x = x - other.x;
		q.y = y - other.y;
		q.z = z - other.z;

		return q;
	}

	template <typename T>
	Quaternion<T> Quaternion<T>::operator*(const Quaternion& other) const
	{
		Quaternion q;

		q.w = w * other.w - x * other.x - y * other.y - z * other.z;
		q.x = w * other.x + x * other.w + y * other.z - z * other.y;
		q.y = w * other.y - x * other.z + y * other.w + z * other.x;
		q.z = w * other.z + x * other.y - y * other.x + z * other.w;

		return q;
	}

	template <typename T>
	Quaternion<T> Quaternion<T>::operator*(T value) const
	{
		Quaternion q;

		q.w = value * w;
		q.x = value * x;
		q.y = value * y;
		q.z = value * z;

		return q;
	}

	template <typename T>
	Quaternion<T> Quaternion<T>::operator/(const Quaternion& other) const
	{
		return operator*(other.getInverse());
	}

	template <typename T>
	Quaternion<T>& Quaternion<T>::operator+=(const Quaternion& other)
	{
		return operator=(operator+(other));
	}

	template <typename T>
	Quaternion<T>& Quaternion<T>::operator-=(const Quaternion& other)
	{
		return operator=(operator-(other));
	}

	template <typename T>
	Quaternion<T>& Quaternion<T>::operator*=(const Quaternion& other)
	{
		return operator=(operator*(other));
	}

	template <typename T>
	Quaternion<T>& Quaternion<T>::operator*=(T value)
	{
		return operator=(operator*(value));
	}

	template <typename T>
	Quaternion<T>& Quaternion<T>::operator/=(const Quaternion& other)
	{
		return operator=(operator/(other));
	}

	template <typename T>
	Quaternion<T> Quaternion<T>::operator+() const
	{
		return *this;
	}

	template <typename T>
	Quaternion<T> Quaternion<T>::operator-() const
	{
		return Quaternion(-w, -x, -y, -z);
	}
}

#endif
