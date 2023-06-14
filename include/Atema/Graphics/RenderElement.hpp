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

#ifndef ATEMA_GRAPHICS_RENDERELEMENT_HPP
#define ATEMA_GRAPHICS_RENDERELEMENT_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/ShaderBinding.hpp>
#include <Atema/Math/AABB.hpp>

#include <vector>

namespace at
{
	class VertexBuffer;
	class IndexBuffer;
	class SurfaceMaterialInstance;

	class ATEMA_GRAPHICS_API RenderElement
	{
	public:
		RenderElement();
		RenderElement(const RenderElement& other) = default;
		RenderElement(RenderElement&& other) noexcept = default;
		~RenderElement() = default;

		RenderElement& operator=(const RenderElement& other) = default;
		RenderElement& operator=(RenderElement&& other) noexcept = default;

		AABBf aabb;
		const VertexBuffer* vertexBuffer;
		const IndexBuffer* indexBuffer;
		const SurfaceMaterialInstance* materialInstance;
		const ShaderBinding* objectBinding;
	};
}

#endif