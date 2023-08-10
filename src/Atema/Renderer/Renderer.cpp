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

#include <Atema/Renderer/Renderer.hpp>
#include <Atema/Renderer/Utils.hpp>
#include <Atema/Window/Window.hpp>

using namespace at;

namespace
{
	constexpr size_t ImageComponentTypeCount = static_cast<size_t>(ImageComponentType::_COUNT);
	constexpr size_t ImageUsageCount = static_cast<size_t>(ImageUsage::All) + 1;

	// ImageFormatArray[ImageComponentType][ComponentCount][Usages]
	using ImageFormatArray = std::array<std::array<std::array<std::optional<ImageFormat>, ImageUsageCount>, 4>, ImageComponentTypeCount>;

	ImageFormatArray initializeFormats(Renderer& renderer)
	{
		ImageFormatArray formats;

		// For all component types
		for (size_t t = 0; t < ImageComponentTypeCount; t++)
		{
			// For all minimum component count
			for (size_t mc = 1; mc <= 4; mc++)
			{
				// For all usages
				for (size_t u = 0; u < ImageUsageCount; u++)
				{
					// For all component count
					for (size_t c = mc; c <= 4; c++)
					{
						const ImageFormat format = getImageColorFormat(static_cast<ImageComponentType>(t), c);

						const size_t usages = static_cast<size_t>(renderer.getImageFormatOptimalUsages(format).getValue());

						if ((usages & u) == u)
						{
							formats[t][mc - 1][u] = format;
							break;
						}
					}
				}
			}
		}

		return formats;
	}
}

Ptr<Renderer> Renderer::s_renderer = nullptr;

Renderer::Renderer(const Settings& settings) :
	m_settings(settings),
	m_hashType(0)
{
}

Renderer::~Renderer()
{
}

Renderer& Renderer::instance()
{
	if (!s_renderer)
	{
		ATEMA_ERROR("No Renderer available");
	}

	return *s_renderer;
}

void Renderer::destroy()
{
	s_renderer.reset();
}

bool Renderer::isDepthImageFormat(ImageFormat format)
{
	switch (format)
	{
		case ImageFormat::D16_UNORM:
		case ImageFormat::D32_SFLOAT:
		case ImageFormat::D16_UNORM_S8_UINT:
		case ImageFormat::D24_UNORM_S8_UINT:
		case ImageFormat::D32_SFLOAT_S8_UINT:
		{
			return true;
		}
		default:
		{
			return false;
		}
	}

	return false;
}

bool Renderer::isStencilImageFormat(ImageFormat format)
{
	switch (format)
	{
		case ImageFormat::D16_UNORM_S8_UINT:
		case ImageFormat::D24_UNORM_S8_UINT:
		case ImageFormat::D32_SFLOAT_S8_UINT:
		{
			return true;
		}
		default:
		{
			return false;
		}
	}

	return false;
}

const Hash Renderer::getID() const noexcept
{
	return m_hashType;
}

const Renderer::Settings& Renderer::getSettings() const noexcept
{
	return m_settings;
}

std::optional<ImageFormat> Renderer::getSupportedImageFormat(ImageComponentType componentType, size_t minimumComponentCount, Flags<ImageUsage> usages)
{
	static const ImageFormatArray s_supportedFormats = initializeFormats(*this);

	return s_supportedFormats[static_cast<size_t>(componentType)][minimumComponentCount - 1][static_cast<size_t>(usages.getValue())];
}

void Renderer::submitAndWait(const std::vector<Ptr<CommandBuffer>>& commandBuffers)
{
	auto fence = Fence::create({});

	submit(commandBuffers, {}, {}, fence);

	// Wait for the command to be done
	fence->wait();
}
