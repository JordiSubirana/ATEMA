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

#ifndef ATEMA_VULKANRENDERER_VULKANRENDERPASS_HPP
#define ATEMA_VULKANRENDERER_VULKANRENDERPASS_HPP

#include <Atema/VulkanRenderer/Config.hpp>
#include <Atema/Renderer/RenderPass.hpp>
#include <Atema/VulkanRenderer/Vulkan.hpp>
#include <Atema/Core/Signal.hpp>

namespace at
{
	class ATEMA_VULKANRENDERER_API VulkanRenderPass final : public RenderPass
	{
	public:
		VulkanRenderPass() = delete;
		VulkanRenderPass(const VulkanDevice& device, const RenderPass::Settings& settings);
		virtual ~VulkanRenderPass();

		VkRenderPass getHandle() const noexcept;

		const std::vector<AttachmentDescription>& getAttachments() const noexcept override;
		size_t getColorAttachmentCount(uint32_t subpassIndex) const;

		Signal<> onDestroy;

	private:
		const VulkanDevice& m_device;
		VkRenderPass m_renderPass;
		std::vector<AttachmentDescription> m_attachments;
		std::vector<uint32_t> m_subpassColorAttachmentCount;
	};
}

#endif
