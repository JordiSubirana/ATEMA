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

#include <Atema/Renderer/DescriptorSet.hpp>
#include <Atema/Renderer/Renderer.hpp>

using namespace at;

DescriptorSet::DescriptorSet()
{
}

DescriptorSet::~DescriptorSet()
{
}

void DescriptorSet::update(uint32_t binding, const Ptr<Buffer>& buffer)
{
	update(binding, buffer, 0);
}

void DescriptorSet::update(uint32_t binding, const Ptr<Buffer>& buffer, size_t bufferRange)
{
	update({ binding }, { 0 }, { { buffer } }, { { bufferRange } }, {}, {}, {}, {});
}

void DescriptorSet::update(uint32_t binding, uint32_t index, const std::vector<Ptr<Buffer>>& buffers)
{
	update(binding, index, buffers, {});
}

void DescriptorSet::update(uint32_t binding, uint32_t index, const std::vector<Ptr<Buffer>>& buffers, const std::vector<size_t>& bufferRanges)
{
	update({ binding }, { index }, { buffers }, { bufferRanges }, {}, {}, {}, {});
}

void DescriptorSet::update(uint32_t binding, const Ptr<ImageView>& imageView, const Ptr<Sampler>& sampler)
{
	update({}, {}, {}, {}, { binding }, { 0 }, { { imageView } }, { { sampler } });
}

void DescriptorSet::update(uint32_t binding, uint32_t index, const std::vector<Ptr<ImageView>>& imageViews, const std::vector<Ptr<Sampler>>& samplers)
{
	update({}, {}, {}, {}, { binding }, { index }, { imageViews }, { samplers });
}
