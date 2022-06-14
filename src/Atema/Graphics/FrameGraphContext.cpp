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

#include <Atema/Graphics/FrameGraphContext.hpp>
#include <Atema/Renderer/RenderFrame.hpp>

using namespace at;

FrameGraphContext::FrameGraphContext(
	RenderFrame& renderFrame,
	CommandBuffer& commandBuffer,
	std::unordered_map<FrameGraphTextureHandle, WPtr<Image>>& textureMap,
	Ptr<RenderPass> renderPass,
	Ptr<Framebuffer> framebuffer) :
	NonCopyable(),
	m_renderFrame(renderFrame),
	m_commandBuffer(commandBuffer),
	m_textureMap(textureMap),
	m_renderPass(renderPass),
	m_framebuffer(framebuffer)
{
}

FrameGraphContext::~FrameGraphContext()
{
}

size_t FrameGraphContext::getFrameIndex() const noexcept
{
	return m_renderFrame.getFrameIndex();
}

CommandBuffer& FrameGraphContext::getCommandBuffer() const noexcept
{
	return m_commandBuffer;
}

Ptr<Image> FrameGraphContext::getTexture(FrameGraphTextureHandle textureHandle) const
{
	return m_textureMap.at(textureHandle).lock();
}

Ptr<CommandBuffer> FrameGraphContext::createSecondaryCommandBuffer()
{
	auto commandBuffer = m_renderFrame.createCommandBuffer({ true, true }, QueueType::Graphics);

	commandBuffer->beginSecondary(m_renderPass, m_framebuffer);

	auto commandBufferResource = commandBuffer;

	destroyAfterUse(std::move(commandBufferResource));

	return commandBuffer;
}

Ptr<CommandBuffer> FrameGraphContext::createSecondaryCommandBuffer(size_t threadIndex)
{
	auto commandBuffer = m_renderFrame.createCommandBuffer({ true, true }, QueueType::Graphics, threadIndex);

	commandBuffer->beginSecondary(m_renderPass, m_framebuffer);

	auto commandBufferResource = commandBuffer;

	destroyAfterUse(std::move(commandBufferResource));

	return commandBuffer;
}
