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

#include <Atema/VulkanRenderer/VulkanFramebuffer.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>
#include <Atema/VulkanRenderer/VulkanImage.hpp>
#include <Atema/VulkanRenderer/VulkanImageView.hpp>
#include <Atema/VulkanRenderer/VulkanRenderPass.hpp>

using namespace at;

VulkanFramebuffer::VulkanFramebuffer(const VulkanDevice& device, const Framebuffer::Settings& settings) :
	Framebuffer(),
	m_device(device),
	m_framebuffer(VK_NULL_HANDLE)
{
	auto renderPass = std::static_pointer_cast<VulkanRenderPass>(settings.renderPass);
	std::vector<VkImageView> attachments;

	for (auto& imageView : settings.imageViews)
	{
		auto vkImageView = std::static_pointer_cast<VulkanImageView>(imageView);

		attachments.push_back(vkImageView->getHandle());
	}
	
	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass->getHandle();
	framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.width = settings.width;
	framebufferInfo.height = settings.height;
	framebufferInfo.layers = 1;

	ATEMA_VK_CHECK(m_device.vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_framebuffer));

	m_size.x = settings.width;
	m_size.y = settings.height;
}

VulkanFramebuffer::~VulkanFramebuffer()
{
	ATEMA_VK_DESTROY(m_device, vkDestroyFramebuffer, m_framebuffer);
}

VkFramebuffer VulkanFramebuffer::getHandle() const noexcept
{
	return m_framebuffer;
}

Vector2u VulkanFramebuffer::getSize() const noexcept
{
	return m_size;
}
