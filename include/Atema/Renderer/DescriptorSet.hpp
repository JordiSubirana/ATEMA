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

#ifndef ATEMA_RENDERER_DESCRIPTORSET_HPP
#define ATEMA_RENDERER_DESCRIPTORSET_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Renderer/Enums.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Core/Pointer.hpp>

#include <vector>

namespace at
{
	class Buffer;
	class ImageView;
	class Sampler;

	class ATEMA_RENDERER_API DescriptorSet : public NonCopyable
	{
	public:
		virtual ~DescriptorSet();

		// Default size of 0 means remaining size
		virtual void update(uint32_t binding, const Buffer& buffer, size_t offset = 0, size_t size = 0) = 0;
		// Updates an array of buffers, starting at index
		virtual void update(uint32_t binding, uint32_t index, const std::vector<const Buffer*>& buffers) = 0;
		// Updates an array of buffer ranges, starting at index
		virtual void update(uint32_t binding, uint32_t index, const std::vector<const Buffer*>& buffers, const std::vector<size_t>& bufferOffsets, const std::vector<size_t>& bufferSizes) = 0;
		virtual void update(uint32_t binding, const ImageView& imageView, const Sampler& sampler) = 0;
		// Updates an array of sampled images, starting at index
		virtual void update(uint32_t binding, uint32_t index, const std::vector<const ImageView*>& imageViews, const std::vector<const Sampler*>& samplers) = 0;
		
	protected:
		DescriptorSet();
	};
}

#endif
