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

#ifndef ATEMA_VULKANRENDERER_VULKANRENDERER_HPP
#define ATEMA_VULKANRENDERER_VULKANRENDERER_HPP

#include <Atema/VulkanRenderer/Config.hpp>
#include <Atema/Renderer/Renderer.hpp>
#include <Atema/Renderer/Vertex.hpp>
#include <Atema/VulkanRenderer/Vulkan.hpp>

#include <vector>
#include <array>
#include <unordered_map>

namespace at
{
	class ATEMA_VULKANRENDERER_API VulkanRenderer final : public Renderer
	{
	public:
		VulkanRenderer() = delete;
		VulkanRenderer(const Renderer::Settings& settings);
		virtual ~VulkanRenderer();

		static VulkanRenderer& instance();

		// Renderer methods
		void initialize() override;

		void waitForIdle() override;
		
		void registerWindow(Ptr<Window> window) override;
		void unregisterWindow(Ptr<Window> window) override;

		Ptr<Image> createImage(const Image::Settings& settings) override;
		Ptr<Sampler> createSampler(const Sampler::Settings& settings) override;
		Ptr<SwapChain> createSwapChain(const SwapChain::Settings& settings) override;
		Ptr<RenderPass> createRenderPass(const RenderPass::Settings& settings) override;
		Ptr<Framebuffer> createFramebuffer(const Framebuffer::Settings& settings) override;
		Ptr<Shader> createShader(const Shader::Settings& settings) override;
		Ptr<DescriptorSetLayout> createDescriptorSetLayout(const DescriptorSetLayout::Settings& settings) override;
		Ptr<DescriptorPool> createDescriptorPool(const DescriptorPool::Settings& settings) override;
		Ptr<GraphicsPipeline> createGraphicsPipeline(const GraphicsPipeline::Settings& settings) override;
		Ptr<CommandPool> createCommandPool(const CommandPool::Settings& settings) override;
		Ptr<CommandBuffer> createCommandBuffer(const CommandBuffer::Settings& settings) override;
		Ptr<Fence> createFence(const Fence::Settings& settings) override;
		Ptr<Semaphore> createSemaphore() override;
		Ptr<Buffer> createBuffer(const Buffer::Settings& settings) override;

		void submit(const std::vector<Ptr<CommandBuffer>>& commandBuffers, const std::vector<Ptr<Semaphore>>& waitSemaphores, const std::vector<Flags<PipelineStage>>& waitStages, const std::vector<Ptr<Semaphore>>& signalSemaphores, Ptr<Fence> fence = nullptr) override;
		SwapChainResult present(const Ptr<SwapChain>& swapChain, uint32_t imageIndex, const std::vector<Ptr<Semaphore>>& waitSemaphores) override;

		// Vulkan specific
		VkSurfaceKHR getWindowSurface(Ptr<Window> window) const;

		VkInstance getInstanceHandle() const noexcept;
		VkPhysicalDevice getPhysicalDeviceHandle() const noexcept;
		VkDevice getLogicalDeviceHandle() const noexcept;
		
		uint32_t getGraphicsQueueIndex() const noexcept;
		uint32_t getPresentQueueIndex() const noexcept;
		
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		
	private:
		struct QueueFamilyData
		{
			QueueFamilyData() :
				hasGraphics(false), graphicsIndex(0),
				hasPresent(false), presentIndex(0)
			{
			}

			bool isComplete() const
			{
				return hasGraphics && hasPresent;
			}

			bool hasGraphics;
			uint32_t graphicsIndex;
			bool hasPresent;
			uint32_t presentIndex;
		};

		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		bool checkValidationLayerSupport();
		void createInstance();
		void createSurface();
		QueueFamilyData getQueueFamilyData(VkPhysicalDevice device);
		SwapChainSupportDetails getSwapChainSupport(VkPhysicalDevice device);
		bool checkPhysicalDeviceExtensionSupport(VkPhysicalDevice device);
		ImageSamples getMaxUsableSampleCount(VkPhysicalDevice device);
		int getPhysicalDeviceScore(VkPhysicalDevice device, const QueueFamilyData& queueFamilyData);
		void getPhysicalDevice();
		void createDevice();

		void unregisterWindows();
		
		void destroy();
		void destroyInstance();
		void destroyDevice();
		
		VkInstance m_instance;
		UPtr<Vulkan> m_vulkan;
		VkPhysicalDevice m_physicalDevice;
		QueueFamilyData m_queueFamilyData;
		VkDevice m_device;
		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;
		ImageSamples m_maxSamples;
		VkSurfaceKHR m_surface;

		std::unordered_map<Window*, VkSurfaceKHR> m_windowSurfaces;
	};
}

#endif
