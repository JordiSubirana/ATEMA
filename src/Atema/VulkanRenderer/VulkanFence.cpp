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

#include <Atema/VulkanRenderer/VulkanFence.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>

using namespace at;

VulkanFence::VulkanFence(const VulkanDevice& device, const Fence::Settings& settings) :
	Fence(),
	m_device(device),
	m_fence(VK_NULL_HANDLE)
{
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = settings.signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0; // Specify initial state

	ATEMA_VK_CHECK(m_device.vkCreateFence(m_device, &fenceInfo, nullptr, &m_fence));
}

VulkanFence::~VulkanFence()
{
	ATEMA_VK_DESTROY(m_device, vkDestroyFence, m_fence);
}

VkFence VulkanFence::getHandle() const noexcept
{
	return m_fence;
}

void VulkanFence::wait()
{
	m_device.vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, UINT64_MAX);
}

void VulkanFence::reset()
{
	m_device.vkResetFences(m_device, 1, &m_fence);
}
