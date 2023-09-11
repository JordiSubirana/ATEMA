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

#ifndef ATEMA_GRAPHICS_RENDERRESOURCEMANAGER_HPP
#define ATEMA_GRAPHICS_RENDERRESOURCEMANAGER_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Renderer/BufferPool.hpp>
#include <Atema/Graphics/RenderContext.hpp>

#include <unordered_map>

namespace at
{
	class ATEMA_GRAPHICS_API RenderResourceManager
	{
	public:
		RenderResourceManager();
		// Minimum : 256B (will be set to this value if it is less than it)
		// Default : 1MB
		RenderResourceManager(size_t bufferPoolPageSize);
		RenderResourceManager(const RenderResourceManager& other) = delete;
		RenderResourceManager(RenderResourceManager&& other) noexcept = delete;
		virtual ~RenderResourceManager() = default;

		Ptr<BufferAllocation> createBuffer(const Buffer::Settings& settings);
		
		void beginTransfer(CommandBuffer& commandBuffer, RenderContext& renderContext);

		// Only valid between beginTransfer & endTransfer
		CommandBuffer& getCommandBuffer() const;

		// Only valid between beginTransfer & endTransfer
		RenderContext& getRenderContext() const;

		// Returns a writable memory block used to fill the buffer allocation
		// For device local buffers, uninitialized staging buffers will be created
		void* mapBuffer(BufferAllocation& bufferAllocation);

		// Returns a writable memory block used to fill the buffer
		// For device local buffers, uninitialized staging buffers will be created
		// Size of 0 means the whole buffer
		void* mapBuffer(Buffer& buffer, size_t offset = 0, size_t size = 0);

		void endTransfer();

		RenderResourceManager& operator=(const RenderResourceManager& other) = delete;
		RenderResourceManager& operator=(RenderResourceManager&& other) noexcept = delete;

	private:
		struct Range
		{
			Range();
			Range(size_t offset, size_t size);

			size_t offset;
			size_t size;
		};

		struct StagingData
		{
			Ptr<BufferAllocation> stagingBuffer;

			std::vector<Range> ranges;
		};

		BufferPool& getBufferPool(const Flags<BufferUsage>& usages);
		StagingData& createBufferStagingData(BufferAllocation& bufferAllocation);
		StagingData& createBufferStagingData(Buffer& buffer);
		StagingData& getBufferStagingData(BufferAllocation& bufferAllocation);
		StagingData& getBufferStagingData(Buffer& buffer);
		static void* prepareTransfer(StagingData& stagingData, size_t offset, size_t size);

		// This method will be called once at the end of the update process
		// It can be overridden to group different update calls previously registered by updateXXX methods
		// Does nothing by default
		void updateResources();

		void destroyPendingResources();
		
		CommandBuffer* m_commandBuffer;
		RenderContext* m_renderContext;

		size_t m_bufferPageSize;

		std::unordered_map<Flags<BufferUsage>, UPtr<BufferPool>> m_bufferPools;
		std::unordered_map<Buffer*, UPtr<StagingData>> m_bufferStagingData;
	};
}

#endif
