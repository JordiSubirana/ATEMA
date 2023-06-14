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

#ifndef ATEMA_GRAPHICS_RENDERDATA_HPP
#define ATEMA_GRAPHICS_RENDERDATA_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/Camera.hpp>
#include <Atema/Graphics/Renderable.hpp>

#include <vector>
#include <unordered_map>

namespace at
{
	class ATEMA_GRAPHICS_API RenderData
	{
	public:
		RenderData();
		RenderData(const RenderData& other) = default;
		RenderData(RenderData&& other) noexcept = default;
		~RenderData() = default;

		bool isValid() const noexcept;

		void setCamera(const Camera& camera);
		void addRenderable(Renderable& renderable);

		void removeRenderable(const Renderable& renderable);

		void clearRenderables();

		const Camera& getCamera() const noexcept;
		const std::vector<Renderable*>& getRenderables() const noexcept;

		RenderData& operator=(const RenderData& other) = default;
		RenderData& operator=(RenderData&& other) noexcept = default;

	private:
		const Camera* m_camera;

		std::vector<Renderable*> m_renderables;
		std::unordered_map<const Renderable*, size_t> m_renderableIndices;
	};
}

#endif