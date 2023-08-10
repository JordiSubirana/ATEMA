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

#include <Atema/Renderer/Color.hpp>
#include <Atema/Math/Math.hpp>

#include <algorithm>
#include <array>

using namespace at;

const Color Color::Black =		Color(0.0f, 0.0f, 0.0f, 1.0f);
const Color Color::White =		Color(1.0f, 1.0f, 1.0f, 1.0f);
const Color Color::Gray =		Color(0.5f, 0.5f, 0.5f, 1.0f);
const Color Color::Red =		Color(1.0f, 0.0f, 0.0f, 1.0f);
const Color Color::Green =		Color(0.0f, 1.0f, 0.0f, 1.0f);
const Color Color::Blue =		Color(0.0f, 0.0f, 1.0f, 1.0f);
const Color Color::Cyan =		Color(0.0f, 1.0f, 1.0f, 1.0f);
const Color Color::Magenta =	Color(1.0f, 0.0f, 1.0f, 1.0f);
const Color Color::Yellow =		Color(1.0f, 1.0f, 0.0f, 1.0f);
const Color Color::Orange =		Color(1.0f, 0.5f, 0.0f, 1.0f);
const Color Color::Purple =		Color(0.5f, 0.0f, 1.0f, 1.0f);

constexpr Color::Color() noexcept :
	r(0.0f),
	g(0.0f),
	b(0.0f),
	a(1.0f)
{
	
}

constexpr Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept :
	r(static_cast<float>(r) / 255.0f),
	g(static_cast<float>(g) / 255.0f),
	b(static_cast<float>(b) / 255.0f),
	a(static_cast<float>(a) / 255.0f)
{
}

constexpr Color::Color(float r, float g, float b, float a) noexcept :
	r(r),
	g(g),
	b(b),
	a(a)
{
}

constexpr Color::Color(const Vector3f& rgb, float a) noexcept :
	r(rgb.x),
	g(rgb.y),
	b(rgb.z),
	a(a)
{
}

constexpr Color::Color(const Vector4f& rgba) noexcept :
	r(rgba.x),
	g(rgba.y),
	b(rgba.z),
	a(rgba.w)
{
}

Color Color::fromCMYK(float c, float m, float y, float k, float a)
{
	Color color;

	color.r = (1.0f - c) * (1.0f - k);
	color.g = (1.0f - m) * (1.0f - k);
	color.b = (1.0f - y) * (1.0f - k);
	color.a = a;
	
	return color;
}

Color Color::fromHSL(float hue, float saturation, float lightness, float alpha)
{
	Color color;
	
	color.a = alpha;
	
	if (Math::equals(saturation, 0.0f))
	{
		color.r = lightness;
		color.g = lightness;
		color.b = lightness;
	}
	else
	{
		float tmp1;

		if (lightness < 0.5f)
			tmp1 = lightness * (1.0f + saturation);
		else
			tmp1 = lightness + saturation - lightness * saturation;

		float tmp2 = 2.0f - lightness - tmp1;

		hue /= 360.0f;

		std::array<float, 3> tmpChannels; // RGB

		tmpChannels[0] = hue + (1.0f / 3.0f);
		tmpChannels[1] = hue;
		tmpChannels[2] = hue - (1.0f / 3.0f);

		for (auto& channel : tmpChannels)
		{
			if (channel < 0.0f)
				channel += 1.0f;
			else if (channel > 1.0f)
				channel -= 1.0f;

			if ((channel * 6.0f) < 1.0f)
				channel = tmp2 + (tmp1 - tmp2) * channel * 6.0f;
			else if ((channel * 2.0f) < 1.0f)
				channel = tmp1;
			else if ((channel * 3.0f) < 2.0f)
				channel = tmp2 + (tmp1 - tmp2) * (2.0f / 3.0f - channel) * 6.0f;
			else
				channel = tmp2;
		}
		
		color.r = tmpChannels[0];
		color.g = tmpChannels[1];
		color.b = tmpChannels[2];
	}

	return color;
}

Color Color::fromHSV(float hue, float saturation, float value, float alpha)
{
	Color color;

	color.a = alpha;

	if (Math::equals(saturation, 0.0f))
	{
		color.r = value;
		color.g = value;
		color.b = value;
	}
	else
	{
		const float h = (hue / 360.0f) * 6.0f;
		const int i = static_cast<int>(h);

		const float tmp1 = value * (1.0f - saturation);
		const float tmp2 = value * (1.0f - saturation * (h - static_cast<float>(i)));
		const float tmp3 = value * (1.0f - saturation * (1.0f - (h - static_cast<float>(i))));
		
		switch (i)
		{
			case 0:
			{
				color.r = value;
				color.g = tmp3;
				color.b = tmp1;
				break;
			}
			case 1:
			{
				color.r = tmp2;
				color.g = value;
				color.b = tmp1;
				break;
			}
			case 2:
			{
				color.r = tmp1;
				color.g = value;
				color.b = tmp3;
				break;
			}
			case 3:
			{
				color.r = tmp1;
				color.g = tmp2;
				color.b = value;
				break;
			}
			case 4:
			{
				color.r = tmp3;
				color.g = tmp1;
				color.b = value;
				break;
			}
			default:
			{
				color.r = value;
				color.g = tmp1;
				color.b = tmp2;
				break;
			}
		}
	}

	return color;
}

void Color::toCMYK(float* cyan, float* magenta, float* yellow, float* black) const
{
	const auto k = 1.0f - std::max({ r, g, b });

	// Black
	if (Math::equals(k, 1.0f))
	{
		if (cyan)
			*cyan = 0.0f;

		if (magenta)
			*magenta = 0.0f;

		if (yellow)
			*yellow = 0.0f;
	}
	// Non-black
	else
	{
		if (cyan)
			*cyan = (1.0f - r - k) / (1.0f - k);

		if (magenta)
			*magenta = (1.0f - g - k) / (1.0f - k);

		if (yellow)
			*yellow = (1.0f - g - k) / (1.0f - k);
	}

	if (black)
		*black = k;
}

void Color::toHSL(float* hue, float* saturation, float* lightness) const
{
	const auto cmin = std::min({ r, g, b });
	const auto cmax = std::max({ r, g, b });
	
	const auto delta = cmax - cmin;

	const auto l = (cmax + cmin) / 2.0f;

	if (lightness)
		*lightness = l;
	
	if (Math::equals(delta, 0.0f))
	{
		if (hue)
			*hue = 0.0f;

		if (saturation)
			*saturation = 0.0f;
	}
	else
	{
		if (saturation)
		{
			if (l <= 0.5f)
				*saturation = delta / (cmax + cmin);
			else
				*saturation = delta / (2.0f - cmax - cmin);
		}

		if (hue)
		{
			float h;

			if (Math::equals(r, cmax))
				h = (g - b) / delta;
			else if (Math::equals(g, cmax))
				h = 2.0f + (b - r) / delta;
			else
				h = 4.0f + (r - g) / delta;

			h *= 60.0f;

			if (h < 0.0f)
				h += 360.0f;

			*hue = h;
		}
	}
}

void Color::toHSV(float* hue, float* saturation, float* value) const
{
	const auto cmin = std::min({ r, g, b });
	const auto cmax = std::max({ r, g, b });

	const auto delta = cmax - cmin;

	if (value)
		*value = cmax;

	if (Math::equals(delta, 0.0f))
	{
		if (hue)
			*hue = 0.0f;

		if (saturation)
			*saturation = 0.0f;
	}
	else
	{
		if (saturation)
			*saturation = delta / cmax;

		if (hue)
		{
			float h;

			if (Math::equals(r, cmax))
				h = (g - b) / delta;
			else if (Math::equals(g, cmax))
				h = 2.0f + (b - r) / delta;
			else
				h = 4.0f + (r - g) / delta;

			h *= 60.0f;

			if (h < 0.0f)
				h += 360.0f;

			*hue = h;
		}
	}
}

Vector3f Color::toVector3f() const
{
	return Vector3f(r, g, b);
}

Vector4f Color::toVector4f() const
{
	return Vector4f(r, g, b, a);
}
