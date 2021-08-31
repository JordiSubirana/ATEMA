/*
	Copyright 2021 Jordi SUBIRANA

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

#ifndef ATEMA_SANDBOX_BASICRENDERPIPELINE_HPP
#define ATEMA_SANDBOX_BASICRENDERPIPELINE_HPP

#include <Atema/Atema.hpp>

#include "Scene.hpp"

class BasicRenderPipeline : public at::RenderPipeline
{
public:
	BasicRenderPipeline() = delete;
	BasicRenderPipeline(const at::RenderPipeline::Settings& settings);
	virtual ~BasicRenderPipeline();

protected:
	void resize(const at::Vector2u& size) override;
	void updateFrame(at::TimeStep elapsedTime) override;
	void setupFrame(uint32_t frameIndex, at::Ptr<at::CommandBuffer> commandBuffer) override;

private:
	void loadScene();
	void updateUniformBuffers(uint32_t frameIndex);

	uint32_t m_maxFramesInFlight;
	float m_totalTime;
	
	// Object resources
	at::Ptr<at::DescriptorSetLayout> m_objectDescriptorSetLayout;
	at::Ptr<at::DescriptorPool> m_objectDescriptorPool;
	at::Ptr<Scene> m_scene;
	std::vector<ObjectFrameData> m_objectFrameData;

	// Frame resources
	at::Ptr<at::DescriptorSetLayout> m_frameDescriptorSetLayout;
	at::Ptr<at::DescriptorPool> m_frameDescriptorPool;
	std::vector<at::Ptr<at::Buffer>> m_frameUniformBuffers;
	std::vector<at::Ptr<at::DescriptorSet>> m_frameDescriptorSets;
	
	// Pipeline resources
	at::Ptr<at::GraphicsPipeline> m_pipeline;

	// Thread resources
	std::vector<std::vector<std::vector<at::Ptr<at::CommandBuffer>>>> m_threadCommandBuffers;
};

#endif
