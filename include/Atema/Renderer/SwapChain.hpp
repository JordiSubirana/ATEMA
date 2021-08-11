/*
	Copyright 2021 Jordi SUBIRANA

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

#ifndef ATEMA_RENDERER_SWAPCHAIN_HPP
#define ATEMA_RENDERER_SWAPCHAIN_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Renderer/Enums.hpp>

#include <vector>

namespace at
{
	class Window;
	class Image;
	class Fence;
	class Semaphore;
	
	class ATEMA_RENDERER_API SwapChain
	{
	public:
		struct Settings
		{
			Ptr<Window> window;
			ImageFormat format;
		};

		enum class AcquireResult
		{
			// Success
			Success,
			// Fence/Semaphore/Query not completed yet
			NotReady,
			// Can still be used but the swapchain and the window surface properties don't match
			Suboptimal,
			// Need to recreate the swapchain
			OutOfDate,
			// Unknown error
			Error
		};
		
		virtual ~SwapChain();

		static Ptr<SwapChain> create(const Settings& settings);

		virtual std::vector<Ptr<Image>>& getImages() noexcept = 0;
		virtual const std::vector<Ptr<Image>>& getImages() const noexcept = 0;

		virtual AcquireResult acquireNextImage(uint32_t& imageIndex, const Ptr<Fence>& fence) = 0;
		virtual AcquireResult acquireNextImage(uint32_t& imageIndex, const Ptr<Semaphore>& semaphore) = 0;
		
	protected:
		SwapChain();
	};
}

#endif
