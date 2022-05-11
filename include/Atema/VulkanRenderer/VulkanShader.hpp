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

#ifndef ATEMA_VULKANRENDERER_VULKANSHADER_HPP
#define ATEMA_VULKANRENDERER_VULKANSHADER_HPP

#include <Atema/VulkanRenderer/Config.hpp>
#include <Atema/Renderer/Shader.hpp>
#include <Atema/VulkanRenderer/Vulkan.hpp>

namespace at
{
	class ATEMA_VULKANRENDERER_API VulkanShader final : public Shader
	{
	public:
		VulkanShader() = delete;
		VulkanShader(const VulkanDevice& device, const Shader::Settings& settings);
		virtual ~VulkanShader();

		VkShaderModule getHandle() const noexcept;

	private:
		const VulkanDevice& m_device;
		VkShaderModule m_shaderModule;
	};
}

#endif
