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

#ifndef ATEMA_VULKANRENDERER_VULKANRENDERER_HPP
#define ATEMA_VULKANRENDERER_VULKANRENDERER_HPP

#include <Atema/VulkanRenderer/Config.hpp>
#include <Atema/Renderer/Renderer.hpp>
#include <Atema/Renderer/Vertex.hpp>
#include <Atema/VulkanRenderer/Vulkan.hpp>

#include <vector>
#include <array>
#include <unordered_map>
#include <Atema/VulkanRenderer/VulkanDevice.hpp>
#include <Atema/VulkanRenderer/VulkanInstance.hpp>
#include <Atema/VulkanRenderer/VulkanPhysicalDevice.hpp>

namespace at
{
	class ATEMA_VULKANRENDERER_API VulkanRenderer final : public Renderer
	{
	public:
		VulkanRenderer() = delete;
		VulkanRenderer(const Renderer::Settings& settings);
		virtual ~VulkanRenderer();

		static VulkanRenderer& instance();

		const Limits& getLimits() const noexcept override;
		
		// Renderer methods
		void initialize() override;

		void waitForIdle() override;

		Ptr<CommandPool> getDefaultCommandPool() override;
		Ptr<CommandPool> getCommandPool(size_t threadIndex) override;
		
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
		Ptr<Fence> createFence(const Fence::Settings& settings) override;
		Ptr<Semaphore> createSemaphore() override;
		Ptr<Buffer> createBuffer(const Buffer::Settings& settings) override;

		void submit(const std::vector<Ptr<CommandBuffer>>& commandBuffers, const std::vector<Ptr<Semaphore>>& waitSemaphores, const std::vector<Flags<PipelineStage>>& waitStages, const std::vector<Ptr<Semaphore>>& signalSemaphores, Ptr<Fence> fence = nullptr) override;
		SwapChainResult present(const Ptr<SwapChain>& swapChain, uint32_t imageIndex, const std::vector<Ptr<Semaphore>>& waitSemaphores) override;

		// Vulkan specific
		VkSurfaceKHR getWindowSurface(Ptr<Window> window) const;

		const VulkanInstance& getInstance() const noexcept;
		const VulkanPhysicalDevice& getPhysicalDevice() const noexcept;
		const VulkanDevice& getDevice() const noexcept;
		
		uint32_t getGraphicsQueueIndex() const noexcept;
		uint32_t getPresentQueueIndex() const noexcept;

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
		QueueFamilyData getQueueFamilyData(const VulkanPhysicalDevice& device) const;
		SwapChainSupportDetails getSwapChainSupport(const VulkanPhysicalDevice& device) const;
		static bool checkPhysicalDeviceExtensionSupport(const VulkanPhysicalDevice& device);
		static ImageSamples getMaxUsableSampleCount(const VulkanPhysicalDevice& device);
		int getPhysicalDeviceScore(const VulkanPhysicalDevice& device, const QueueFamilyData& queueFamilyData);
		void pickPhysicalDevice();
		void createDevice();
		void createThreadCommandPools();

		void unregisterWindows();

		Limits m_limits;
		UPtr<VulkanInstance> m_instance;
		const VulkanPhysicalDevice* m_physicalDevice;
		UPtr<VulkanDevice> m_device;
		QueueFamilyData m_queueFamilyData;
		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;
		ImageSamples m_maxSamples;
		VkSurfaceKHR m_surface;

		std::unordered_map<Window*, VkSurfaceKHR> m_windowSurfaces;
		
		std::vector<Ptr<CommandPool>> m_threadCommandPools;
	};
}

#endif
