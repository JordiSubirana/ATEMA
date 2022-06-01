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

#ifndef ATEMA_RENDERER_DESCRIPTORSETLAYOUT_HPP
#define ATEMA_RENDERER_DESCRIPTORSETLAYOUT_HPP

#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Renderer/Config.hpp>
#include <Atema/Renderer/DescriptorSet.hpp>
#include <Atema/Renderer/Enums.hpp>

#include <vector>

namespace at
{
	struct ATEMA_RENDERER_API DescriptorSetBinding
	{
		DescriptorSetBinding();
		DescriptorSetBinding(DescriptorType type, uint32_t binding, uint32_t count, Flags<ShaderStage> shaderStages);

		DescriptorType type;
		uint32_t binding;
		uint32_t count;
		Flags<ShaderStage> shaderStages;
	};

	class ATEMA_RENDERER_API DescriptorSetLayout : public NonCopyable
	{
	public:
		struct Settings
		{
			std::vector<DescriptorSetBinding> bindings;
			uint32_t pageSize = 128;
		};

		virtual ~DescriptorSetLayout();

		static Ptr<DescriptorSetLayout> create(const Settings& settings);

		virtual const std::vector<DescriptorSetBinding>& getBindings() const noexcept = 0;

		virtual Ptr<DescriptorSet> createSet() = 0;

	protected:
		DescriptorSetLayout();
	};
}

#endif
