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

#ifndef ATEMA_GRAPHICS_FRAMEGRAPHTEXTURE_HPP
#define ATEMA_GRAPHICS_FRAMEGRAPHTEXTURE_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Renderer/Enums.hpp>

namespace at
{
	using FrameGraphTextureHandle = size_t;

	struct ATEMA_GRAPHICS_API FrameGraphTextureSettings
	{
		FrameGraphTextureSettings();
		FrameGraphTextureSettings(uint32_t width, uint32_t height, ImageFormat format);

		uint32_t width;
		uint32_t height;
		ImageFormat format; // Default ImageFormat::RGBA8_UNORM
	};
}

#endif
