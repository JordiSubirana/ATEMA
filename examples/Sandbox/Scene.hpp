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

#ifndef ATEMA_SANDBOX_SCENE_HPP
#define ATEMA_SANDBOX_SCENE_HPP

#include <Atema/Atema.hpp>

struct MaterialData;
struct ModelData;

struct ObjectData
{
	ObjectData();
	
	at::Ptr<at::Buffer> vertexBuffer;
	at::Ptr<at::Buffer> indexBuffer;
	uint32_t indexCount = 0;
	at::Ptr<at::Image> texture;
	at::Ptr<at::Sampler> sampler;
	at::Matrix4f transform;
	at::Vector3f position;
	at::Vector3f rotation;
};

class ObjectFrameData
{
public:
	ObjectFrameData() = delete;
	ObjectFrameData(const ObjectData& object, uint32_t frameCount, at::Ptr<at::DescriptorPool> descriptorPool);

	at::Ptr<at::Buffer> getBuffer(uint32_t frameIndex);
	at::Ptr<at::DescriptorSet> getDescriptorSet(uint32_t frameIndex);

private:
	std::vector<at::Ptr<at::Buffer>> m_uniformBuffers;
	std::vector<at::Ptr<at::DescriptorSet>> m_descriptorSets;
};

struct UniformFrameElement
{
	at::Matrix4f proj;
	at::Matrix4f view;
};

struct UniformObjectElement
{
	at::Matrix4f model;
};

class Scene
{
public:
	Scene();
	virtual ~Scene();

	void updateObjects(at::TimeStep timeStep, size_t threadCount = 0);
	
	const std::vector<ObjectData>& getObjects() const noexcept;

private:
	void loadResources();
	
	// Global resources
	at::Ptr<ModelData> m_modelData;
	at::Ptr<MaterialData> m_materialData;

	std::vector<ObjectData> m_objects;
};

#endif
