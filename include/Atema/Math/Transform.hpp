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

#ifndef ATEMA_MATH_TRANSFORM_HPP
#define ATEMA_MATH_TRANSFORM_HPP

#include <Atema/Math/Config.hpp>
#include <Atema/Math/Matrix.hpp>
#include <Atema/Math/Vector.hpp>

namespace at
{
	class ATEMA_MATH_API Transform
	{
	public:
		Transform();
		Transform(const Vector3f& translation, const Vector3f& rotation);
		Transform(const Vector3f& translation, const Vector3f& rotation, const Vector3f& scale);
		~Transform();

		Transform& translate(const Vector3f& translation);
		Transform& rotate(const Vector3f& rotation);
		Transform& scale(const Vector3f& scale);

		void set(const Vector3f& translation, const Vector3f& rotation);
		void set(const Vector3f& translation, const Vector3f& rotation, const Vector3f& scale);
		
		void setTranslation(const Vector3f& translation);
		void setRotation(const Vector3f& rotation);
		void setScale(const Vector3f& scale);

		const Vector3f& getTranslation() const noexcept;
		const Vector3f& getRotation() const noexcept;
		const Vector3f& getScale() const noexcept;

		const Matrix4f& getMatrix() const noexcept;

	private:
		Vector3f m_translation;
		Vector3f m_rotation;
		Vector3f m_scale;

		Matrix4f m_transform;
	};
}

#endif
