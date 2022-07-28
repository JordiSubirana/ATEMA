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

#ifndef ATEMA_VULKANRENDERER_VULKANRENDERWINDOW_HPP
#define ATEMA_VULKANRENDERER_VULKANRENDERWINDOW_HPP

#include <Atema/VulkanRenderer/Config.hpp>
#include <Atema/Renderer/RenderWindow.hpp>
#include <Atema/VulkanRenderer/Vulkan.hpp>
#include <Atema/Core/SparseSet.hpp>

namespace at
{
	class Fence;
	class Image;
	class Framebuffer;
	class RenderPass;
	class VulkanRenderFrame;
	class VulkanSwapChain;

	class ATEMA_VULKANRENDERER_API VulkanRenderWindow : public RenderWindow
	{
	public:
		VulkanRenderWindow() = delete;
		VulkanRenderWindow(VulkanDevice& device, const RenderWindow::Settings& settings);
		~VulkanRenderWindow();

		VulkanDevice& getDevice() noexcept;

		ImageFormat getColorFormat() const noexcept override;
		ImageFormat getDepthFormat() const noexcept override;

		size_t getMaxFramesInFlight() const noexcept override;
		RenderFrame& acquireFrame() override;

		VkSurfaceKHR getSurface() const noexcept;

		uint32_t getPresentQueueFamilyIndex() const noexcept;

		Ptr<RenderPass> getRenderPass() const noexcept override;
		Ptr<Framebuffer> getFramebuffer(size_t imageIndex) const noexcept;

		const VulkanSwapChain& getSwapChain() const;

		void present(const VulkanRenderFrame& renderFrame);

	private:
		void onResize(const Vector2u& newSize) override;

		void createSurface();
		void destroySurface();

		void initializePresentQueue();

		void createRenderPass();

		void createSwapChainResources();
		void destroySwapChainResources();

		VulkanDevice& m_device;

		ImageFormat m_colorFormat;
		ImageFormat m_depthFormat;

		VkSurfaceKHR m_surface;

		Ptr<RenderPass> m_renderPass;

		std::vector<Ptr<Framebuffer>> m_framebuffers;

		uint32_t m_presentQueueFamilyIndex;
		VkQueue m_presentQueue;

		Ptr<VulkanSwapChain> m_swapChain;
		uint32_t m_currentSwapChainImage;
		bool m_recreateSwapChain;
		Ptr<Image> m_depthImage;
		std::vector<Ptr<Fence>> m_imageFences;

		size_t m_frameCount;
		size_t m_currentFrameIndex;
		std::vector<Ptr<VulkanRenderFrame>> m_renderFrames;
	};
}

#endif
