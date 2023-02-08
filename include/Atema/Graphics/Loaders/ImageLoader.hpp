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

#ifndef ATEMA_GRAPHICS_IMAGELOADER_HPP
#define ATEMA_GRAPHICS_IMAGELOADER_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Renderer/Image.hpp>

#include <list>

namespace at
{
	class Buffer;
	class CommandBuffer;
	
	struct ATEMA_GRAPHICS_API ImageLoader
	{
		enum class Format
		{
			R,
			RG,
			RGB,
			RGBA
		};
		
		struct Settings
		{
			// Default value 0 stands for automatic mip levels
			uint32_t mipLevels = 0;

			ImageSamples samples = ImageSamples::S1;

			ImageTiling tiling = ImageTiling::Optimal;

			// - ImageUsage::TransferDst will be added if not specified
			// - ImageUsage::TransferSrc will be added if mipLevels != 1
			Flags<ImageUsage> usages = ImageUsage::ShaderSampling;

			// Optional command buffer
			// If valid:
			//	- it will be used if there are staging buffers to copy to images (ie if image is not mappable)
			//	- CommandBuffer::begin() & CommandBuffer::end() must be called by the user before and after the loading
			// If not, a temporary command buffer will be created by the loader and images will be valid just after the loading
			// See 'stagingBuffers' member for more details
			Ptr<CommandBuffer> commandBuffer;

			// Optional list containing all the staging buffers used by the loader
			// This is an output parameter, storing all the staging buffers created to load the image, if any
			// The user must keep them around until 'commandBuffer' is executed
			// If 'commandBuffer' is valid, 'stagingBuffers' must be valid
			// See 'commandBuffer' member for more details
			std::list<Ptr<Buffer>>* stagingBuffers;
		};

		static Ptr<Image> loadImage(const uint8_t* data, uint32_t width, uint32_t height, Format format, const Settings& settings);

		// Depending on the supported image formats detected at runtime, component count could differ the naive value of the format
		static size_t getComponentCount(Format format);
	};
}

#endif