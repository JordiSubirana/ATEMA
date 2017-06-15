/*
	Copyright 2017 Jordi SUBIRANA

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

#include <Atema/Renderer/Texture.hpp>

#include <Atema/Core/Error.hpp>
#include <Atema/Renderer/RenderSystem.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_TGA
#include <stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

namespace at
{
	Texture::Implementation::Implementation()
	{
	}

	Texture::Texture()
		: Texture(32, 32) //32x32 is the default texture size
	{
	}

	Texture::Texture(unsigned width, unsigned height)
		: m_impl(nullptr)
	{
		RenderSystem *renderer = RenderSystem::getCurrent();

		if (!renderer)
			ATEMA_ERROR("There is no current RenderSystem.");

		m_impl = renderer->createTexture();

		resize(width, height);
	}

	Texture::~Texture()
	{
		delete m_impl;
	}

	Texture::Implementation* Texture::getImplementation()
	{
		return m_impl;
	}

	const Texture::Implementation* Texture::getImplementation() const
	{
		return m_impl;
	}

	void Texture::loadFromFile(const std::string& filename)
	{
		int w = 0, h = 0, bpp = 0;
		unsigned width = 0, height = 0;
		unsigned char *data = nullptr;

		std::vector<Color> pixels;

		if (filename.empty())
			ATEMA_ERROR("No filename given.");

		data = stbi_load(filename.c_str(), &w, &h, &bpp, STBI_rgb_alpha);

		if (!data)
			ATEMA_ERROR("Image file could not be loaded.");

		width = static_cast<unsigned>(w);
		height = static_cast<unsigned>(h);

		pixels.resize(width*height);

		for (size_t i = 0; i < pixels.size(); i++)
		{
			for (size_t o = 0; o < 4; o++)
			{
				pixels[i][o] = static_cast<float>(data[i * 4 + o]) / 255.0f;
			}
		}

		stbi_image_free(data);

		setData(pixels, width, height);
	}

	void Texture::setData(const std::vector<Color>& pixels, unsigned width, unsigned height)
	{
		if (pixels.size() < width*height)
			ATEMA_ERROR("Required dimension must not be greater than the number of pixels.");

		m_impl->setData(pixels, width, height);
	}

	void Texture::getData(std::vector<Color>& pixels) const
	{
		m_impl->getData(pixels);
	}

	void Texture::resize(unsigned width, unsigned height)
	{
		m_impl->resize(width, height);
	}

	unsigned Texture::getWidth() const
	{
		return m_impl->getWidth();
	}

	unsigned Texture::getHeight() const
	{
		return m_impl->getHeight();
	}
}
