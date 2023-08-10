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

#include <Atema/Graphics/Loaders/DefaultImageLoader.hpp>
#include <Atema/Renderer/Utils.hpp>

#include <unordered_set>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

using namespace at;

namespace
{
	std::unordered_set<std::string_view> extensions =
	{
		".jpg",
		".jpeg",
		".png",
		".tga",
		".bmp",
		".hdr",
	};

	int getSTBIFormat(ImageFormat format)
	{
		static std::array<int, 4> stbiFormats =
		{
			STBI_grey,
			STBI_grey_alpha,
			STBI_rgb,
			STBI_rgb_alpha,
		};

		return stbiFormats[getComponentCount(format) - 1];
	}

	ImageLoader::Format getLoaderFormat(int channels)
	{
		switch (channels)
		{
			case 1:
				return ImageLoader::Format::R;
			case 2:
				return ImageLoader::Format::RG;
			case 3:
				return ImageLoader::Format::RGB;
			case 4:
				return ImageLoader::Format::RGBA;
			default:
			{
				ATEMA_ERROR("Invalid channel count");
			}
		}

		return ImageLoader::Format::R;
	}
}

Ptr<Image> DefaultImageLoader::load(const std::filesystem::path& path, const ImageLoader::Settings& settings)
{
	if (!isExtensionSupported(path.extension()))
		return nullptr;

	// Load the texture data
	const auto filenameStr = path.string();
	const auto filename = filenameStr.c_str();

	int width, height, channels;

	if (stbi_info(filename, &width, &height, &channels) != 1)
		ATEMA_ERROR("Failed to load the image");

	const auto loaderFormat = getLoaderFormat(channels);

	Ptr<Image> image;
	if (stbi_is_hdr(filename))
	{
		const ImageFormat format = ImageLoader::getSupportedHDRFormat(loaderFormat, settings.usages);

		float* pixels = stbi_loadf(filename, &width, &height, &channels, getSTBIFormat(format));

		image = ImageLoader::loadImage(pixels, width, height, loaderFormat, settings);

		stbi_image_free(pixels);
	}
	else
	{
		const ImageFormat format = ImageLoader::getSupportedColorFormat(loaderFormat, settings.usages);

		stbi_uc* pixels = stbi_load(filename, &width, &height, &channels, getSTBIFormat(format));

		image = ImageLoader::loadImage(reinterpret_cast<uint8_t*>(pixels), width, height, loaderFormat, settings);

		stbi_image_free(pixels);
	}

	return image;
}

bool DefaultImageLoader::isExtensionSupported(const std::filesystem::path& extension)
{
	return extensions.count(extension.string()) > 0;
}
