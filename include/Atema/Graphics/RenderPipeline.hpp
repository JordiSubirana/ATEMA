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

#ifndef ATEMA_GRAPHICS_RENDERPIPELINE_HPP
#define ATEMA_GRAPHICS_RENDERPIPELINE_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Math/Vector.hpp>
#include <Atema/Renderer/Enums.hpp>

#include <vector>

namespace at
{
	class Window;
	class SwapChain;
	class RenderPass;
	class CommandPool;
	class CommandBuffer;
	class Image;
	class Framebuffer;
	class Fence;
	class Semaphore;
	
	class ATEMA_GRAPHICS_API RenderPipeline : public NonCopyable
	{
	public:
		struct ATEMA_GRAPHICS_API Settings
		{
			Ptr<Window> window;
			
			uint32_t maxFramesInFlight = 2;
			
			ImageFormat colorFormat = ImageFormat::BGRA8_SRGB;
			ImageFormat depthFormat = ImageFormat::D32F;
			
			std::function<void(const Vector2u&)> resizeCallback;
			std::function<void(uint32_t frameIndex, Ptr<CommandBuffer> commandBuffer)> updateFrameCallback;
		};

		RenderPipeline() = delete;
		RenderPipeline(const Settings& settings);
		virtual ~RenderPipeline();

		void startFrame();

		void beginScreenRenderPass(bool useSecondaryBuffers = false);
		void endScreenRenderPass();
		
		Ptr<Window> getWindow() const noexcept;
		const Ptr<SwapChain>& getSwapChain() const noexcept;
		const Ptr<RenderPass>& getRenderPass() const noexcept;
		const Ptr<Framebuffer>& getCurrentFramebuffer() const noexcept;
		const std::vector<Ptr<CommandPool>>& getCommandPools() const noexcept;
		
	private:
		void createSwapChainResources();
		void destroySwapChainResources();

		WPtr<Window> m_window;
		uint32_t m_maxFramesInFlight;
		ImageFormat m_colorFormat;
		ImageFormat m_depthFormat;
		std::function<void(const Vector2u&)> m_resizeCallback;
		std::function<void(uint32_t frameIndex, Ptr<CommandBuffer> commandBuffer)> m_updateFrameCallback;
		uint32_t m_currentFrame;
		uint32_t m_currentSwapChainImage;
		Ptr<CommandBuffer> m_currentCommandBuffer;
		Ptr<SwapChain> m_swapChain;
		Ptr<RenderPass> m_renderPass;
		Ptr<Image> m_depthImage;
		std::vector<Ptr<Framebuffer>> m_framebuffers;
		std::vector<Ptr<CommandPool>> m_commandPools;
		std::vector<Ptr<CommandBuffer>> m_commandBuffers;
		std::vector<Ptr<Fence>> m_fences;
		std::vector<Ptr<Fence>> m_imageFences;
		std::vector<Ptr<Semaphore>> m_imageAvailableSemaphores;
		std::vector<Ptr<Semaphore>> m_renderFinishedSemaphores;
	};
}

#endif
