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

#ifndef ATEMA_MATH_MATRIX_INL
#define ATEMA_MATH_MATRIX_INL

#include <Atema/Math/Matrix.hpp>
#include <Atema/Core/Error.hpp>

namespace at
{
	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>::Matrix()
	{
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			this->m_data[i] = static_cast<T>(0);
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>::Matrix(T arg) :
		Matrix()
	{
		size_t min = (COL < ROW ? COL : ROW);

		for (size_t i = 0; i < min; i++)
			this->m_columns[i][i] = arg;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>::Matrix(const Matrix<COL, ROW, T>& arg)
	{
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			this->m_data[i] = arg.m_data[i];
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>::~Matrix() noexcept
	{

	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<ROW, COL, T> Matrix<COL, ROW, T>::createIdentity()
	{
		return Matrix<ROW, COL, T>(1);
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<ROW, COL, T> Matrix<COL, ROW, T>::getTransposed() const noexcept
	{
		Matrix<ROW, COL, T> tmp;

		for (size_t c = 0; c < COL; c++)
			for (size_t r = 0; r < ROW; r++)
				tmp[r][c] = this->m_columns[c][r];

		return tmp;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T> Matrix<COL, ROW, T>::operator +(const Matrix<COL, ROW, T>& arg) const
	{
		Matrix<COL, ROW, T> tmp(*this);
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			tmp.m_data[i] += arg.m_data[i];

		return tmp;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T> Matrix<COL, ROW, T>::operator -(const Matrix<COL, ROW, T>& arg) const
	{
		Matrix<COL, ROW, T> tmp(*this);
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			tmp.m_data[i] -= arg.m_data[i];

		return tmp;
	}

	template <size_t COL, size_t ROW, typename T>
	template <size_t OTHER_COL>
	Matrix<OTHER_COL, ROW, T> Matrix<COL, ROW, T>::operator*(const Matrix<OTHER_COL, COL, T>& arg) const
	{
		Matrix<OTHER_COL, ROW, T> tmp;

		for (size_t tmpCol = 0; tmpCol < OTHER_COL; tmpCol++)
		{
			for (size_t row = 0; row < ROW; row++)
			{
				for (size_t col = 0; col < COL; col++)
					tmp[tmpCol][row] += this->m_columns[col][row] * arg[tmpCol][col];
			}
		}

		return tmp;
	}

	template <size_t COL, size_t ROW, typename T>
	Vector<ROW, T> Matrix<COL, ROW, T>::operator *(const Vector<COL, T>& arg) const
	{
		Vector<ROW, T> tmp;

		for (size_t t_r = 0; t_r < ROW; t_r++)
		{
			for (size_t t_c = 0; t_c < COL; t_c++)
			{
				tmp[t_r] += this->m_columns[t_c][t_r] * arg[t_c];
			}
		}

		return tmp;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T> Matrix<COL, ROW, T>::operator +(T arg) const
	{
		Matrix<COL, ROW, T> tmp(*this);
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			tmp.m_data[i] += arg;

		return tmp;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T> Matrix<COL, ROW, T>::operator -(T arg) const
	{
		Matrix<COL, ROW, T> tmp(*this);
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			tmp.m_data[i] -= arg;

		return tmp;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T> Matrix<COL, ROW, T>::operator *(T arg) const
	{
		Matrix<COL, ROW, T> tmp(*this);
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			tmp.m_data[i] *= arg;

		return tmp;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T> Matrix<COL, ROW, T>::operator /(T arg) const
	{
		Matrix<COL, ROW, T> tmp(*this);
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			tmp.m_data[i] /= arg;

		return tmp;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>& Matrix<COL, ROW, T>::operator +=(const Matrix<COL, ROW, T>& arg)
	{
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			this->m_data[i] += arg.m_data[i];

		return *this;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>& Matrix<COL, ROW, T>::operator -=(const Matrix<COL, ROW, T>& arg)
	{
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			this->m_data[i] -= arg.m_data[i];

		return *this;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>& Matrix<COL, ROW, T>::operator +=(T arg)
	{
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			this->m_data[i] += arg;

		return *this;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>& Matrix<COL, ROW, T>::operator -=(T arg)
	{
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			this->m_data[i] -= arg;

		return *this;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>& Matrix<COL, ROW, T>::operator *=(T arg)
	{
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			this->m_data[i] *= arg;

		return *this;
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>& Matrix<COL, ROW, T>::operator /=(T arg)
	{
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			this->m_data[i] /= arg;

		return *this;
	}

	template <size_t COL, size_t ROW, typename T>
	Vector<ROW, T>& Matrix<COL, ROW, T>::operator[](size_t index)
	{
		if (index >= COL)
			ATEMA_ERROR("Index is out of range.");

		return this->m_columns[index];
	}

	template <size_t COL, size_t ROW, typename T>
	const Vector<ROW, T>& Matrix<COL, ROW, T>::operator[](size_t index) const
	{
		if (index >= COL)
			ATEMA_ERROR("Index is out of range.");

		return this->m_columns[index];
	}

	template <size_t COL, size_t ROW, typename T>
	Matrix<COL, ROW, T>& Matrix<COL, ROW, T>::operator=(const Matrix<COL, ROW, T>& arg)
	{
		size_t size = COL*ROW;

		for (size_t i = 0; i < size; i++)
			this->m_data[i] = arg.m_data[i];

		return *this;
	}

	template <size_t COL, size_t ROW, typename T>
	T* Matrix<COL, ROW, T>::get()
	{
		return this->m_data;
	}

	template <size_t COL, size_t ROW, typename T>
	const T* Matrix<COL, ROW, T>::get() const
	{
		return this->m_data;
	}

	//----- Matrix3f -----//
	template <typename T>
	Matrix2<T>::Matrix2() : Matrix<2, 2, T>()
	{
	}

	template <typename T>
	Matrix2<T>::Matrix2(T arg) : Matrix<2, 2, T>(arg)
	{
	}

	template <typename T>
	Matrix2<T>::Matrix2(const Matrix<2, 2, T>& arg) :Matrix<2, 2, T>(arg)
	{
	}

	template <typename T>
	Matrix2<T>::~Matrix2()
	{
	}

	template <typename T>
	Matrix2<T> Matrix2<T>::createInverse(const Matrix2<T>& m)
	{
		T oneOverDeterminant = static_cast<T>(1) / (m[0][0] * m[1][1] - m[0][1] * m[1][0]);

		Matrix2<T> inverse;
		inverse[0][0] = +m[1][1] * oneOverDeterminant;
		inverse[1][0] = -m[1][0] * oneOverDeterminant;
		inverse[0][1] = -m[0][1] * oneOverDeterminant;
		inverse[1][1] = +m[0][0] * oneOverDeterminant;

		return inverse;
	}

	template <typename T>
	Matrix2<T> Matrix2<T>::createInverse() const
	{
		return createInverse(*this);
	}

	template <typename T>
	Matrix2<T>& Matrix2<T>::inverse()
	{
		std::swap(*this, createInverse());

		return *this;
	}

	//----- Matrix3f -----//
	template <typename T>
	Matrix3<T>::Matrix3() : Matrix<3, 3, T>()
	{
	}

	template <typename T>
	Matrix3<T>::Matrix3(T arg) : Matrix<3, 3, T>(arg)
	{
	}

	template <typename T>
	Matrix3<T>::Matrix3(const Matrix<3, 3, T>& arg) : Matrix<3, 3, T>(arg)
	{
	}

	template <typename T>
	Matrix3<T>::~Matrix3()
	{
	}

	template <typename T>
	Matrix3<T> Matrix3<T>::createRotation(const Vector3<T>& angles)
	{
		Matrix3<T> m;

		Vector3<T> c(angles);
		for (auto& val : c.data)
			val = std::cos(val);

		Vector3<T> s(angles);
		for (auto& val : s.data)
			val = std::sin(val);

		m[0] = {c.y * c.z, c.y * s.z, -s.y};
		m[1] = {-c.x * s.z + s.x * s.y * c.z, c.x * c.z + s.x * s.y * s.z, s.x * c.y};
		m[2] = {s.x * s.z + c.x * s.y * c.z, -s.x * c.z + c.x * s.y * s.z, c.x * c.y};

		return m;
	}

	template <typename T>
	Matrix3<T> Matrix3<T>::createInverse(const Matrix3<T>& m)
	{
		T oneOverDeterminant = static_cast<T>(1) / (
			+ m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2])
			- m[1][0] * (m[0][1] * m[2][2] - m[2][1] * m[0][2])
			+ m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2]));

		Matrix3<T> inverse;
		inverse[0][0] = +(m[1][1] * m[2][2] - m[2][1] * m[1][2]) * oneOverDeterminant;
		inverse[1][0] = -(m[1][0] * m[2][2] - m[2][0] * m[1][2]) * oneOverDeterminant;
		inverse[2][0] = +(m[1][0] * m[2][1] - m[2][0] * m[1][1]) * oneOverDeterminant;
		inverse[0][1] = -(m[0][1] * m[2][2] - m[2][1] * m[0][2]) * oneOverDeterminant;
		inverse[1][1] = +(m[0][0] * m[2][2] - m[2][0] * m[0][2]) * oneOverDeterminant;
		inverse[2][1] = -(m[0][0] * m[2][1] - m[2][0] * m[0][1]) * oneOverDeterminant;
		inverse[0][2] = +(m[0][1] * m[1][2] - m[1][1] * m[0][2]) * oneOverDeterminant;
		inverse[1][2] = -(m[0][0] * m[1][2] - m[1][0] * m[0][2]) * oneOverDeterminant;
		inverse[2][2] = +(m[0][0] * m[1][1] - m[1][0] * m[0][1]) * oneOverDeterminant;

		return inverse;
	}

	template <typename T>
	Matrix3<T> Matrix3<T>::createInverse() const
	{
		return createInverse(*this);
	}

	template <typename T>
	Matrix3<T>& Matrix3<T>::inverse()
	{
		std::swap(*this, createInverse());

		return *this;
	}

	//----- Matrix4f -----//
	template <typename T>
	Matrix4<T>::Matrix4() : Matrix<4, 4, T>()
	{
	}

	template <typename T>
	Matrix4<T>::Matrix4(T arg) : Matrix<4, 4, T>(arg)
	{
	}

	template <typename T>
	Matrix4<T>::Matrix4(const Matrix<4, 4, T>& arg) : Matrix<4, 4, T>(arg)
	{
	}

	template <typename T>
	Matrix4<T>::~Matrix4()
	{
	}

	template <typename T>
	Matrix4<T> Matrix4<T>::createTranslation(const Vector3<T>& offset)
	{
		Matrix4<T> m = createIdentity();

		m[3].x = offset.x;
		m[3].y = offset.y;
		m[3].z = offset.z;

		return m;
	}

	template <typename T>
	Matrix4<T> Matrix4<T>::createRotation(const Vector3<T>& angles)
	{
		Matrix4<T> m;

		Vector3<T> c(angles);
		for (auto& val : c.data)
			val = std::cos(val);

		Vector3<T> s(angles);
		for (auto& val : s.data)
			val = std::sin(val);

		m[0] = {c.y * c.z, c.y * s.z, -s.y, 0};
		m[1] = {-c.x * s.z + s.x * s.y * c.z, c.x * c.z + s.x * s.y * s.z, s.x * c.y, 0};
		m[2] = {s.x * s.z + c.x * s.y * c.z, -s.x * c.z + c.x * s.y * s.z, c.x * c.y, 0};
		m[3] = {0, 0, 0, 1};

		return m;
	}

	template <typename T>
	Matrix4<T> Matrix4<T>::createScale(const Vector3<T>& scales)
	{
		Matrix4<T> m;

		m[0][0] = scales.x;
		m[1][1] = scales.y;
		m[2][2] = scales.z;
		m[3][3] = static_cast<T>(1);

		return m;
	}

	template <typename T>
	Matrix4<T> Matrix4<T>::createLookAt(const Vector3<T>& eye, const Vector3<T>& center, const Vector3<T>& up)
	{
		Vector3<T> f = (center - eye);
		f.normalize();
		Vector3<T> u = (up);
		u.normalize();
		Vector3<T> s = (cross(f, u));
		s.normalize();
		u = cross(s, f);

		Matrix4<T> mat = createIdentity();

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

	template <typename T>
	Matrix4<T> Matrix4<T>::createPerspective(T fov, T ratio, T zNear, T zFar)
	{
		Matrix4<T> mat;

		T range = zNear * std::tan(fov / static_cast<T>(2));
		T left = -range;
		T right = range;
		T bottom = -range / ratio;
		T top = range / ratio;

		mat[0][0] = (static_cast<T>(2) * zNear) / (right - left);
		mat[0][1] = static_cast<T>(0);
		mat[0][2] = static_cast<T>(0);
		mat[0][3] = static_cast<T>(0);

		mat[1][0] = static_cast<T>(0);
		mat[1][1] = (static_cast<T>(2) * zNear) / (top - bottom);
		mat[1][2] = static_cast<T>(0);
		mat[1][3] = static_cast<T>(0);

		mat[2][0] = static_cast<T>(0);
		mat[2][1] = static_cast<T>(0);
		mat[2][2] = -(zFar + zNear) / (zFar - zNear);
		mat[2][3] = static_cast<T>(-1);

		mat[3][0] = static_cast<T>(0);
		mat[3][1] = static_cast<T>(0);
		mat[3][2] = -zFar * zNear * static_cast<T>(2) / (zFar - zNear);
		mat[3][3] = static_cast<T>(1);

		return (mat);
	}

	template <typename T>
	Matrix4<T> Matrix4<T>::createOrtho(T left, T right, T bottom, T top, T zNear, T zFar)
	{
		Matrix4<T> mat(1.0f);

		mat[0][0] = static_cast<T>(2) / (right - left);
		mat[0][1] = static_cast<T>(0);
		mat[0][2] = static_cast<T>(0);
		mat[0][3] = static_cast<T>(0);

		mat[1][0] = static_cast<T>(0);
		mat[1][1] = static_cast<T>(2) / (top - bottom);
		mat[1][2] = static_cast<T>(0);
		mat[1][3] = static_cast<T>(0);

		mat[2][0] = static_cast<T>(0);
		mat[2][1] = static_cast<T>(0);
		mat[2][2] = static_cast<T>(1) / (zNear - zFar);
		mat[2][3] = static_cast<T>(0);

		mat[3][0] = (left + right) / (left - right);
		mat[3][1] = (bottom + top) / (bottom - top);
		mat[3][2] = zNear / (zNear - zFar);
		mat[3][3] = static_cast<T>(1);

		return mat;
	}

	template <typename T>
	Matrix4<T> Matrix4<T>::createInverse(const Matrix4<T>& m)
	{
		T coef00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
		T coef02 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
		T coef03 = m[1][2] * m[2][3] - m[2][2] * m[1][3];

		T coef04 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
		T coef06 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
		T coef07 = m[1][1] * m[2][3] - m[2][1] * m[1][3];

		T coef08 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
		T coef10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
		T coef11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];

		T coef12 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
		T coef14 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
		T coef15 = m[1][0] * m[2][3] - m[2][0] * m[1][3];

		T coef16 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
		T coef18 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
		T coef19 = m[1][0] * m[2][2] - m[2][0] * m[1][2];

		T coef20 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
		T coef22 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
		T coef23 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

		Vector4<T> fac0(coef00, coef00, coef02, coef03);
		Vector4<T> fac1(coef04, coef04, coef06, coef07);
		Vector4<T> fac2(coef08, coef08, coef10, coef11);
		Vector4<T> fac3(coef12, coef12, coef14, coef15);
		Vector4<T> fac4(coef16, coef16, coef18, coef19);
		Vector4<T> fac5(coef20, coef20, coef22, coef23);

		Vector4<T> vec0(m[1][0], m[0][0], m[0][0], m[0][0]);
		Vector4<T> vec1(m[1][1], m[0][1], m[0][1], m[0][1]);
		Vector4<T> vec2(m[1][2], m[0][2], m[0][2], m[0][2]);
		Vector4<T> vec3(m[1][3], m[0][3], m[0][3], m[0][3]);

		Vector4<T> inv0(vec1 * fac0 - vec2 * fac1 + vec3 * fac2);
		Vector4<T> inv1(vec0 * fac0 - vec2 * fac3 + vec3 * fac4);
		Vector4<T> inv2(vec0 * fac1 - vec1 * fac3 + vec3 * fac5);
		Vector4<T> inv3(vec0 * fac2 - vec1 * fac4 + vec2 * fac5);

		Vector4<T> signA(+1, -1, +1, -1);
		Vector4<T> signB(-1, +1, -1, +1);

		Matrix4<T> inverse;

		inverse[0] = inv0 * signA;
		inverse[1] = inv1 * signB;
		inverse[2] = inv2 * signA;
		inverse[3] = inv3 * signB;

		Vector4<T> row0(inverse[0][0], inverse[1][0], inverse[2][0], inverse[3][0]);

		Vector4<T> dot0(m[0] * row0);
		T dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);

		T oneOverDeterminant = static_cast<T>(1) / dot1;

		return inverse * oneOverDeterminant;
	}

	template <typename T>
	Matrix4<T> Matrix4<T>::createInverse() const
	{
		return createInverse(*this);
	}

	template <typename T>
	Matrix4<T>& Matrix4<T>::inverse()
	{
		std::swap(*this, createInverse());

		return *this;
	}
}

template <typename T>
at::Vector3<T> operator*(const at::Matrix4<T>& mat, const at::Vector3<T>& vec)
{
	at::Vector4<T> vec_tmp;
	at::Vector3<T> tmp;

	vec_tmp.x = vec.x;
	vec_tmp.y = vec.y;
	vec_tmp.z = vec.z;
	vec_tmp.w = 1;

	vec_tmp = mat * vec_tmp;

	tmp.x = vec_tmp.x;
	tmp.y = vec_tmp.y;
	tmp.z = vec_tmp.z;

	return (tmp);
}

#endif