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

#include <Atema/Graphics/Loaders/ImageLoader.hpp>
#include <Atema/Renderer/Renderer.hpp>
#include <Atema/Renderer/Utils.hpp>

using namespace at;

namespace
{
	struct Format
	{
		ImageFormat format = ImageFormat::RGBA8_UNORM;
		size_t componentCount = 4;
	};

	void initializeDefaultFormat(Format& format, const std::vector<ImageFormat>& targetFormats)
	{
		const auto& renderer = Renderer::instance();
		
		for (auto& targetFormat : targetFormats)
		{
			const auto usages = renderer.getImageFormatOptimalUsages(targetFormat);

			if (usages & ImageUsage::ShaderSampling && usages & ImageUsage::TransferSrc && usages & ImageUsage::TransferDst)
			{
				format.format = targetFormat;
				format.componentCount = at::getComponentCount(targetFormat);

				return;
			}
		}
	}

	std::vector<Format> initializeDefaultFormats()
	{
		static const std::vector<ImageFormat> formats1 =
		{
			ImageFormat::R8_UNORM
		};

		static const std::vector<ImageFormat> formats2 =
		{
			ImageFormat::RG8_UNORM
		};

		static const std::vector<ImageFormat> formats3 =
		{
			ImageFormat::RGB8_UNORM,
		};

		static const std::vector<ImageFormat> formats4 =
		{
			ImageFormat::RGBA8_UNORM
		};

		std::vector<Format> defaultFormats(4);

		initializeDefaultFormat(defaultFormats[0], formats1);
		initializeDefaultFormat(defaultFormats[1], formats2);
		initializeDefaultFormat(defaultFormats[2], formats3);
		initializeDefaultFormat(defaultFormats[3], formats4);

		return std::move(defaultFormats);
	}

	const Format& getDefaultFormat(ImageLoader::Format format)
	{
		static const auto defaultFormats = initializeDefaultFormats();

		return defaultFormats[static_cast<size_t>(format)];
	}
}

Ptr<Image> at::ImageLoader::loadImage(const uint8_t* data, uint32_t width, uint32_t height, Format format, const Settings& settings)
{
	const auto& defaultFormat = getDefaultFormat(format);
	
	ATEMA_ASSERT(data, "Invalid pixel data");
	ATEMA_ASSERT(width > 0 && height > 0, "Invalid image size");

	// std::max for the largest dimension, std::log2 to find how many times we can divide by 2
	// std::floor handles the case when the dimension is not power of 2, +1 to add a mip level to the original image
	const auto mipLevels = settings.mipLevels == 0 ? static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1 : settings.mipLevels;
	
	const auto imageSize = width * height * defaultFormat.componentCount;

	auto stagingBuffer = Buffer::create({ BufferUsage::TransferSrc | BufferUsage::Map, imageSize });

	Ptr<CommandBuffer> commandBuffer = settings.commandBuffer;

	// Command buffer not provided by the user : we create one
	if (!settings.commandBuffer)
	{
		auto commandPool = Renderer::instance().getCommandPool(QueueType::Graphics);

		commandBuffer = commandPool->createBuffer({ true });

		commandBuffer->begin();
	}
	// The command buffer is provided, so the user will be responsible to delete staging buffers once the command is executed
	else
	{
		ATEMA_ASSERT(settings.stagingBuffers, "Invalid staging buffer list");

		settings.stagingBuffers->emplace_back(stagingBuffer);
	}

	// Fill staging buffer
	auto bufferData = stagingBuffer->map();

	memcpy(bufferData, data, imageSize);

	stagingBuffer->unmap();

	// Create image
	Image::Settings imageSettings;
	imageSettings.format = defaultFormat.format;
	imageSettings.width = width;
	imageSettings.height = height;
	imageSettings.mipLevels = mipLevels;
	imageSettings.usages = settings.usages | ImageUsage::TransferDst;
	
	if (mipLevels > 1)
		imageSettings.usages |= ImageUsage::TransferSrc;

	auto image = Image::create(imageSettings);

	// Copy staging buffer to index buffer

	commandBuffer->imageBarrier(image, ImageBarrier::InitializeTransferDst);

	if (mipLevels == 1)
	{
		commandBuffer->copyBuffer(stagingBuffer, image, ImageLayout::ShaderRead);
	}
	else
	{
		commandBuffer->copyBuffer(stagingBuffer, image, ImageLayout::TransferDst);

		commandBuffer->createMipmaps(image, PipelineStage::FragmentShader, MemoryAccess::ShaderRead, ImageLayout::ShaderRead);
	}

	// We created our own command buffer, execute it right now, so we can delete staging buffers
	if (!settings.commandBuffer)
	{
		commandBuffer->end();

		Renderer::instance().submitAndWait({ commandBuffer });
	}

	return image;
}

size_t ImageLoader::getComponentCount(Format format)
{
	return getDefaultFormat(format).componentCount;
}
