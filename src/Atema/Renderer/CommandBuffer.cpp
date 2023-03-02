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

#include <Atema/Renderer/CommandBuffer.hpp>
#include <Atema/Renderer/Renderer.hpp>

using namespace at;

CommandBuffer::CommandBuffer(QueueType queueType) :
	m_queueType(queueType)
{
}

CommandBuffer::~CommandBuffer()
{
}

QueueType CommandBuffer::getQueueType() const noexcept
{
	return m_queueType;
}

void CommandBuffer::imageBarrier(const Image& image, ImageBarrier barrier)
{
	Flags<PipelineStage> srcPipelineStages;
	Flags<MemoryAccess> srcMemoryAccesses;
	ImageLayout srcLayout = ImageLayout::Undefined;

	Flags<PipelineStage> dstPipelineStages;
	Flags<MemoryAccess> dstMemoryAccesses;
	ImageLayout dstLayout = ImageLayout::Undefined;

	switch (barrier)
	{
		case ImageBarrier::InitializeTransferDst:
		{
			srcLayout = ImageLayout::Undefined;
			dstLayout = ImageLayout::TransferDst;

			// We don't need to wait : earliest possible pipeline stage
			srcPipelineStages = PipelineStage::TopOfPipe;
			// Not a real stage for graphics & compute pipelines
			dstPipelineStages = PipelineStage::Transfer;

			// We don't need to wait : empty access mask
			srcMemoryAccesses = 0;
			dstMemoryAccesses = MemoryAccess::TransferWrite;

			break;
		}
		case ImageBarrier::TransferDstToFragmentShaderRead:
		{
			srcLayout = ImageLayout::TransferDst;
			dstLayout = ImageLayout::ShaderRead;

			srcPipelineStages = PipelineStage::Transfer;
			dstPipelineStages = PipelineStage::FragmentShader;

			srcMemoryAccesses = MemoryAccess::TransferWrite;
			dstMemoryAccesses = MemoryAccess::ShaderRead;

			break;
		}
		default:
		{
			ATEMA_ERROR("Invalid ImageBarrier");
			return;
		}
	}

	this->imageBarrier(image, srcPipelineStages, srcMemoryAccesses, srcLayout, dstPipelineStages, dstMemoryAccesses, dstLayout);
}
