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
#include <Atema/Graphics/Loaders/ImageLoader.hpp>
#include <Atema/Renderer/Renderer.hpp>
#include <Atema/Renderer/Utils.hpp>

using namespace at;

namespace
{
	const std::vector<ImageComponentType> colorComponentTypes =
	{
		ImageComponentType::UNORM8
	};

	const std::vector<ImageComponentType> hdrComponentTypes =
	{
		ImageComponentType::SFLOAT32
	};

	constexpr int ImageUsageCount = static_cast<int>(ImageUsage::All) + 1;

	using ImageFormatArray = std::array<std::array<std::optional<ImageFormat>, ImageUsageCount>, 4>;

	ImageFormatArray initializeFormats(const std::vector<ImageComponentType>& componentTypes)
	{
		Renderer& renderer = Renderer::instance();

		ImageFormatArray formats;

		for (size_t componentCount = 1; componentCount <= 4; componentCount++)
		{
			for (int usages = 0; usages < ImageUsageCount; usages++)
			{
				for (const auto& componentType : componentTypes)
				{
					auto format = renderer.getSupportedImageFormat(componentType, componentCount, static_cast<Flags<ImageUsage>>(usages));

					if (format.has_value())
					{
						formats[componentCount - 1][usages] = format;
						break;
					}
				}
			}
		}

		return formats;
	}

	template <typename T>
	constexpr bool supportsHDR()
	{
		return false;
	}

	template <>
	constexpr bool supportsHDR<float>()
	{
		return true;
	}

	template <typename T>
	Ptr<Image> loadImage(const T* data, uint32_t width, uint32_t height, ImageLoader::Format format, const ImageLoader::Settings& settings)
	{
		ATEMA_ASSERT(data, "Invalid pixel data");
		ATEMA_ASSERT(width > 0 && height > 0, "Invalid image size");

		const size_t pixelCount = static_cast<size_t>(width) * static_cast<size_t>(height);

		if (settings.type == ImageType::CubeMap)
		{
			width /= 4;
			height /= 3;

			ATEMA_ASSERT(width == height, "Cubemap image must have a 4:3 ratio");
		}

		// std::max for the largest dimension, std::log2 to find how many times we can divide by 2
		// std::floor handles the case when the dimension is not power of 2, +1 to add a mip level to the original image
		const uint32_t mipLevels = settings.mipLevels == 0 ? static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1 : settings.mipLevels;

		Flags<ImageUsage> imageUsages = settings.usages | ImageUsage::TransferDst;

		if (mipLevels > 1)
			imageUsages |= ImageUsage::TransferSrc;

		ImageFormat imageFormat;

		if constexpr (supportsHDR<T>())
			imageFormat = ImageLoader::getSupportedHDRFormat(format, imageUsages);
		else
			imageFormat = ImageLoader::getSupportedColorFormat(format, imageUsages);

		const size_t imageByteSize = getByteSize(imageFormat) * pixelCount;

		Ptr<Buffer> stagingBuffer = Buffer::create({ BufferUsage::TransferSrc | BufferUsage::Map, imageByteSize });

		Ptr<CommandBuffer> commandBuffer = settings.commandBuffer;

		// Command buffer not provided by the user : we create one
		if (!settings.commandBuffer)
		{
			CommandPool& commandPool = *Renderer::instance().getCommandPool(QueueType::Graphics);

			commandBuffer = commandPool.createBuffer({ true });

			commandBuffer->begin();
		}
		// The command buffer is provided, so the user will be responsible to delete staging buffers once the command is executed
		else
		{
			ATEMA_ASSERT(settings.stagingBuffers, "Invalid staging buffer list");

			settings.stagingBuffers->emplace_back(stagingBuffer);
		}

		// Fill staging buffer
		void* bufferData = stagingBuffer->map();

		memcpy(bufferData, data, imageByteSize);

		stagingBuffer->unmap();

		// Create image
		Image::Settings imageSettings;
		imageSettings.format = imageFormat;
		imageSettings.type = settings.type;
		imageSettings.width = width;
		imageSettings.height = height;
		imageSettings.mipLevels = mipLevels;
		imageSettings.usages = imageUsages;

		Ptr<Image> image = Image::create(imageSettings);

		// Copy staging buffer to index buffer

		commandBuffer->imageBarrier(*image, ImageBarrier::InitializeTransferDst);

		if (mipLevels == 1)
		{
			if (settings.type == ImageType::CubeMap)
				commandBuffer->copyBufferToCubemap(*stagingBuffer, *image, ImageLayout::TransferDst);
			else
				commandBuffer->copyBufferToImage(*stagingBuffer, *image, ImageLayout::TransferDst);

			commandBuffer->imageBarrier(*image, ImageBarrier::TransferDstToFragmentShaderRead);
		}
		else
		{
			if (settings.type == ImageType::CubeMap)
				commandBuffer->copyBufferToCubemap(*stagingBuffer, *image, ImageLayout::TransferDst);
			else
				commandBuffer->copyBufferToImage(*stagingBuffer, *image, ImageLayout::TransferDst);

			commandBuffer->createMipmaps(*image, PipelineStage::FragmentShader, MemoryAccess::ShaderRead, ImageLayout::ShaderRead);
		}

		// We created our own command buffer, execute it right now, so we can delete staging buffers
		if (!settings.commandBuffer)
		{
			commandBuffer->end();

			Renderer::instance().submitAndWait({ commandBuffer });
		}

		return image;
	}
}

Ptr<Image> ImageLoader::loadImage(const uint8_t* data, uint32_t width, uint32_t height, Format format, const Settings& settings)
{
	return ::loadImage(data, width, height, format, settings);
}

Ptr<Image> ImageLoader::loadImage(const float* data, uint32_t width, uint32_t height, Format format, const Settings& settings)
{
	return ::loadImage(data, width, height, format, settings);
}

ImageFormat ImageLoader::getSupportedColorFormat(Format format, Flags<ImageUsage> usages)
{
	static ImageFormatArray s_formats = initializeFormats(colorComponentTypes);

	const auto& imageFormat = s_formats[static_cast<size_t>(format)][usages.getValue()];

	if (!imageFormat.has_value())
		ATEMA_ERROR("No ImageFormat with these requirements is supported");

	return imageFormat.value();
}

ImageFormat ImageLoader::getSupportedHDRFormat(Format format, Flags<ImageUsage> usages)
{
	static ImageFormatArray s_formats = initializeFormats(hdrComponentTypes);

	const auto& imageFormat = s_formats[static_cast<size_t>(format)][usages.getValue()];

	if (!imageFormat.has_value())
		ATEMA_ERROR("No ImageFormat with these requirements is supported");

	return imageFormat.value();
}
