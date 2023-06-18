/*
	Copyright 2023 Jordi SUBIRANA

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

#include <Atema/VulkanRenderer/UI/VulkanUiContext.hpp>
#include <Atema/VulkanRenderer/VulkanCommandBuffer.hpp>
#include <Atema/Renderer/DescriptorSetLayout.hpp>
#include <Atema/VulkanRenderer/VulkanDescriptorSetLayout.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>
#include <Atema/VulkanRenderer/VulkanRenderPass.hpp>
#include <Atema/VulkanRenderer/VulkanRenderWindow.hpp>
#include <Atema/VulkanRenderer/VulkanDescriptorPool.hpp>
#include <Atema/VulkanRenderer/VulkanSwapChain.hpp>

#include <imgui/imgui_impl_vulkan.h>

// Dear ImGui implementation
#include <imgui/imgui_impl_vulkan.cpp>

using namespace at;

namespace
{
	void checkVkResult(VkResult error)
	{
		if (error != 0)
			ATEMA_ERROR("Vulkan Error : VkResult = " + std::to_string(error));
	}
}

VulkanUiContext::VulkanUiContext(const UiContext::Settings& settings) :
	UiContext(settings)
{
	ATEMA_ASSERT(settings.renderWindow, "Invalid RenderWindow");

	auto& renderer = static_cast<VulkanRenderer&>(Renderer::instance());
	auto& window = static_cast<VulkanRenderWindow&>(*settings.renderWindow);

	// Create descriptor pool
	{
		DescriptorSetLayout::Settings settings;
		settings.pageSize = 1000;

		uint32_t bindingIndex = 0;
		settings.bindings =
		{
			{ DescriptorType::Sampler, bindingIndex++, 1, ShaderStage::Fragment },
			{ DescriptorType::CombinedImageSampler, bindingIndex++, 1, ShaderStage::Fragment },
			{ DescriptorType::SampledImage, bindingIndex++, 1, ShaderStage::Fragment },
			{ DescriptorType::StorageImage, bindingIndex++, 1, ShaderStage::Fragment },
			{ DescriptorType::UniformTexelBuffer, bindingIndex++, 1, ShaderStage::Fragment },
			{ DescriptorType::StorageTexelBuffer, bindingIndex++, 1, ShaderStage::Fragment },
			{ DescriptorType::UniformBuffer, bindingIndex++, 1, ShaderStage::Fragment },
			{ DescriptorType::StorageBuffer, bindingIndex++, 1, ShaderStage::Fragment },
			{ DescriptorType::UniformBufferDynamic, bindingIndex++, 1, ShaderStage::Fragment },
			{ DescriptorType::StorageBufferDynamic, bindingIndex++, 1, ShaderStage::Fragment },
			{ DescriptorType::InputAttachment, bindingIndex++, 1, ShaderStage::Fragment }
		};

		m_descriptorSetLayout = DescriptorSetLayout::create(settings);
	}

	const auto& vkRenderPass = static_cast<VulkanRenderPass&>(*window.getRenderPass());
	const auto& vkDescriptorSetLayout = static_cast<VulkanDescriptorSetLayout&>(*m_descriptorSetLayout);

	ImGui_ImplVulkan_LoadFunctions([](const char* name, void*)
		{
			return static_cast<VulkanRenderer&>(Renderer::instance()).getDevice().getProcAddr(name);
		});

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = renderer.getInstance().getHandle();
	init_info.PhysicalDevice = renderer.getPhysicalDevice().getHandle();
	init_info.Device = renderer.getDevice().getHandle();
	init_info.QueueFamily = renderer.getDevice().getQueueFamilyIndex(QueueType::Graphics);
	init_info.Queue = renderer.getDevice().getQueue(QueueType::Graphics);
	init_info.PipelineCache = nullptr;
	init_info.DescriptorPool = vkDescriptorSetLayout.getDescriptorPool().getHandle();
	init_info.Subpass = 0;
	init_info.MinImageCount = static_cast<uint32_t>(Renderer::FramesInFlight);
	init_info.ImageCount = static_cast<uint32_t>(window.getSwapChain().getImages().size());
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr;
	init_info.CheckVkResultFn = checkVkResult;
	ImGui_ImplVulkan_Init(&init_info, vkRenderPass.getHandle());

	// Upload Fonts
	{
		auto commandPool = Renderer::instance().getCommandPool(QueueType::Graphics);

		auto commandBuffer = commandPool->createBuffer({ true });

		commandBuffer->begin();

		ImGui_ImplVulkan_CreateFontsTexture(static_cast<VulkanCommandBuffer&>(*commandBuffer).getHandle());

		commandBuffer->end();

		Renderer::instance().submitAndWait({ commandBuffer });

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
}

VulkanUiContext::~VulkanUiContext()
{
}

void VulkanUiContext::draw(ImDrawData* drawData, CommandBuffer& commandBuffer)
{
	ImGui_ImplVulkan_RenderDrawData(drawData, static_cast<VulkanCommandBuffer&>(commandBuffer).getHandle());
}

void VulkanUiContext::shutdownPlatform()
{
	ImGui_ImplVulkan_Shutdown();
}

void VulkanUiContext::newPlatformFrame()
{
	ImGui_ImplVulkan_NewFrame();
}
