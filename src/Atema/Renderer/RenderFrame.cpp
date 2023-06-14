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

#include <Atema/Renderer/RenderFrame.hpp>

namespace
{
	// 1MB by default
	constexpr size_t StagingBufferBlockSize = 1048576;
}

using namespace at;

RenderFrame::RenderFrame() :
	m_stagingBufferPool(BufferUsage::TransferSrc | BufferUsage::Map, StagingBufferBlockSize)
{
}

RenderFrame::~RenderFrame()
{
	destroyResources();
}

WaitCondition RenderFrame::getImageAvailableWaitCondition() const noexcept
{
	WaitCondition waitCondition;
	waitCondition.semaphore = getImageAvailableSemaphore();
	waitCondition.pipelineStages = PipelineStage::ColorAttachmentOutput;

	return waitCondition;
}

void RenderFrame::destroyResources()
{
	std::lock_guard lockGuard(m_resourceMutex);

	m_resources.clear();
}

void RenderFrame::initializeFrame()
{
	m_stagingBufferPool.clear();
}

BufferRange RenderFrame::createStagingBuffer(size_t byteSize)
{
	return m_stagingBufferPool.create(byteSize);
}
