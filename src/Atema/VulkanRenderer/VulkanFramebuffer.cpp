/*
	Copyright 2021 Jordi SUBIRANA

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

#include <Atema/VulkanRenderer/VulkanFramebuffer.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>
#include <Atema/VulkanRenderer/VulkanImage.hpp>
#include <Atema/VulkanRenderer/VulkanRenderPass.hpp>

using namespace at;

VulkanFramebuffer::VulkanFramebuffer(const Framebuffer::Settings& settings) :
	Framebuffer(),
	m_framebuffer(VK_NULL_HANDLE)
{
	auto& renderer = VulkanRenderer::getInstance();
	auto device = renderer.getLogicalDeviceHandle();

	auto renderPass = std::static_pointer_cast<VulkanRenderPass>(settings.renderPass);
	std::vector<VkImageView> attachments;

	for (auto& image : settings.images)
	{
		auto vkImage = std::static_pointer_cast<VulkanImage>(image);

		attachments.push_back(vkImage->getViewHandle());
	}
	
	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass->getHandle();
	framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.width = settings.width;
	framebufferInfo.height = settings.height;
	framebufferInfo.layers = 1;

	ATEMA_VK_CHECK(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_framebuffer));
}

VulkanFramebuffer::~VulkanFramebuffer()
{
	auto& renderer = VulkanRenderer::getInstance();
	auto device = renderer.getLogicalDeviceHandle();

	ATEMA_VK_DESTROY(device, vkDestroyFramebuffer, m_framebuffer);
}
