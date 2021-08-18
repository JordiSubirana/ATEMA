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

#include "Resources.hpp"

struct ObjectData
{
	at::Ptr<at::Buffer> vertexBuffer;
	at::Ptr<at::Buffer> indexBuffer;
	uint32_t indexCount = 0;
	at::Ptr<at::Image> texture;
	at::Ptr<at::Sampler> sampler;
	std::vector<at::Ptr<at::Buffer>> uniformBuffers;
	std::vector<at::Ptr<at::DescriptorSet>> descriptorSets;
};

class BasicRenderPipeline : public at::RenderPipeline
{
public:
	BasicRenderPipeline() = delete;
	BasicRenderPipeline(const at::RenderPipeline::Settings& settings);
	virtual ~BasicRenderPipeline();

protected:
	void resize(const at::Vector2u& size) override;
	void setupFrame(uint32_t frameIndex, at::TimeStep elapsedTime, at::Ptr<at::CommandBuffer> commandBuffer) override;

private:
	void loadResources();
	void updateUniformBuffers(uint32_t frameIndex);
	
	std::vector<at::Ptr<ObjectData>> m_objects;

	float m_totalTime;
	
	// Global resources
	at::Ptr<ModelData> m_modelData;
	at::Ptr<MaterialData> m_materialData;

	// Pipeline resources
	at::Ptr<at::DescriptorSetLayout> m_descriptorSetLayout;
	at::Ptr<at::GraphicsPipeline> m_pipeline;
	at::Ptr<at::DescriptorPool> m_descriptorPool;
};

#endif