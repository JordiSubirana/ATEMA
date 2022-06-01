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

#ifndef ATEMA_RENDERER_RENDERER_HPP
#define ATEMA_RENDERER_RENDERER_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Window/Window.hpp>
#include <Atema/Renderer/Buffer.hpp>
#include <Atema/Renderer/CommandBuffer.hpp>
#include <Atema/Renderer/CommandPool.hpp>
#include <Atema/Renderer/DescriptorSetLayout.hpp>
#include <Atema/Renderer/Fence.hpp>
#include <Atema/Renderer/Image.hpp>
#include <Atema/Renderer/RenderPass.hpp>
#include <Atema/Renderer/Framebuffer.hpp>
#include <Atema/Renderer/GraphicsPipeline.hpp>
#include <Atema/Renderer/RenderWindow.hpp>
#include <Atema/Renderer/Sampler.hpp>
#include <Atema/Renderer/Semaphore.hpp>
#include <Atema/Renderer/Shader.hpp>

namespace at
{
	class ATEMA_RENDERER_API Renderer
	{
	public:
		struct Settings
		{
			bool sampleShading = true;
		};

		struct Limits
		{
			uint32_t			maxImageDimension1D;
			uint32_t			maxImageDimension2D;
			uint32_t			maxImageDimension3D;
			uint32_t			maxImageDimensionCube;
			uint32_t			maxImageArrayLayers;
			uint32_t			maxTexelBufferElements;
			uint32_t			maxUniformBufferRange;
			uint32_t			maxStorageBufferRange;
			uint32_t			maxPushConstantsSize;
			uint32_t			maxMemoryAllocationCount;
			uint32_t			maxSamplerAllocationCount;
			uint64_t			bufferImageGranularity;
			uint64_t			sparseAddressSpaceSize;
			uint32_t			maxBoundDescriptorSets;
			uint32_t			maxPerStageDescriptorSamplers;
			uint32_t			maxPerStageDescriptorUniformBuffers;
			uint32_t			maxPerStageDescriptorStorageBuffers;
			uint32_t			maxPerStageDescriptorSampledImages;
			uint32_t			maxPerStageDescriptorStorageImages;
			uint32_t			maxPerStageDescriptorInputAttachments;
			uint32_t			maxPerStageResources;
			uint32_t			maxDescriptorSetSamplers;
			uint32_t			maxDescriptorSetUniformBuffers;
			uint32_t			maxDescriptorSetUniformBuffersDynamic;
			uint32_t			maxDescriptorSetStorageBuffers;
			uint32_t			maxDescriptorSetStorageBuffersDynamic;
			uint32_t			maxDescriptorSetSampledImages;
			uint32_t			maxDescriptorSetStorageImages;
			uint32_t			maxDescriptorSetInputAttachments;
			uint32_t			maxVertexInputAttributes;
			uint32_t			maxVertexInputBindings;
			uint32_t			maxVertexInputAttributeOffset;
			uint32_t			maxVertexInputBindingStride;
			uint32_t			maxVertexOutputComponents;
			uint32_t			maxTessellationGenerationLevel;
			uint32_t			maxTessellationPatchSize;
			uint32_t			maxTessellationControlPerVertexInputComponents;
			uint32_t			maxTessellationControlPerVertexOutputComponents;
			uint32_t			maxTessellationControlPerPatchOutputComponents;
			uint32_t			maxTessellationControlTotalOutputComponents;
			uint32_t			maxTessellationEvaluationInputComponents;
			uint32_t			maxTessellationEvaluationOutputComponents;
			uint32_t			maxGeometryShaderInvocations;
			uint32_t			maxGeometryInputComponents;
			uint32_t			maxGeometryOutputComponents;
			uint32_t			maxGeometryOutputVertices;
			uint32_t			maxGeometryTotalOutputComponents;
			uint32_t			maxFragmentInputComponents;
			uint32_t			maxFragmentOutputAttachments;
			uint32_t			maxFragmentDualSrcAttachments;
			uint32_t			maxFragmentCombinedOutputResources;
			uint32_t			maxComputeSharedMemorySize;
			uint32_t			maxComputeWorkGroupCount[3];
			uint32_t			maxComputeWorkGroupInvocations;
			uint32_t			maxComputeWorkGroupSize[3];
			uint32_t			subPixelPrecisionBits;
			uint32_t			subTexelPrecisionBits;
			uint32_t			mipmapPrecisionBits;
			uint32_t			maxDrawIndexedIndexValue;
			uint32_t			maxDrawIndirectCount;
			float				maxSamplerLodBias;
			float				maxSamplerAnisotropy;
			uint32_t			maxViewports;
			uint32_t			maxViewportDimensions[2];
			float				viewportBoundsRange[2];
			uint32_t			viewportSubPixelBits;
			uint64_t			minMemoryMapAlignment;
			uint64_t			minTexelBufferOffsetAlignment;
			uint64_t			minUniformBufferOffsetAlignment;
			uint64_t			minStorageBufferOffsetAlignment;
			int32_t				minTexelOffset;
			uint32_t			maxTexelOffset;
			int32_t				minTexelGatherOffset;
			uint32_t			maxTexelGatherOffset;
			float				minInterpolationOffset;
			float				maxInterpolationOffset;
			uint32_t			subPixelInterpolationOffsetBits;
			uint32_t			maxFramebufferWidth;
			uint32_t			maxFramebufferHeight;
			uint32_t			maxFramebufferLayers;
			Flags<ImageSamples>	framebufferColorSampleCounts;
			Flags<ImageSamples>	framebufferDepthSampleCounts;
			Flags<ImageSamples>	framebufferStencilSampleCounts;
			Flags<ImageSamples>	framebufferNoAttachmentsSampleCounts;
			uint32_t			maxColorAttachments;
			Flags<ImageSamples>	sampledImageColorSampleCounts;
			Flags<ImageSamples>	sampledImageIntegerSampleCounts;
			Flags<ImageSamples>	sampledImageDepthSampleCounts;
			Flags<ImageSamples>	sampledImageStencilSampleCounts;
			Flags<ImageSamples>	storageImageSampleCounts;
			uint32_t			maxSampleMaskWords;
			bool				timestampComputeAndGraphics;
			float				timestampPeriod;
			uint32_t			maxClipDistances;
			uint32_t			maxCullDistances;
			uint32_t			maxCombinedClipAndCullDistances;
			uint32_t			discreteQueuePriorities;
			float				pointSizeRange[2];
			float				lineWidthRange[2];
			float				pointSizeGranularity;
			float				lineWidthGranularity;
			bool				strictLines;
			bool				standardSampleLocations;
			uint64_t			optimalBufferCopyOffsetAlignment;
			uint64_t			optimalBufferCopyRowPitchAlignment;
			uint64_t			nonCoherentAtomSize;
		};
		
		Renderer() = delete;
		virtual ~Renderer();

		template <typename T>
		static Renderer& create(const Settings& settings);

		static Renderer& instance();

		static void destroy();

		static bool isDepthImageFormat(ImageFormat format);

		static bool isStencilImageFormat(ImageFormat format);

		virtual void initialize() = 0;

		const Settings& getSettings() const noexcept;

		virtual const Limits& getLimits() const noexcept = 0;

		virtual void waitForIdle() = 0;

		// Returns the default command pool for a given queue command type
		virtual Ptr<CommandPool> getCommandPool(QueueType queueType) = 0;
		// Returns the default command pool for a given queue command type for the desired thread
		// 'threadIndex' refers to the index of the thread in TaskManager (AtemaCore)
		virtual Ptr<CommandPool> getCommandPool(QueueType queueType, size_t threadIndex) = 0;
		
		// Object creation
		virtual Ptr<Image> createImage(const Image::Settings& settings) = 0;
		virtual Ptr<Sampler> createSampler(const Sampler::Settings& settings) = 0;
		virtual Ptr<RenderPass> createRenderPass(const RenderPass::Settings& settings) = 0;
		virtual Ptr<Framebuffer> createFramebuffer(const Framebuffer::Settings& settings) = 0;
		virtual Ptr<Shader> createShader(const Shader::Settings& settings) = 0;
		virtual Ptr<DescriptorSetLayout> createDescriptorSetLayout(const DescriptorSetLayout::Settings& settings) = 0;
		virtual Ptr<GraphicsPipeline> createGraphicsPipeline(const GraphicsPipeline::Settings& settings) = 0;
		virtual Ptr<CommandPool> createCommandPool(const CommandPool::Settings& settings) = 0;
		virtual Ptr<Fence> createFence(const Fence::Settings& settings) = 0;
		virtual Ptr<Semaphore> createSemaphore() = 0;
		virtual Ptr<Buffer> createBuffer(const Buffer::Settings& settings) = 0;
		virtual Ptr<RenderWindow> createRenderWindow(const RenderWindow::Settings& settings) = 0;

		// Rendering
		virtual void submit(
			const std::vector<Ptr<CommandBuffer>>& commandBuffers,
			const std::vector<WaitCondition>& waitConditions,
			const std::vector<Ptr<Semaphore>>& signalSemaphores,
			Ptr<Fence> fence = nullptr) = 0;

		void submitAndWait(const std::vector<Ptr<CommandBuffer>>& commandBuffers);
		
	protected:
		Renderer(const Settings& settings);

	private:
		static Ptr<Renderer> s_renderer;
		Settings m_settings;
	};

	template <typename T>
	Renderer& Renderer::create(const Settings& settings)
	{
		static_assert(std::is_base_of<Renderer, T>::value, "Invalid Renderer type");

		if (s_renderer)
		{
			ATEMA_ERROR("A Renderer already exists");

			return *s_renderer;
		}

		auto renderer = std::make_shared<T>(settings);
		
		s_renderer = std::static_pointer_cast<Renderer>(renderer);

		s_renderer->initialize();

		return *s_renderer;
	}
}

#endif
