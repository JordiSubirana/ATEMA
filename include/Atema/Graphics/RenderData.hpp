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
#include <Atema/Graphics/Light.hpp>
#include <Atema/Graphics/RenderLight.hpp>

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

		// Defines Renderables update policy
		// If true, on scene change, the user must call clearRenderables then add all renderables
		// If false, on scene change, the user must mainly call addRenderable / removeRenderable
		// Default : false
		void resetRenderablesEveryFrame(bool reset);

		// This method is faster when Renderables are reset every frame
		void addRenderable(Renderable& renderable);

		// This method is faster when Renderables are not reset every frame
		void removeRenderable(const Renderable& renderable);

		void clearRenderables();

		void resetLightsEveryFrame(bool reset);

		void addLight(Light& light);

		void removeLight(const Light& light);

		void clearLights();

		const Camera& getCamera() const noexcept;
		const std::vector<Renderable*>& getRenderables() const noexcept;
		const std::vector<Ptr<RenderLight>>& getRenderLights() const noexcept;

		RenderData& operator=(const RenderData& other) = default;
		RenderData& operator=(RenderData&& other) noexcept = default;

	private:
		const Camera* m_camera;

		bool m_resetRenderablesEveryFrame;
		std::vector<Renderable*> m_renderables;
		std::unordered_map<const Renderable*, size_t> m_renderableIndices;

		bool m_resetLightsEveryFrame;
		std::unordered_map<const Light*, size_t> m_lightIndices;
		std::vector<Ptr<RenderLight>> m_renderLights;
	};
}

#endif