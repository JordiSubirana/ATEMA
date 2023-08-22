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
#include <Atema/Graphics/RenderObject.hpp>
#include <Atema/Graphics/SkyBox.hpp>

#include <vector>
#include <unordered_map>

namespace at
{
	class AbstractFrameRenderer;

	class ATEMA_GRAPHICS_API RenderScene : public RenderResource
	{
	public:
		RenderScene() = delete;
		RenderScene(RenderResourceManager& resourceManager, AbstractFrameRenderer& frameRenderer);
		RenderScene(const RenderScene& other) = default;
		RenderScene(RenderScene&& other) noexcept = default;
		~RenderScene() = default;

		bool isValid() const noexcept;

		void setCamera(const Camera& camera);

		void setSkyBox(Ptr<SkyBox> skyBox);

		void addLight(Light& light);
		void addRenderable(Renderable& renderable);
		
		void removeLight(const Light& light);
		void removeRenderable(const Renderable& renderable);

		void clear();
		void clearLights();
		void clearRenderables();

		void recompileMaterials();

		const Camera& getCamera() const noexcept;
		const Ptr<SkyBox>& getSkyBox() const noexcept;
		RenderMaterial& getRenderMaterial(Ptr<Material> material);
		RenderMaterialInstance& getRenderMaterialInstance(const MaterialInstance& materialInstance);
		const std::vector<Ptr<RenderObject>>& getRenderObjects() const noexcept;
		const std::vector<Ptr<RenderLight>>& getRenderLights() const noexcept;

		RenderScene& operator=(const RenderScene& other) = default;
		RenderScene& operator=(RenderScene&& other) noexcept = default;

	protected:
		void updateResources() override;

	private:
		AbstractFrameRenderer* m_frameRenderer;

		const Camera* m_camera;

		Ptr<SkyBox> m_skyBox;
		
		std::vector<Ptr<RenderObject>> m_renderObjects;
		std::unordered_map<const Renderable*, size_t> m_renderObjectIndices;
		
		std::vector<Ptr<RenderLight>> m_renderLights;
		std::unordered_map<const Light*, size_t> m_renderLightIndices;

		std::unordered_map<const Material*, Ptr<RenderMaterial>> m_renderMaterials;
		std::unordered_map<const MaterialInstance*, Ptr<RenderMaterialInstance>> m_renderMaterialInstances;
	};
}

#endif