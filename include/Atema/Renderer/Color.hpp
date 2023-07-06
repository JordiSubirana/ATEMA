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

#ifndef ATEMA_RENDERER_COLOR_HPP
#define ATEMA_RENDERER_COLOR_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Math/Vector.hpp>

namespace at
{
	class ATEMA_RENDERER_API Color
	{
	public:
		constexpr Color() noexcept;
		constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) noexcept;
		constexpr Color(float r, float g, float b, float a = 1.0f) noexcept;
		constexpr Color(const Color& other) noexcept = default;
		Color(Color&& other) noexcept = default;
		~Color() = default;

		static const Color Black;
		static const Color White;
		static const Color Gray;
		static const Color Red;
		static const Color Green;
		static const Color Blue;
		static const Color Cyan;
		static const Color Magenta;
		static const Color Yellow;
		static const Color Orange;
		static const Color Purple;

		static Color fromCMYK(float cyan, float magenta, float yellow, float black, float alpha = 1.0f);
		static Color fromHSL(float hue, float saturation, float lightness, float alpha = 1.0f);
		static Color fromHSV(float hue, float saturation, float value, float alpha = 1.0f);
		
		void toCMYK(float *cyan, float *magenta, float *yellow, float *black) const;
		void toHSL(float* hue, float* saturation, float* lightness) const;
		void toHSV(float* hue, float* saturation, float* value) const;
		Vector3f toVector3f() const;
		Vector4f toVector4f() const;

		Color& operator=(const Color& other) noexcept = default;
		Color& operator=(Color&& other) noexcept = default;
		
		float r, g, b, a;
	};
}

#endif
