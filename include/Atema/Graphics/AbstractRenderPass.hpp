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

#ifndef ATEMA_GRAPHICS_ABSTRACTRENDERPASS_HPP
#define ATEMA_GRAPHICS_ABSTRACTRENDERPASS_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/FrameGraph.hpp>
#include <Atema/Graphics/RenderResourceManager.hpp>

namespace at
{
	class RenderScene;

	class ATEMA_GRAPHICS_API AbstractRenderPass
	{
	public:
		AbstractRenderPass(const AbstractRenderPass& other) = default;
		AbstractRenderPass(AbstractRenderPass&& other) noexcept = default;
		virtual ~AbstractRenderPass() = default;

		virtual const char* getName() const noexcept = 0;

		void initializeFrame(const RenderScene& renderScene);
		virtual void updateResources(CommandBuffer& commandBuffer);
		void finalizeFrame();

		AbstractRenderPass& operator=(const AbstractRenderPass& other) = default;
		AbstractRenderPass& operator=(AbstractRenderPass&& other) noexcept = default;

	protected:
		AbstractRenderPass();

		virtual void beginFrame();
		virtual void endFrame();

		// This is valid only between doBeginFrame & doEndFrame
		const RenderScene& getRenderScene() const noexcept;

	private:
		const RenderScene* m_renderScene;
	};
}

#endif