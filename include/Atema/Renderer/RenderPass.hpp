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

#ifndef ATEMA_RENDERER_RENDERPASS_HPP
#define ATEMA_RENDERER_RENDERPASS_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Renderer/Enums.hpp>

#include <vector>

namespace at
{
	struct ATEMA_RENDERER_API AttachmentDescription
	{
		AttachmentDescription() = default;

		ImageFormat format = ImageFormat::RGBA8_SRGB;
		ImageSamples samples = ImageSamples::S1;
		AttachmentLoading loading = AttachmentLoading::Clear;
		AttachmentStoring storing = AttachmentStoring::Store;
		ImageLayout initialLayout = ImageLayout::Undefined;
		ImageLayout finalLayout = ImageLayout::Attachment;
	};
	
	class ATEMA_RENDERER_API RenderPass : public NonCopyable
	{
	public:
		static constexpr uint32_t UnusedAttachment = std::numeric_limits<uint32_t>::max();
		
		struct SubpassSettings
		{
			// Shader input attachments
			// The array index must match shader input location
			// Use RenderPass::UnusedAttachment for unused input locations
			std::vector<uint32_t> input;
			
			// Shader output color attachments
			// The array index must match shader output location
			// Use RenderPass::UnusedAttachment for unused color locations
			std::vector<uint32_t> color;

			// Resolve attachments
			// The array index must match output index
			// Use RenderPass::UnusedAttachment for unused resolve indices
			std::vector<uint32_t> resolve;

			// Depth/Stencil output attachment (Unused by default)
			uint32_t depthStencil = RenderPass::UnusedAttachment;
		};

		struct ExternalBarrier
		{
			size_t subpassIndex = 0;

			Flags<PipelineStage> srcPipelineStages;
			Flags<MemoryAccess> srcMemoryAccesses;

			Flags<PipelineStage> dstPipelineStages;
			Flags<MemoryAccess> dstMemoryAccesses;
		};
		
		struct Settings
		{
			std::vector<AttachmentDescription> attachments;
			std::vector<SubpassSettings> subpasses;

			// External barriers to synchronize attachments with the first subpass using them
			std::vector<ExternalBarrier> inputBarriers;
			// External barriers to synchronize attachments from the last subpass using them to the next user operation
			std::vector<ExternalBarrier> outputBarriers;
		};
		
		virtual ~RenderPass();

		static Ptr<RenderPass> create(const Settings& settings);

		virtual const std::vector<AttachmentDescription>& getAttachments() const noexcept = 0;

	protected:
		RenderPass();
	};
}

#endif
