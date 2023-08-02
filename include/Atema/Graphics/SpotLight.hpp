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

#ifndef ATEMA_GRAPHICS_SPOTLIGHT_HPP
#define ATEMA_GRAPHICS_SPOTLIGHT_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/Light.hpp>

namespace at
{
	class ATEMA_GRAPHICS_API SpotLight : public Light
	{
	public:
		SpotLight();
		SpotLight(const SpotLight& other) = default;
		SpotLight(SpotLight&& other) noexcept = default;
		~SpotLight() = default;

		LightType getType() const noexcept override;

		void setPosition(const Vector3f& position);
		const Vector3f& getPosition() const noexcept;

		void setDirection(const Vector3f& direction);
		const Vector3f& getDirection() const noexcept;

		void setRange(float range);
		float getRange() const noexcept;

		void setAngle(float angle);
		float getAngle() const noexcept;
		
		SpotLight& operator=(const SpotLight& other) = default;
		SpotLight& operator=(SpotLight&& other) noexcept = default;

	private:
		Vector3f m_position;
		Vector3f m_direction;
		float m_range;
		float m_angle;
	};
}

#endif
