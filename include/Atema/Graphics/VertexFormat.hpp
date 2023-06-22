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

#ifndef ATEMA_GRAPHICS_VERTEX_HPP
#define ATEMA_GRAPHICS_VERTEX_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Graphics/Enums.hpp>
#include <Atema/Renderer/VertexInput.hpp>

#include <vector>
#include <unordered_map>

namespace at
{
	class VertexFormat;

	class ATEMA_GRAPHICS_API VertexComponent : public VertexInput
	{
		friend class VertexFormat;

	public:
		VertexComponent();
		VertexComponent(const VertexComponent& other) = default;
		VertexComponent(VertexComponentType type, uint32_t binding, uint32_t location, VertexInputFormat format);

		VertexComponent& operator=(const VertexComponent& other) = default;

		VertexComponentType type;

		// Returns the component byte offset when in a VertexFormat
		size_t getByteOffset() const;

	private:
		size_t m_byteOffset;
	};

	class ATEMA_GRAPHICS_API VertexFormat
	{
	public:
		VertexFormat();
		VertexFormat(const VertexFormat& other) = default;
		VertexFormat(VertexFormat&& other) noexcept = default;
		VertexFormat(const std::vector<VertexComponent>& components);
		~VertexFormat() = default;

		VertexFormat& operator=(const VertexFormat& other) = default;
		VertexFormat& operator=(VertexFormat&& other) noexcept = default;

		static VertexFormat create(DefaultVertexFormat defaultFormat);

		void addComponent(const VertexComponent& component);

		bool hasComponent(VertexComponentType type) const;

		const VertexComponent& getComponent(VertexComponentType type) const noexcept;
		const std::vector<VertexComponent>& getComponents() const noexcept;

		size_t getByteSize() const;

	private:
		std::vector<VertexComponent> m_components;
		std::unordered_map<VertexComponentType, size_t> m_componentsByType;
		size_t m_byteSize;
	};
}

#endif
