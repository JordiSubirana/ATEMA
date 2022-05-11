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

#include <Atema/VulkanRenderer/VulkanDescriptorPool.hpp>
#include <Atema/VulkanRenderer/VulkanDescriptorSet.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>

using namespace at;

// Internal pool
VulkanDescriptorPool::Pool::Pool(const VulkanDevice& device, VkDescriptorSetLayout layout, const SparseSet<VkDescriptorType>& bindingTypes, const VkDescriptorPoolCreateInfo& settings) :
	m_device(device),
	m_pool(VK_NULL_HANDLE),
	m_layout(layout),
	m_bindingTypes(bindingTypes),
	m_size(0),
	m_maxSize(settings.maxSets)
{
	ATEMA_VK_CHECK(m_device.vkCreateDescriptorPool(m_device, &settings, nullptr, &m_pool));
}

VulkanDescriptorPool::Pool::~Pool()
{
	ATEMA_VK_DESTROY(m_device, vkDestroyDescriptorPool, m_pool);
}

bool VulkanDescriptorPool::Pool::isFull() const noexcept
{
	return m_size == m_maxSize;
}

Ptr<DescriptorSet> VulkanDescriptorPool::Pool::createSet()
{
	VkDescriptorSet handle = VK_NULL_HANDLE;

	// Recycle an unused set if any
	if (!m_unusedSets.empty())
	{
		handle = m_unusedSets.front();
		m_unusedSets.pop();
	}
	// Allocate a new set
	else
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_pool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_layout;

		ATEMA_VK_CHECK(m_device.vkAllocateDescriptorSets(m_device, &allocInfo, &handle));
	}

	m_size++;

	auto descriptorSet = std::make_shared<VulkanDescriptorSet>(m_device, handle, m_bindingTypes, [this, handle]()
	{
		m_unusedSets.push(handle);
		m_size--;
	});
	
	return std::static_pointer_cast<DescriptorSet>(descriptorSet);
}

// Descriptor pool
VulkanDescriptorPool::VulkanDescriptorPool(const VulkanDevice& device, const DescriptorPool::Settings& settings) :
	DescriptorPool(),
	m_device(device),
	m_poolSettings({}),
	m_layout(VK_NULL_HANDLE)
{
	m_layout = std::static_pointer_cast<VulkanDescriptorSetLayout>(settings.layout)->getHandle();
	
	// Save pool creation settings
	const auto& bindings = settings.layout->getBindings();
	
	m_poolSizes.reserve(bindings.size());
	m_bindingTypes.reserve(bindings.size());
	
	for (auto& binding : bindings)
	{
		const auto bindingType = Vulkan::getDescriptorType(binding.type);

		VkDescriptorPoolSize poolSize;
		poolSize.type = bindingType;
		poolSize.descriptorCount = settings.pageSize * binding.count;

		m_poolSizes.push_back(poolSize);

		m_bindingTypes.emplace(binding.binding) = bindingType;
	}

	m_poolSettings.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	m_poolSettings.poolSizeCount = static_cast<uint32_t>(m_poolSizes.size());
	m_poolSettings.pPoolSizes = m_poolSizes.data();
	m_poolSettings.maxSets = settings.pageSize;
	m_poolSettings.flags = 0;
	
	// We need at least one pool
	addPool();
}

VulkanDescriptorPool::~VulkanDescriptorPool()
{
	m_pools.clear();
}

Ptr<DescriptorSet> VulkanDescriptorPool::createSet()
{
	// Check if a pool can create a set
	for (auto& pool : m_pools)
	{
		if (!pool->isFull())
		{
			return pool->createSet();
		}
	}

	// If not, add a new pool then create the set
	addPool();
	
	return m_pools.back()->createSet();
}

void VulkanDescriptorPool::addPool()
{
	m_pools.push_back(std::make_shared<Pool>(m_device, m_layout, m_bindingTypes, m_poolSettings));
}
