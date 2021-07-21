/*
	Copyright 2021 Jordi SUBIRANA

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

#ifndef ATEMA_CORE_MATRIX_HPP
#define ATEMA_CORE_MATRIX_HPP

#include <Atema/Core/Vector.hpp>

#include <iostream>

namespace at
{
	template <size_t COL, size_t ROW, typename T>
	class Matrix
	{
	public:
		Matrix();
		Matrix(T arg);
		Matrix(const Matrix<COL, ROW, T>& arg);
		~Matrix() noexcept;

		static Matrix<ROW, COL, T> identity();

		Matrix<ROW, COL, T> getTransposed() const noexcept;

		Matrix<COL, ROW, T> operator +(const Matrix<COL, ROW, T>& arg) const;
		Matrix<COL, ROW, T> operator -(const Matrix<COL, ROW, T>& arg) const;
		Matrix<ROW, ROW, T> operator *(const Matrix<ROW, COL, T>& arg) const;
		Vector<ROW, T> operator *(const Vector<COL, T>& arg) const;
		Matrix<COL, ROW, T> operator +(T arg) const;
		Matrix<COL, ROW, T> operator -(T arg) const;
		Matrix<COL, ROW, T> operator *(T arg) const;
		Matrix<COL, ROW, T> operator /(T arg) const;

		Matrix<COL, ROW, T>& operator +=(const Matrix<COL, ROW, T>& arg);
		Matrix<COL, ROW, T>& operator -=(const Matrix<COL, ROW, T>& arg);
		Matrix<COL, ROW, T>& operator +=(T arg);
		Matrix<COL, ROW, T>& operator -=(T arg);
		Matrix<COL, ROW, T>& operator *=(T arg);
		Matrix<COL, ROW, T>& operator /=(T arg);

		Vector<ROW, T>& operator[](size_t index);
		const Vector<ROW, T>& operator[](size_t index) const;

		Matrix<COL, ROW, T>& operator=(const Matrix<COL, ROW, T>& arg);

		T* get();
		const T* get() const;

	private:
		union
		{
			T m_data[COL*ROW];
			Vector<ROW, T> m_columns[COL];
		};
	};

	template <typename T> using Matrix2 = Matrix<2, 2, T>;
	template <typename T> using Matrix2x2 = Matrix<2, 2, T>;
	template <typename T> using Matrix2x3 = Matrix<2, 3, T>;
	template <typename T> using Matrix2x4 = Matrix<2, 4, T>;

	template <typename T> using Matrix3 = Matrix<3, 3, T>;
	template <typename T> using Matrix3x2 = Matrix<3, 2, T>;
	template <typename T> using Matrix3x3 = Matrix<3, 3, T>;
	template <typename T> using Matrix3x4 = Matrix<3, 4, T>;

	template <typename T> using Matrix4 = Matrix<4, 4, T>;
	template <typename T> using Matrix4x2 = Matrix<4, 2, T>;
	template <typename T> using Matrix4x3 = Matrix<4, 3, T>;
	template <typename T> using Matrix4x4 = Matrix<4, 4, T>;

	using Matrix2i = Matrix2<int>;
	using Matrix2x2i = Matrix2x2<int>;
	using Matrix2x3i = Matrix2x3<int>;
	using Matrix2x4i = Matrix2x4<int>;
	using Matrix2u = Matrix2<unsigned>;
	using Matrix2x2u = Matrix2x2<unsigned>;
	using Matrix2x3u = Matrix2x3<unsigned>;
	using Matrix2x4u = Matrix2x4<unsigned>;
	using Matrix2f = Matrix2<float>;
	using Matrix2x2f = Matrix2x2<float>;
	using Matrix2x3f = Matrix2x3<float>;
	using Matrix2x4f = Matrix2x4<float>;
	using Matrix2d = Matrix2<double>;
	using Matrix2x2d = Matrix2x2<double>;
	using Matrix2x3d = Matrix2x3<double>;
	using Matrix2x4d = Matrix2x4<double>;

	using Matrix3i = Matrix3<int>;
	using Matrix3x2i = Matrix3x2<int>;
	using Matrix3x3i = Matrix3x3<int>;
	using Matrix3x4i = Matrix3x4<int>;
	using Matrix3u = Matrix3<unsigned>;
	using Matrix3x2u = Matrix3x2<unsigned>;
	using Matrix3x3u = Matrix3x3<unsigned>;
	using Matrix3x4u = Matrix3x4<unsigned>;
	using Matrix3f = Matrix3<float>;
	using Matrix3x2f = Matrix3x2<float>;
	using Matrix3x3f = Matrix3x3<float>;
	using Matrix3x4f = Matrix3x4<float>;
	using Matrix3d = Matrix3<double>;
	using Matrix3x2d = Matrix3x2<double>;
	using Matrix3x3d = Matrix3x3<double>;
	using Matrix3x4d = Matrix3x4<double>;

	using Matrix4i = Matrix4<int>;
	using Matrix4x2i = Matrix4x2<int>;
	using Matrix4x3i = Matrix4x3<int>;
	using Matrix4x4i = Matrix4x4<int>;
	using Matrix4u = Matrix4<unsigned>;
	using Matrix4x2u = Matrix4x2<unsigned>;
	using Matrix4x3u = Matrix4x3<unsigned>;
	using Matrix4x4u = Matrix4x4<unsigned>;
	using Matrix4f = Matrix4<float>;
	using Matrix4x2f = Matrix4x2<float>;
	using Matrix4x3f = Matrix4x3<float>;
	using Matrix4x4f = Matrix4x4<float>;
	using Matrix4d = Matrix4<double>;
	using Matrix4x2d = Matrix4x2<double>;
	using Matrix4x3d = Matrix4x3<double>;
	using Matrix4x4d = Matrix4x4<double>;

	template <typename T>
	Matrix3<T> inverse(const Matrix3<T>& m)
	{
		T OneOverDeterminant = static_cast<T>(1) / (
			+m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2])
			- m[1][0] * (m[0][1] * m[2][2] - m[2][1] * m[0][2])
			+ m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2]));

		Matrix3<T> Inverse;
		Inverse[0][0] = +(m[1][1] * m[2][2] - m[2][1] * m[1][2]) * OneOverDeterminant;
		Inverse[1][0] = -(m[1][0] * m[2][2] - m[2][0] * m[1][2]) * OneOverDeterminant;
		Inverse[2][0] = +(m[1][0] * m[2][1] - m[2][0] * m[1][1]) * OneOverDeterminant;
		Inverse[0][1] = -(m[0][1] * m[2][2] - m[2][1] * m[0][2]) * OneOverDeterminant;
		Inverse[1][1] = +(m[0][0] * m[2][2] - m[2][0] * m[0][2]) * OneOverDeterminant;
		Inverse[2][1] = -(m[0][0] * m[2][1] - m[2][0] * m[0][1]) * OneOverDeterminant;
		Inverse[0][2] = +(m[0][1] * m[1][2] - m[1][1] * m[0][2]) * OneOverDeterminant;
		Inverse[1][2] = -(m[0][0] * m[1][2] - m[1][0] * m[0][2]) * OneOverDeterminant;
		Inverse[2][2] = +(m[0][0] * m[1][1] - m[1][0] * m[0][1]) * OneOverDeterminant;

		return Inverse;
	}

	template <typename T>
	Matrix4<T> inverse(const Matrix4<T>& m)
	{
		T Coef00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
		T Coef02 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
		T Coef03 = m[1][2] * m[2][3] - m[2][2] * m[1][3];

		T Coef04 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
		T Coef06 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
		T Coef07 = m[1][1] * m[2][3] - m[2][1] * m[1][3];

		T Coef08 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
		T Coef10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
		T Coef11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];

		T Coef12 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
		T Coef14 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
		T Coef15 = m[1][0] * m[2][3] - m[2][0] * m[1][3];

		T Coef16 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
		T Coef18 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
		T Coef19 = m[1][0] * m[2][2] - m[2][0] * m[1][2];

		T Coef20 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
		T Coef22 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
		T Coef23 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

		Vector4<T> Fac0(Coef00, Coef00, Coef02, Coef03);
		Vector4<T> Fac1(Coef04, Coef04, Coef06, Coef07);
		Vector4<T> Fac2(Coef08, Coef08, Coef10, Coef11);
		Vector4<T> Fac3(Coef12, Coef12, Coef14, Coef15);
		Vector4<T> Fac4(Coef16, Coef16, Coef18, Coef19);
		Vector4<T> Fac5(Coef20, Coef20, Coef22, Coef23);

		Vector4<T> Vec0(m[1][0], m[0][0], m[0][0], m[0][0]);
		Vector4<T> Vec1(m[1][1], m[0][1], m[0][1], m[0][1]);
		Vector4<T> Vec2(m[1][2], m[0][2], m[0][2], m[0][2]);
		Vector4<T> Vec3(m[1][3], m[0][3], m[0][3], m[0][3]);

		Vector4<T> Inv0(Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2);
		Vector4<T> Inv1(Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4);
		Vector4<T> Inv2(Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5);
		Vector4<T> Inv3(Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5);

		Vector4<T> SignA(+1, -1, +1, -1);
		Vector4<T> SignB(-1, +1, -1, +1);

		Matrix4<T> Inverse;

		Inverse[0] = Inv0 * SignA;
		Inverse[1] = Inv1 * SignB;
		Inverse[2] = Inv2 * SignA;
		Inverse[3] = Inv3 * SignB;

		Vector4<T> Row0(Inverse[0][0], Inverse[1][0], Inverse[2][0], Inverse[3][0]);

		Vector4<T> Dot0(m[0] * Row0);
		T Dot1 = (Dot0.x + Dot0.y) + (Dot0.z + Dot0.w);

		T OneOverDeterminant = static_cast<T>(1) / Dot1;

		return Inverse * OneOverDeterminant;
	}

	inline Matrix4f translation(const Vector3f& offset)
	{
		Matrix4f m = Matrix4f::identity();

		m[3].x = offset.x;
		m[3].y = offset.y;
		m[3].z = offset.z;

		return m;
	}

	inline Matrix3f rotation(const Vector3f& angles)
	{
		Matrix3f m;

		Vector3f c(angles);
		for (auto& val : c.data)
			val = std::cos(val);

		Vector3f s(angles);
		for (auto& val : s.data)
			val = std::sin(val);

		m[0] = { c.y * c.z, c.y * s.z, -s.y };
		m[1] = { -c.x * s.z + s.x * s.y * c.z, c.x * c.z + s.x * s.y * s.z, s.x * c.y };
		m[2] = { s.x * s.z + c.x * s.y * c.z, -s.x * c.z + c.x * s.y * s.z, c.x * c.y };

		return m;
	}

	inline Matrix4f rotation4f(const Vector3f& angles)
	{
		Matrix4f m;

		Vector3f c(angles);
		for (auto& val : c.data)
			val = std::cos(val);

		Vector3f s(angles);
		for (auto& val : s.data)
			val = std::sin(val);

		m[0] = { c.y * c.z, c.y * s.z, -s.y, 0 };
		m[1] = { -c.x * s.z + s.x * s.y * c.z, c.x * c.z + s.x * s.y * s.z, s.x * c.y, 0 };
		m[2] = { s.x * s.z + c.x * s.y * c.z, -s.x * c.z + c.x * s.y * s.z, c.x * c.y, 0 };
		m[3] = { 0, 0, 0, 1 };

		return m;
	}

	// Transform tools
	inline Matrix4f lookAt(const Vector3f& eye, const Vector3f& center, const Vector3f& up)
	{
		Vector3f f = (center - eye);
		f.normalize();
		Vector3f u = (up);
		u.normalize();
		Vector3f s = (cross(f, u));
		s.normalize();
		u = cross(s, f);

		Matrix4f mat = Matrix4f::identity();

		mat[0][0] = s.x;
		mat[1][0] = s.y;
		mat[2][0] = s.z;
		mat[0][1] = u.x;
		mat[1][1] = u.y;
		mat[2][1] = u.z;
		mat[0][2] = -f.x;
		mat[1][2] = -f.y;
		mat[2][2] = -f.z;
		mat[3][0] = -s.dot(eye);
		mat[3][1] = -u.dot(eye);
		mat[3][2] = f.dot(eye);

		return mat;
	}

	inline Matrix4f perspective(float fov, float ratio, float z_near, float z_far)
	{
		Matrix4f mat;

		float range = z_near * std::tan(fov / 2.0f);
		float left = -range;
		float right = range;
		float bottom = -range / ratio;
		float top = range / ratio;

		mat[0][0] = (2.0f * z_near) / (right - left);
		mat[0][1] = 0.0f;
		mat[0][2] = 0.0f;
		mat[0][3] = 0.0f;

		mat[1][0] = 0.0f;
		mat[1][1] = (2.0f * z_near) / (top - bottom);
		mat[1][2] = 0.0f;
		mat[1][3] = 0.0f;

		mat[2][0] = 0.0f;
		mat[2][1] = 0.0f;
		mat[2][2] = -(z_far + z_near) / (z_far - z_near);
		mat[2][3] = -1.0f;

		mat[3][0] = 0.0f;
		mat[3][1] = 0.0f;
		mat[3][2] = -z_far * z_near * 2.0f / (z_far - z_near);
		mat[3][3] = 1.0f;

		return (mat);
	}

	inline Matrix4f ortho(float left, float right, float bottom, float top, float z_near, float z_far)
	{
		Matrix4f mat;

		mat[0][0] = 2.0f / (right - left);
		mat[0][1] = 0.0f;
		mat[0][2] = 0.0f;
		mat[0][3] = 0.0f;

		mat[1][0] = 0.0f;
		mat[1][1] = 2.0f / (top - bottom);
		mat[1][2] = 0.0f;
		mat[1][3] = 0.0f;

		mat[2][0] = 0.0f;
		mat[2][1] = 0.0f;
		mat[2][2] = -2.0f / (z_far - z_near);
		mat[2][3] = 0.0f;

		mat[3][0] = -(right + left) / (right - left);
		mat[3][1] = -(top + bottom) / (top - bottom);
		mat[3][2] = -(z_far + z_near) / (z_far - z_near);
		mat[3][3] = 1.0f;

		return mat;
	}
}

template <typename T>
at::Vector3<T> operator*(const at::Matrix4<T>& mat, const at::Vector3<T>& vec);

#include <Atema/Core/Matrix.inl>



#endif