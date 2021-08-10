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
	struct Vec2f
	{
		Vec2f() : x(0.0f), y(0.0f) {}
		Vec2f(float x, float y) : x(x), y(y) {}

		float x, y;

		bool operator==(const Vec2f& other) const
		{
			return x == other.x && y == other.y;
		}
	};

	struct Vec3f
	{
		Vec3f() : x(0.0f), y(0.0f), z(0.0f) {}
		Vec3f(float x, float y, float z) : x(x), y(y), z(z) {}

		float x, y, z;

		bool operator==(const Vec3f& other) const
		{
			return x == other.x && y == other.y && z == other.z;
		}
	};

	struct BaseVertex
	{
		Vec3f pos;
		Vec3f color;
		Vec2f texCoord;

		bool operator==(const BaseVertex& other) const
		{
			return pos == other.pos && color == other.color && texCoord == other.texCoord;
		}

		static VkVertexInputBindingDescription getBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(BaseVertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(BaseVertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(BaseVertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(BaseVertex, texCoord);

			return attributeDescriptions;
		}
	};

	

	class ATEMA_VULKANRENDERER_API VulkanRenderer final : public Renderer
	{
	public:
		VulkanRenderer() = delete;
		VulkanRenderer(const Renderer::Settings& settings);
		virtual ~VulkanRenderer();

		static VulkanRenderer& getInstance();
		
		void initialize() override;

		void drawFrame();

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
		VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice device);
		int getPhysicalDeviceScore(VkPhysicalDevice device, const QueueFamilyData& queueFamilyData);
		void getPhysicalDevice();
		void createDevice();
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		void createSwapChain();
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
		void createImageViews();
		void createRenderPass();
		VkShaderModule createShaderModule(const std::vector<char>& code);
		void createDescriptorSetLayout();
		void createGraphicsPipeline();
		void createFramebuffers();
		void createCommandPool();
		void createCommandBuffers();
		void createSemaphores();
		void createFences();
		void createBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void createImage(VkImage& image, VkDeviceMemory& imageMemory, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits sampleCount, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
		VkFormat findDepthFormat() const;
		bool hasStencilComponent(VkFormat format);
		void createColorResources();
		void createDepthResources();
		void createTextureImage();
		void createTextureImageView();
		void createTextureSampler();
		void loadModel();
		void createVertexBuffer();
		void createIndexBuffer();
		void createUniformBuffers();
		void createDescriptorPool();
		void createDescriptorSets();

		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);

		void recreateSwapChain();
		void destroySwapChainResources();

		void loop();

		void updateUniformBuffer(uint32_t index);

		void destroy();
		void destroyInstance();
		void destroySurface();
		void destroyDevice();
		void destroySwapChain();
		void destroyImageViews();
		void destroyRenderPass();
		void destroyDescriptorSetLayout();
		void destroyDescriptorPool();
		void destroyGraphicsPipeline();
		void destroyFramebuffers();
		void destroyCommandPool();
		void destroyCommandBuffers();
		void destroySemaphores();
		void destroyFences();
		void destroyColorResources();
		void destroyDepthResources();
		void destroyTextureImage();
		void destroyTextureImageView();
		void destroyTextureSampler();
		void destroyVertexBuffer();
		void destroyIndexBuffer();
		void destroyUniformBuffers();

	public:
		VkInstance getInstanceHandle() const noexcept;
		VkPhysicalDevice getPhysicalDeviceHandle() const noexcept;
		VkDevice getLogicalDeviceHandle() const noexcept;

		void registerWindow(Ptr<Window> window) override;
		void unregisterWindow(Ptr<Window> window) override;
		void unregisterWindows();
		VkSurfaceKHR getWindowSurface(Ptr<Window> window) const;
		
		uint32_t getGraphicsQueueIndex() const noexcept;
		uint32_t getPresentQueueIndex() const noexcept;
		
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	public:
		Ptr<Image> createImage(const Image::Settings& settings) override;
		Ptr<SwapChain> createSwapChain(const SwapChain::Settings& settings) override;
		Ptr<RenderPass> createRenderPass(const RenderPass::Settings& settings) override;
		Ptr<Framebuffer> createFramebuffer(const Framebuffer::Settings& settings) override;
		Ptr<Shader> createShader(const Shader::Settings& settings) override;
		Ptr<DescriptorSetLayout> createDescriptorSetLayout(const DescriptorSetLayout::Settings& settings) override;
		Ptr<GraphicsPipeline> createGraphicsPipeline(const GraphicsPipeline::Settings& settings) override;
		
	private:
		size_t m_currentFrame;
		bool m_framebufferResized;

		VkInstance m_instance;
		UPtr<Vulkan> m_vulkan;
		VkSurfaceKHR m_surface;
		VkPhysicalDevice m_physicalDevice;
		QueueFamilyData m_queueFamilyData;
		VkDevice m_device;
		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;
		VkSwapchainKHR m_swapChain;
		std::vector<VkImage> m_swapChainImages;
		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;
		std::vector<VkImageView> m_swapChainImageViews;
		VkRenderPass m_renderPass;
		VkDescriptorSetLayout m_descriptorSetLayout;
		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_graphicsPipeline;
		std::vector<VkFramebuffer> m_swapChainFramebuffers;
		VkCommandPool m_commandPool;
		std::vector<VkCommandBuffer> m_commandBuffers;
		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;
		std::vector<VkFence> m_imagesInFlight;

		VkBuffer m_vertexBuffer;
		VkDeviceMemory m_vertexBufferMemory;
		VkBuffer m_indexBuffer;
		VkDeviceMemory m_indexBufferMemory;

		std::vector<BaseVertex> m_modelVertices;
		std::vector<uint32_t> m_modelIndices;

		std::vector<VkBuffer> m_uniformBuffers;
		std::vector<VkDeviceMemory> m_uniformBuffersMemory;

		VkDescriptorPool m_descriptorPool;
		std::vector<VkDescriptorSet> m_descriptorSets;

		uint32_t m_textureMipLevels;
		VkImage m_textureImage;
		VkDeviceMemory m_textureImageMemory;
		VkImageView m_textureImageView;
		VkSampler m_textureSampler;

		VkSampleCountFlagBits m_msaaSamples;
		VkImage m_colorImage;
		VkDeviceMemory m_colorImageMemory;
		VkImageView m_colorImageView;

		VkImage m_depthImage;
		VkDeviceMemory m_depthImageMemory;
		VkImageView m_depthImageView;

		std::unordered_map<Window*, VkSurfaceKHR> m_windowSurfaces;
	};
}

#endif
