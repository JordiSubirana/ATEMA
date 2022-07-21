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

#include <Atema/VulkanRenderer/VulkanShader.hpp>
#include <Atema/VulkanRenderer/VulkanRenderer.hpp>
#include <Atema/Shader/Atsl/AtslParser.hpp>
#include <Atema/Shader/Atsl/AtslToAstConverter.hpp>
#include <Atema/Shader/Spirv/SpirvShaderWriter.hpp>

using namespace at;

VulkanShader::VulkanShader(const VulkanDevice& device, const Shader::Settings& settings) :
	Shader(),
	m_device(device),
	m_shaderModule(VK_NULL_HANDLE)
{
	if (!settings.shaderData || settings.shaderDataSize == 0)
		ATEMA_ERROR("Invalid shader source");

	switch (settings.shaderLanguage)
	{
		case ShaderLanguage::Ast:
		{
			auto ast = static_cast<Statement*>(settings.shaderData);

			SpirvShaderWriter spvWriter;

			ast->accept(spvWriter);

			std::vector<uint32_t> code;

			spvWriter.compile(code);

			create(code.data(), code.size());

			break;
		}
		case ShaderLanguage::Atsl:
		{
			AtslParser atslParser;

			const auto atslTokens = atslParser.createTokens(std::string(static_cast<const char*>(settings.shaderData), settings.shaderDataSize));

			AtslToAstConverter converter;

			auto ast = converter.createAst(atslTokens);

			SpirvShaderWriter spvWriter;

			ast->accept(spvWriter);

			std::vector<uint32_t> code;

			spvWriter.compile(code);

			create(code.data(), code.size());

			break;
		}
		case ShaderLanguage::SpirV:
		{
			create(static_cast<uint32_t*>(settings.shaderData), settings.shaderDataSize);

			break;
		}
		default:
			ATEMA_ERROR("Unsupported shader language");
	}
}

VulkanShader::~VulkanShader()
{
	ATEMA_VK_DESTROY(m_device, vkDestroyShaderModule, m_shaderModule);
}

VkShaderModule VulkanShader::getHandle() const noexcept
{
	return m_shaderModule;
}

void VulkanShader::create(const uint32_t* code, size_t codeSize)
{
	if (!code || codeSize == 0)
		ATEMA_ERROR("Invalid shader source");

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = codeSize * sizeof(uint32_t);
	createInfo.pCode = code;

	ATEMA_VK_CHECK(m_device.vkCreateShaderModule(m_device, &createInfo, nullptr, &m_shaderModule));
}
