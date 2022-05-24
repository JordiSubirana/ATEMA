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

#ifndef ATEMA_VULKANRENDERER_VULKANRENDERFRAME_HPP
#define ATEMA_VULKANRENDERER_VULKANRENDERFRAME_HPP

#include <Atema/VulkanRenderer/Config.hpp>
#include <Atema/Renderer/RenderFrame.hpp>
#include <Atema/VulkanRenderer/VulkanFence.hpp>
#include <Atema/VulkanRenderer/VulkanSemaphore.hpp>
#include <Atema/Core/SparseSet.hpp>

namespace at
{
	class VulkanRenderWindow;

	class ATEMA_VULKANRENDERER_API VulkanRenderFrame : public RenderFrame
	{
	public:
		VulkanRenderFrame() = delete;
		VulkanRenderFrame(VulkanRenderWindow& renderWindow, size_t frameIndex);
		~VulkanRenderFrame();

		void wait();

		void setImageIndex(uint32_t imageIndex);
		uint32_t getImageIndex() const noexcept;

		size_t getFrameIndex() const noexcept override;

		Ptr<CommandBuffer> createCommandBuffer(const CommandBuffer::Settings& settings, QueueType queueType) override;
		Ptr<CommandBuffer> createCommandBuffer(const CommandBuffer::Settings& settings, QueueType queueType, size_t threadIndex) override;

		Ptr<RenderPass> getRenderPass() const noexcept override;
		Ptr<Framebuffer> getFramebuffer() const noexcept override;

		Ptr<Semaphore> getImageAvailableSemaphore() const noexcept override;
		Ptr<Semaphore> getRenderFinishedSemaphore() const noexcept override;
		Ptr<Fence> getFence() const noexcept override;

		VkSemaphore getImageAvailableSemaphoreHandle() const;
		VkSemaphore getRenderFinishedSemaphoreHandle() const;

		void submit(
			const std::vector<Ptr<CommandBuffer>>& commandBuffers,
			const std::vector<WaitCondition>& waitConditions,
			const std::vector<Ptr<Semaphore>>& signalSemaphores,
			Ptr<Fence> fence) override;

		void present() override;

	private:
		std::vector<Ptr<CommandPool>>& getCommandPools(QueueType queueType);

		VulkanDevice& m_device;
		VulkanRenderWindow& m_renderWindow;
		Ptr<VulkanFence> m_fence;
		Ptr<VulkanSemaphore> m_imageAvailableSemaphore;
		Ptr<VulkanSemaphore> m_renderFinishedSemaphore;
		std::vector<std::vector<Ptr<CommandPool>>> m_commandPools;
		uint32_t m_imageIndex;
		size_t m_frameIndex;
	};
}

#endif
