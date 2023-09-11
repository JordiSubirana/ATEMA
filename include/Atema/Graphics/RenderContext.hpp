/*
	Copyright 2023 Jordi SUBIRANA

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

#ifndef ATEMA_GRAPHICS_RENDERCONTEXT_HPP
#define ATEMA_GRAPHICS_RENDERCONTEXT_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Renderer/CommandBuffer.hpp>
#include <Atema/Renderer/BufferPool.hpp>

#include <vector>

namespace at
{
	class CommandPool;

	class ATEMA_GRAPHICS_API RenderContext
	{
	public:
		RenderContext();
		RenderContext(const RenderContext& other) = delete;
		RenderContext(RenderContext&& other) noexcept = delete;
		~RenderContext() = default;

		Ptr<CommandBuffer> createCommandBuffer(const CommandBuffer::Settings& settings, QueueType queueType);
		Ptr<CommandBuffer> createCommandBuffer(const CommandBuffer::Settings& settings, QueueType queueType, size_t threadIndex);

		Ptr<BufferAllocation> createStagingBuffer(size_t byteSize);

		template <typename T>
		void destroyAfterUse(T&& resource);

		// Destroys every resource used during previous rendering
		// This includes CommandBuffers, StagingBuffers, and every user specified resource
		// The user must ensure the rendering is finished (using a Fence for example)
		void destroyPendingResources();

		RenderContext& operator=(const RenderContext& other) = delete;
		RenderContext& operator=(RenderContext&& other) noexcept = delete;

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

		std::vector<Ptr<CommandPool>>& getCommandPools(QueueType queueType);

		std::vector<std::vector<Ptr<CommandPool>>> m_commandPools;

		BufferPool m_stagingBufferPool;

		std::vector<Ptr<AbstractResourceHandler>> m_resources;
		std::shared_mutex m_resourceMutex;
	};
}

#include <Atema/Graphics/RenderContext.inl>

#endif
