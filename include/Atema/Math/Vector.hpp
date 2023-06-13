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

#ifndef ATEMA_MATH_VECTOR_HPP
#define ATEMA_MATH_VECTOR_HPP

#include <Atema/Core/Hash.hpp>
#include <Atema/Math/Math.hpp>

#include <array>

#define ATEMA_VECTOR_SWIZZLE_FUNC_2(a, b) Vector<2, T> a##b() const noexcept { return Vector<2, T>(a, b); }
#define ATEMA_VECTOR_SWIZZLE_FUNC_3(a, b, c) Vector<3, T> a##b##c() const noexcept { return Vector<3, T>(a, b, c); }
#define ATEMA_VECTOR_SWIZZLE_FUNC_4(a, b, c, d) Vector<4, T> a##b##c##d() const noexcept { return Vector<4, T>(a, b, c, d); }
#define ATEMA_VECTOR_SWIZZLE_FUNCS_1(a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_2(a, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_3(a, a, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, a, a, a)
#define ATEMA_VECTOR_SWIZZLE_FUNCS_2(a, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_2(a, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_2(b, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_3(a, a, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_3(a, b, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_3(b, b, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_3(b, a, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, a, a, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, a, b, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, a, b, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, b, a, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, b, a, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, b, b, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, b, b, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, a, a, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, a, a, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, a, b, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, a, b, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, b, a, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, b, a, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, b, b, a)
#define ATEMA_VECTOR_SWIZZLE_FUNCS_3(a, b, c) \
	ATEMA_VECTOR_SWIZZLE_FUNC_3(a, b, c) \
	ATEMA_VECTOR_SWIZZLE_FUNC_3(a, c, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_3(b, a, c) \
	ATEMA_VECTOR_SWIZZLE_FUNC_3(b, c, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_3(c, a, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_3(c, b, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, a, b, c) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, a, c, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, b, a, c) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, b, b, c) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, b, c, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, b, c, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, b, c, c) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, c, a, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, c, b, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, c, b, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, c, b, c) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, c, c, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, a, a, c) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, a, b, c) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, a, c, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, a, c, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, a, c, c) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, b, a, c) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, b, c, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, c, a, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, c, a, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, c, a, c) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, c, b, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, c, c, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(c, a, a, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(c, a, b, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(c, a, b, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(c, a, b, c) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(c, a, c, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(c, b, a, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(c, b, a, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(c, b, a, c) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(c, b, b, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(c, b, c, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(c, c, a, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(c, c, b, a)
#define ATEMA_VECTOR_SWIZZLE_FUNCS_4(a, b, c, d) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, b, c, d) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, b, d, c) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, c, b, d) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, c, d, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, d, b, c) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(a, d, c, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, a, c, d) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, a, d, c) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, c, a, d) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, c, d, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, d, a, c) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(b, d, c, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(c, b, a, d) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(c, b, d, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(c, a, b, d) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(c, a, d, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(c, d, b, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(c, d, a, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(d, b, c, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(d, b, a, c) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(d, c, b, a) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(d, c, a, b) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(d, a, b, c) \
	ATEMA_VECTOR_SWIZZLE_FUNC_4(d, a, c, b)



namespace at
{
	template <size_t N, typename T>
	struct Vector;
	
	namespace detail
	{
		template <size_t N, typename T>
		struct VectorBase
		{
			std::array<T, N> data;

			void set(T value);
		};

		template <typename T>
		struct VectorBase<2, T>
		{
			union
			{
				struct { T x, y; };
				std::array<T, 2> data;
			};

			void set(T value);
			void set(T x, T y);
			template <typename U>
			void set(const Vector<2, U>& xy);

			ATEMA_VECTOR_SWIZZLE_FUNCS_1(x)
			ATEMA_VECTOR_SWIZZLE_FUNCS_1(y)
			ATEMA_VECTOR_SWIZZLE_FUNCS_2(x, y)
		};

		template <typename T>
		struct VectorBase<3, T>
		{
			union
			{
				struct { T x, y, z; };
				std::array<T, 3> data;
			};

			void set(T value);
			void set(T x, T y, T z);
			void set(T x, const Vector<2, T>& yz);
			void set(const Vector<2, T>& xy, T z);
			template <typename U>
			void set(const Vector<3, U>& xyz);

			ATEMA_VECTOR_SWIZZLE_FUNCS_1(x)
			ATEMA_VECTOR_SWIZZLE_FUNCS_1(y)
			ATEMA_VECTOR_SWIZZLE_FUNCS_1(z)
			ATEMA_VECTOR_SWIZZLE_FUNCS_2(x, y)
			ATEMA_VECTOR_SWIZZLE_FUNCS_2(y, z)
			ATEMA_VECTOR_SWIZZLE_FUNCS_2(z, x)
			ATEMA_VECTOR_SWIZZLE_FUNCS_3(x, y, z)
		};

		template <typename T>
		struct VectorBase<4, T>
		{
			union
			{
				struct { T x, y, z, w; };
				std::array<T, 4> data;
			};

			void set(T value);
			void set(T x, T y, T z, T w);
			void set(const Vector<2, T>& xy, const Vector<2, T>& zw);
			void set(T x, T y, const Vector<2, T>& zw);
			void set(T x, const Vector<2, T>& yz, T w);
			void set(const Vector<2, T>& xy, T z, T w);
			void set(const Vector<3, T>& xyz, T w);
			void set(T x, const Vector<3, T>& yzw);
			template <typename U>
			void set(const Vector<4, U>& xyzw);

			ATEMA_VECTOR_SWIZZLE_FUNCS_1(x)
			ATEMA_VECTOR_SWIZZLE_FUNCS_1(y)
			ATEMA_VECTOR_SWIZZLE_FUNCS_1(z)
			ATEMA_VECTOR_SWIZZLE_FUNCS_1(w)
			ATEMA_VECTOR_SWIZZLE_FUNCS_2(x, y)
			ATEMA_VECTOR_SWIZZLE_FUNCS_2(y, z)
			ATEMA_VECTOR_SWIZZLE_FUNCS_2(z, w)
			ATEMA_VECTOR_SWIZZLE_FUNCS_2(w, x)
			ATEMA_VECTOR_SWIZZLE_FUNCS_2(z, x)
			ATEMA_VECTOR_SWIZZLE_FUNCS_2(w, y)
			ATEMA_VECTOR_SWIZZLE_FUNCS_3(x, y, z)
			ATEMA_VECTOR_SWIZZLE_FUNCS_3(y, z, w)
			ATEMA_VECTOR_SWIZZLE_FUNCS_3(z, w, x)
			ATEMA_VECTOR_SWIZZLE_FUNCS_3(w, x, y)
			ATEMA_VECTOR_SWIZZLE_FUNCS_4(x, y, z, w)
		};
	}
	
	template <size_t N, typename T>
	struct Vector : detail::VectorBase<N, T>
	{
		Vector();
		Vector(const Vector& other);
		Vector(Vector&& other) noexcept;
		template <typename...Args>
		Vector(Args&&... args);
		~Vector() noexcept;

		Vector<N, T>& normalize() noexcept;
		T getNorm() const noexcept;
		T getSquaredNorm() const noexcept;
		Vector<N, T> getNormalized() const noexcept;

		Vector<N, T> operator +(const Vector<N, T>& arg) const;
		Vector<N, T> operator -(const Vector<N, T>& arg) const;
		Vector<N, T> operator *(const Vector<N, T>& arg) const;
		Vector<N, T> operator /(const Vector<N, T>& arg) const;
		Vector<N, T> operator +(T arg) const;
		Vector<N, T> operator -(T arg) const;
		Vector<N, T> operator *(T arg) const;
		Vector<N, T> operator /(T arg) const;

		Vector<N, T>& operator +=(const Vector<N, T>& arg);
		Vector<N, T>& operator -=(const Vector<N, T>& arg);
		Vector<N, T>& operator *=(const Vector<N, T>& arg);
		Vector<N, T>& operator /=(const Vector<N, T>& arg);
		Vector<N, T>& operator +=(T arg);
		Vector<N, T>& operator -=(T arg);
		Vector<N, T>& operator *=(T arg);
		Vector<N, T>& operator /=(T arg);

		Vector<N, T>& operator ++();
		Vector<N, T> operator ++(int);
		Vector<N, T>& operator --();
		Vector<N, T> operator --(int);

		Vector<N, T> operator+() const;
		Vector<N, T> operator-() const;

		Vector& operator=(const Vector& other);
		Vector& operator=(Vector&& other) noexcept;

		bool equals(const Vector<N, T>& other, T epsilon = Math::Epsilon<T>) const;

		bool operator==(const Vector<N, T>& other) const;
		bool operator!=(const Vector<N, T>& other) const;

		T& operator[](size_t index);
		const T& operator[](size_t index) const;

		T dot(const Vector<N, T>& other) const;

		T angle(const Vector<N, T>& other) const;
	};

	template <typename T>
	using Vector2 = Vector<2, T>;
	template <typename T>
	using Vector3 = Vector<3, T>;
	template <typename T>
	using Vector4 = Vector<4, T>;

	using Vector2i = Vector2<int>;
	using Vector2u = Vector2<unsigned>;
	using Vector2f = Vector2<float>;
	using Vector2d = Vector2<double>;

	using Vector3i = Vector3<int>;
	using Vector3u = Vector3<unsigned>;
	using Vector3f = Vector3<float>;
	using Vector3d = Vector3<double>;

	using Vector4i = Vector4<int>;
	using Vector4u = Vector4<unsigned>;
	using Vector4f = Vector4<float>;
	using Vector4d = Vector4<double>;

	template <typename T>
	Vector3<T> cross(const Vector2<T>& v1, const Vector2<T>& v2)
	{
		T z = v1.x * v2.y - v1.y * v2.x;

		return (Vector3<T>(0, 0, z));
	}

	template <typename T>
	Vector3<T> cross(const Vector3<T>& v1, const Vector3<T>& v2)
	{
		Vector3<T> tmp;

		tmp.x = v1.y * v2.z - v1.z * v2.y;
		tmp.y = v1.z * v2.x - v1.x * v2.z;
		tmp.z = v1.x * v2.y - v1.y * v2.x;

		return (tmp);
	}
}

namespace std
{
	template <size_t N, typename T>
	struct hash<at::Vector<N, T>>
	{
		std::size_t operator()(const at::Vector<N, T>& object) const
		{
			return at::DefaultStdHasher::hash(object);
		}
	};
}

#include <Atema/Math/Vector.inl>

#endif