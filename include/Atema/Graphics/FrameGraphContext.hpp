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

#ifndef ATEMA_GRAPHICS_FRAMEGRAPHCONTEXT_HPP
#define ATEMA_GRAPHICS_FRAMEGRAPHCONTEXT_HPP

#include <unordered_map>
#include <Atema/Graphics/Config.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Graphics/FrameGraphTexture.hpp>
#include <Atema/Renderer/CommandBuffer.hpp>
#include <Atema/Renderer/RenderFrame.hpp>

namespace at
{
	class ImageView;
	class RenderFrame;

	class ATEMA_GRAPHICS_API FrameGraphContext : public NonCopyable
	{
	public:
		FrameGraphContext() = delete;
		FrameGraphContext(
			RenderFrame& renderFrame,
			CommandBuffer& commandBuffer,
			std::unordered_map<FrameGraphTextureHandle, WPtr<Image>>& textureMap,
			std::unordered_map<FrameGraphTextureHandle, WPtr<ImageView>>& viewMap,
			Ptr<RenderPass> renderPass,
			Ptr<Framebuffer> framebuffer);
		~FrameGraphContext();

		size_t getFrameIndex() const noexcept;

		CommandBuffer& getCommandBuffer() const noexcept;

		Ptr<Image> getTexture(FrameGraphTextureHandle textureHandle) const;
		Ptr<ImageView> getImageView(FrameGraphTextureHandle textureHandle) const;

		// Creates a secondary command buffer to use within the current pass
		// This methods calls CommandBuffer::beginSecondaryCommandBuffer
		// The caller must call CommandBuffer::end after recording all needed commands
		// The command buffer is temporary, and will be automatically destroyed after the frame (no need to call destroyAfterUse)
		Ptr<CommandBuffer> createSecondaryCommandBuffer();
		// Overload of createSecondaryCommandBuffer()
		Ptr<CommandBuffer> createSecondaryCommandBuffer(size_t threadIndex);

		template <typename T>
		void destroyAfterUse(T&& resource);

	private:
		RenderFrame& m_renderFrame;
		CommandBuffer& m_commandBuffer;
		std::unordered_map<FrameGraphTextureHandle, WPtr<Image>>& m_textureMap;
		std::unordered_map<FrameGraphTextureHandle, WPtr<ImageView>>& m_viewMap;
		Ptr<RenderPass> m_renderPass;
		Ptr<Framebuffer> m_framebuffer;
	};
}

#include <Atema/Graphics/FrameGraphContext.inl>

#endif
