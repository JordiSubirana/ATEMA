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

#include <array>

namespace at
{
	namespace detail
	{
		template <size_t N, typename T>
		struct VectorBase
		{
			std::array<T, N> data;
		};

		template <typename T>
		struct VectorBase<2, T>
		{
			union
			{
				std::array<T, 2> data;
				struct { T x, y; };
			};
		};

		template <typename T>
		struct VectorBase<3, T>
		{
			union
			{
				std::array<T, 3> data;
				struct { T x, y, z; };
			};
		};

		template <typename T>
		struct VectorBase<4, T>
		{
			union
			{
				std::array<T, 4> data;
				struct { T x, y, z, w; };
			};
		};
	}
	
	template <size_t N, typename T>
	struct Vector : detail::VectorBase<N, T>
	{
		Vector();
		Vector(const Vector& other);
		Vector(Vector&& other) noexcept;
		template <typename...Args>
		Vector(Args...args);
		~Vector() noexcept;

		Vector<N, T>& normalize() noexcept;
		T getNorm() const noexcept;
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
			return at::Hasher<at::DefaultHashFunction<std::size_t>>::hash(object);
		}
	};
}

#include <Atema/Math/Vector.inl>

#endif