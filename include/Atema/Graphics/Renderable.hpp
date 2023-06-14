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

#ifndef ATEMA_GRAPHICS_RENDERABLE_HPP
#define ATEMA_GRAPHICS_RENDERABLE_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/RenderElement.hpp>
#include <Atema/Math/AABB.hpp>
#include <Atema/Math/Transform.hpp>

namespace at
{
	class RenderFrame;
	
	class ATEMA_GRAPHICS_API Renderable
	{
		friend class RenderScene;

	public:
		Renderable();
		Renderable(const Renderable& other) = default;
		Renderable(Renderable&& other) noexcept = default;
		virtual ~Renderable() = default;

		virtual void updateResources(RenderFrame& renderFrame, CommandBuffer& commandBuffer) = 0;

		virtual void getRenderElements(std::vector<RenderElement>& renderElements) const = 0;
		virtual size_t getRenderElementsSize() const noexcept = 0;

		virtual const Transform& getTransform() const noexcept = 0;

		virtual const AABBf& getAABB() const noexcept = 0;

		Renderable& operator=(const Renderable& other) = default;
		Renderable& operator=(Renderable&& other) noexcept = default;

	private:
	};
}

#endif