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

#include <Atema/VulkanRenderer/VulkanShader.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>

#include <fstream>

using namespace at;

namespace
{
	std::vector<char> readFile(const std::filesystem::path& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			ATEMA_ERROR("Failed to open file : " + filename.string());
		}

		size_t fileSize = static_cast<size_t>(file.tellg());

		std::vector<char> buffer(fileSize);

		file.seekg(0);

		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}
}

VulkanShader::VulkanShader(const Shader::Settings& settings) :
	Shader(),
	m_shaderModule(VK_NULL_HANDLE)
{
	auto& renderer = VulkanRenderer::getInstance();
	auto device = renderer.getLogicalDeviceHandle();

	auto shaderCode = readFile(settings.path);

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shaderCode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

	ATEMA_VK_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &m_shaderModule));
}

VulkanShader::~VulkanShader()
{
	auto& renderer = VulkanRenderer::getInstance();
	auto device = renderer.getLogicalDeviceHandle();

	ATEMA_VK_DESTROY(device, vkDestroyShaderModule, m_shaderModule);
}

VkShaderModule VulkanShader::getHandle() const noexcept
{
	return m_shaderModule;
}
