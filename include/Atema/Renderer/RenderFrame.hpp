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

#ifndef ATEMA_RENDERER_RENDERFRAME_HPP
#define ATEMA_RENDERER_RENDERFRAME_HPP

#include <Atema/Core/Pointer.hpp>
#include <Atema/Renderer/CommandBuffer.hpp>
#include <Atema/Renderer/Config.hpp>
#include <Atema/Renderer/Enums.hpp>
#include <Atema/Renderer/Fence.hpp>
#include <Atema/Renderer/Semaphore.hpp>
#include <Atema/Renderer/BufferPool.hpp>

#include <shared_mutex>

namespace at
{
	class ATEMA_RENDERER_API RenderFrame : public NonCopyable
	{
		friend class RenderWindow;

	public:
		RenderFrame();
		~RenderFrame();

		virtual size_t getFrameIndex() const noexcept = 0;

		virtual Ptr<CommandBuffer> createCommandBuffer(const CommandBuffer::Settings& settings, QueueType queueType) = 0;
		virtual Ptr<CommandBuffer> createCommandBuffer(const CommandBuffer::Settings& settings, QueueType queueType, size_t threadIndex) = 0;

		// Create transient staging buffer that will be destroyed when the frame ends
		BufferRange createStagingBuffer(size_t byteSize);

		virtual Ptr<RenderPass> getRenderPass() const noexcept = 0;
		virtual Ptr<Framebuffer> getFramebuffer() const noexcept = 0;

		virtual Ptr<Semaphore> getImageAvailableSemaphore() const noexcept = 0;
		virtual Ptr<Semaphore> getRenderFinishedSemaphore() const noexcept = 0;

		virtual Ptr<Fence> getFence() const noexcept = 0;

		WaitCondition getImageAvailableWaitCondition() const noexcept;

		virtual void submit(
			const std::vector<Ptr<CommandBuffer>>& commandBuffers,
			const std::vector<WaitCondition>& waitConditions,
			const std::vector<Ptr<Semaphore>>& signalSemaphores,
			Ptr<Fence> fence = nullptr) = 0;

		virtual void present() = 0;

		template <typename T>
		void destroyAfterUse(T&& resource);

	protected:
		void destroyResources();
		virtual void initializeFrame();

	private:
		class AbstractResourceHandler
		{
		public:
			AbstractResourceHandler() = default;
			virtual ~AbstractResourceHandler() = default;
		};

		template <typename T>
		class ResourceHandler : public AbstractResourceHandler
		{
		public:
			ResourceHandler() = delete;
			ResourceHandler(T&& resource);
			virtual ~ResourceHandler() = default;

		private:
			T resource;
		};

		std::vector<Ptr<AbstractResourceHandler>> m_resources;
		std::shared_mutex m_resourceMutex;
		BufferPool m_stagingBufferPool;
	};
}

#include <Atema/Renderer/RenderFrame.inl>

#endif
