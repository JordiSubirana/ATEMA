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

#ifndef ATEMA_SANDBOX_GRAPHICSSYSTEM_HPP
#define ATEMA_SANDBOX_GRAPHICSSYSTEM_HPP

#include <Atema/Graphics/DebugRenderer.hpp>
#include <Atema/Math/AABB.hpp>
#include <Atema/Math/Enums.hpp>
#include <Atema/Graphics/FrameRenderer.hpp>
#include <Atema/Graphics/PerspectiveCamera.hpp>
#include <Atema/Graphics/StaticModel.hpp>

#include "System.hpp"
#include "../Settings.hpp"

class GraphicsSystem : public System
{
public:
	GraphicsSystem() = delete;
	GraphicsSystem(const at::Ptr<at::RenderWindow>& renderWindow);
	virtual ~GraphicsSystem();

	void update(at::TimeStep timeStep) override;
	void onEvent(at::Event& event) override;

	void onEntityAdded(at::EntityHandle entity) override;
	void onEntityRemoved(at::EntityHandle entity) override;

	//TODO: Remove this once SkyBox is implemented as a complete entity
	at::RenderScene& getRenderScene() noexcept;

private:
	void checkSettings();
	void onResize(const at::Vector2u& size);
	void updateFrame();

	void updateRenderables();
	void updateCamera();

	void destroyAfterUse(at::Ptr<void> resource);
	void destroyPendingResources(at::RenderContext& renderContext);

	at::WPtr<at::RenderWindow> m_renderWindow;

	// Settings
	float m_baseDepthBias;
	uint32_t m_shadowMapSize;
	uint32_t m_shadowCascadeCount;

	float m_frustumRotation;

	at::FrameRenderer m_frameRenderer;
	at::PerspectiveCamera m_camera;

	std::vector<at::Ptr<void>> m_resourcesToDestroy;

	std::array<at::UPtr<at::RenderContext>, at::Renderer::FramesInFlight> m_renderContexts;
};

#endif
