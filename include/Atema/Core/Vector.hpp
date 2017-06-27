/*
	Copyright 2017 Jordi SUBIRANA

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

#ifndef ATEMA_CORE_VECTOR_HPP
#define ATEMA_CORE_VECTOR_HPP

namespace at
{
	namespace detail
	{
		template <size_t N, typename T>
		struct VectorBase
		{
			T data[N];
		};

		template <typename T>
		struct VectorBase<2, T>
		{
			union
			{
				T data[2];
				struct { T x, y; };
			};
		};

		template <typename T>
		struct VectorBase<3, T>
		{
			union
			{
				T data[3];
				struct { T x, y, z; };
			};
		};

		template <typename T>
		struct VectorBase<4, T>
		{
			union
			{
				T data[4];
				struct { T x, y, z, w; };
			};
		};
	}
	
	template <size_t N, typename T>
	struct Vector : detail::VectorBase<N, T>
	{
		Vector();

		template <typename...Args>
		Vector(Args...args)
		{
			auto tmp = { static_cast<T>(args)... };

			size_t size = tmp.size();
			if (size == 1)
			{
				auto el = *tmp.begin();

				for (size_t i = 0; i < N; i++)
					data[i] = el;
			}
			else
			{
				size_t i = 0;
				for (auto t : tmp)
				{
					if (i >= N)
						break;

					data[i++] = t;
				}
				for (; i < N; i++)
				{
					data[i] = static_cast<T>(0);
				}
			}
		}
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

		T& operator[](size_t index);
		const T& operator[](size_t index) const;
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
}

#include <Atema/Core/Vector.inl>

#endif