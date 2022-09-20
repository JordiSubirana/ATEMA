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

#ifndef ATEMA_RENDERER_IMAGE_HPP
#define ATEMA_RENDERER_IMAGE_HPP

#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Math/Vector.hpp>
#include <Atema/Renderer/Config.hpp>
#include <Atema/Renderer/Enums.hpp>

namespace at
{
	class ImageView;

	class ATEMA_RENDERER_API Image : public NonCopyable
	{
	public:
		struct Settings
		{
			uint32_t width = 8;
			uint32_t height = 8;
			uint32_t layers = 1;
			uint32_t mipLevels = 1;

			ImageFormat format = ImageFormat::RGBA8_UNORM;
			ImageSamples samples = ImageSamples::S1;
			ImageTiling tiling = ImageTiling::Optimal;
			Flags<ImageUsage> usages = ImageUsage::All;
		};
		
		virtual ~Image();

		static Ptr<Image> create(const Settings& settings);

		// Returns a view containing all the required layers and mip levels
		// If layerCount is 0, then the view contains all the remaining layers
		// If mipLevelCount is 0, then the view contains all the remaining mip levels
		virtual Ptr<ImageView> getView(uint32_t baseLayer = 0, uint32_t layerCount = 0, uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 0) = 0;

		virtual ImageFormat getFormat() const noexcept = 0;

		virtual Vector2u getSize() const noexcept = 0;

		virtual uint32_t getLayers() const noexcept = 0;

		virtual uint32_t getMipLevels() const noexcept = 0;
		
	protected:
		Image();
	};
}

#endif
