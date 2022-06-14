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

#include "System.hpp"

class GraphicsSystem : public System
{
public:
	GraphicsSystem() = delete;
	GraphicsSystem(const at::Ptr<at::RenderWindow>& renderWindow);
	virtual ~GraphicsSystem();

	void update(at::TimeStep timeStep) override;
	void onEvent(at::Event& event) override;

private:
	struct FrameData
	{
		at::Ptr<at::Buffer> frameUniformBuffer;
		at::Ptr<at::Buffer> objectsUniformBuffer;
		at::Ptr<at::DescriptorSet> descriptorSet;
	};

	void createFrameGraph();
	void onResize(const at::Vector2u& size);
	void updateFrame();

	void updateUniformBuffers(FrameData& frameData);

	at::WPtr<at::RenderWindow> m_renderWindow;
	
	float m_totalTime;

	at::ImageFormat m_depthFormat;

	at::Viewport m_viewport;
	at::Vector2u m_windowSize;

	// FrameGraph
	at::Ptr<at::FrameGraph> m_frameGraph;

	// Rendering resources (deferred)
	at::Ptr<at::GraphicsPipeline> m_gbufferPipeline;

	// Rendering resources (post process)
	at::Ptr<at::Buffer> m_ppQuad;
	at::Ptr<at::Sampler> m_ppSampler;
	at::Ptr<at::GraphicsPipeline> m_ppPipeline;
	at::Ptr<at::DescriptorSetLayout> m_ppDescriptorSetLayout;

	// Object resources
	at::Ptr<at::DescriptorSetLayout> m_materialDescriptorSetLayout;
	at::Ptr<at::DescriptorSet> m_materialDescriptorSet;

	// Frame resources
	at::Ptr<at::DescriptorSetLayout> m_frameDescriptorSetLayout;
	uint32_t m_dynamicObjectBufferOffset;
	std::vector<FrameData> m_frameDatas;

	// Shaders
	std::unordered_map<std::string, at::Ptr<at::Shader>> m_shaders;
};

#endif
