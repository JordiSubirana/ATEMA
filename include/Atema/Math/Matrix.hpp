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

#ifndef ATEMA_MATH_MATRIX_HPP
#define ATEMA_MATH_MATRIX_HPP

#include <Atema/Math/Vector.hpp>

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

		static Matrix<ROW, COL, T> createIdentity();

		Matrix<ROW, COL, T> getTransposed() const noexcept;

		Matrix<COL, ROW, T> operator +(const Matrix<COL, ROW, T>& arg) const;
		Matrix<COL, ROW, T> operator -(const Matrix<COL, ROW, T>& arg) const;
		template <size_t OTHER_COL>
		Matrix<OTHER_COL, ROW, T> operator *(const Matrix<OTHER_COL, COL, T>& arg) const;
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

	// Specializations
	template <typename T>
	class Matrix2 : public Matrix<2, 2, T>
	{
	public:
		Matrix2();
		Matrix2(T arg);
		Matrix2(const Matrix<2, 2, T>& arg);
		~Matrix2();

		static Matrix2<T> createInverse(const Matrix2<T>& m);

		Matrix2<T> createInverse() const;

		Matrix2<T>& inverse();
	};

	template <typename T>
	class Matrix3 : public Matrix<3, 3, T>
	{
	public:
		Matrix3();
		Matrix3(T arg);
		Matrix3(const Matrix<3, 3, T>& arg);
		~Matrix3();

		static Matrix3<T> createRotation(const Vector3<T>& angles);

		static Matrix3<T> createInverse(const Matrix3<T>& m);

		Matrix3<T> createInverse() const;

		Matrix3<T>& inverse();
	};

	template <typename T>
	class Matrix4 : public Matrix<4, 4, T>
	{
	public:
		Matrix4();
		Matrix4(T arg);
		Matrix4(const Matrix<4, 4, T>& arg);
		~Matrix4();

		static Matrix4<T> createTranslation(const Vector3<T>& offset);

		static Matrix4<T> createRotation(const Vector3<T>& angles);
		
		static Matrix4<T> createScale(const Vector3<T>& scales);

		static Matrix4<T> createLookAt(const Vector3<T>& eye, const Vector3<T>& center, const Vector3<T>& up);

		static Matrix4<T> createPerspective(T fov, T ratio, T z_near, T z_far);

		static Matrix4<T> createOrtho(T left, T right, T bottom, T top, T z_near, T z_far);

		static Matrix4<T> createInverse(const Matrix4<T>& m);

		Matrix4<T> createInverse() const;

		Matrix4<T>& inverse();
	};

	// Aliases
	template <typename T> using Matrix2x2 = Matrix2<T>;
	template <typename T> using Matrix2x3 = Matrix<2, 3, T>;
	template <typename T> using Matrix2x4 = Matrix<2, 4, T>;

	template <typename T> using Matrix3x2 = Matrix<3, 2, T>;
	template <typename T> using Matrix3x3 = Matrix3<T>;
	template <typename T> using Matrix3x4 = Matrix<3, 4, T>;
	
	template <typename T> using Matrix4x2 = Matrix<4, 2, T>;
	template <typename T> using Matrix4x3 = Matrix<4, 3, T>;
	template <typename T> using Matrix4x4 = Matrix4<T>;

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
}

template <typename T>
at::Vector3<T> operator*(const at::Matrix4<T>& mat, const at::Vector3<T>& vec);

#include <Atema/Math/Matrix.inl>



#endif