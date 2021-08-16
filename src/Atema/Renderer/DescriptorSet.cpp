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

#include <Atema/Renderer/DescriptorSet.hpp>
#include <Atema/Renderer/Renderer.hpp>

using namespace at;

// DescriptorSetBinding
DescriptorSetBinding::DescriptorSetBinding() :
	type(DescriptorType::UniformBuffer),
	binding(0),
	count(1),
	shaderStages(ShaderStage::Vertex)
{
}

DescriptorSetBinding::DescriptorSetBinding(DescriptorType type, uint32_t binding, uint32_t count, Flags<ShaderStage> shaderStages) :
	type(type),
	binding(binding),
	count(count),
	shaderStages(shaderStages)
{
}

// DescriptorSetLayout
DescriptorSetLayout::DescriptorSetLayout()
{
}

DescriptorSetLayout::~DescriptorSetLayout()
{
}

Ptr<DescriptorSetLayout> DescriptorSetLayout::create(const Settings& settings)
{
	return Renderer::getInstance().createDescriptorSetLayout(settings);
}

// DescriptorSet
DescriptorSet::DescriptorSet()
{
}

DescriptorSet::~DescriptorSet()
{
}

void DescriptorSet::update(uint32_t binding, const Ptr<Buffer>& buffer)
{
	update({ binding }, { 0 }, { { buffer } }, {}, {}, {}, {});
}

void DescriptorSet::update(uint32_t binding, uint32_t index, const std::vector<Ptr<Buffer>>& buffers)
{
	update({ binding }, { index }, { buffers }, {}, {}, {}, {});
}

void DescriptorSet::update(uint32_t binding, const Ptr<Image>& image, const Ptr<Sampler>& sampler)
{
	update({}, {}, {}, { binding }, { 0 }, { { image } }, { { sampler } });
}

void DescriptorSet::update(uint32_t binding, uint32_t index, const std::vector<Ptr<Image>>& images, const std::vector<Ptr<Sampler>>& samplers)
{
	update({}, {}, {}, { binding }, { index }, { images }, { samplers });
}
