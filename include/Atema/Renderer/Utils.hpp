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

#ifndef ATEMA_RENDERER_UTILS_HPP
#define ATEMA_RENDERER_UTILS_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Renderer/Enums.hpp>
#include <Atema/Renderer/VertexInput.hpp>
#include <Atema/Shader/Ast/Type.hpp>

namespace at
{
	ATEMA_RENDERER_API VertexInputFormat getVertexFormat(const AstType& astVariableType);

	ATEMA_RENDERER_API DescriptorType getDefaultDescriptorType(const AstType& astVariableType);
	ATEMA_RENDERER_API DescriptorType getDefaultDescriptorType(const AstArrayType::ComponentType& astVariableType);
	ATEMA_RENDERER_API uint32_t getDescriptorBindingCount(const AstType& astVariableType);

	ATEMA_RENDERER_API size_t getByteSize(IndexType indexType);

	ATEMA_RENDERER_API size_t getByteSize(ImageFormat format);
	ATEMA_RENDERER_API size_t getComponentCount(ImageFormat format);
	// Only valid for color formats
	ATEMA_RENDERER_API ImageComponentType getComponentType(ImageFormat format);
	// Only valid for color formats
	// componentCount must be in the range [1,4]
	// BGR components are not taken into account (RGB only)
	ATEMA_RENDERER_API ImageFormat getImageColorFormat(ImageComponentType componentType, size_t componentCount);
}

#endif
