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

#ifndef ATEMA_GRAPHICS_STATICRENDERMODEL_HPP
#define ATEMA_GRAPHICS_STATICRENDERMODEL_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/RenderObject.hpp>

namespace at
{
	class RenderScene;
	class RenderMaterial;
	class StaticModel;

	class ATEMA_GRAPHICS_API StaticRenderModel : public RenderObject
	{
	public:
		StaticRenderModel() = delete;
		StaticRenderModel(RenderScene& renderScene, StaticModel& staticModel);
		StaticRenderModel(const StaticRenderModel& other) = delete;
		StaticRenderModel(StaticRenderModel&& other) noexcept = default;
		~StaticRenderModel() = default;

		void getRenderElements(std::vector<RenderElement>& renderElements) const override;
		size_t getRenderElementsSize() const noexcept override;

		StaticRenderModel& operator=(const StaticRenderModel& other) = delete;
		StaticRenderModel& operator=(StaticRenderModel&& other) noexcept = default;

	protected:
		void updateResources(RenderFrame& renderFrame, CommandBuffer& commandBuffer) override;

	private:
		StaticModel* m_staticModel;

		bool m_modelValid;
		std::vector<RenderElement> m_renderElements;
		std::vector<const RenderMaterialInstance*> m_renderMaterialInstances;
		std::vector<Ptr<DescriptorSet>> m_transformDescriptorSets;

		bool m_transformValid;
		Ptr<Buffer> m_transformBuffer;

		ConnectionGuard m_connectionGuard;
	};
}

#endif
