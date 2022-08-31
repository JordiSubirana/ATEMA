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

#include <Atema/Core/Error.hpp>
#include <Atema/Graphics/VertexFormat.hpp>

using namespace at;

//-----
// VertexComponent
VertexComponent::VertexComponent() :
	VertexComponent(VertexComponentType::UserData, 0, 0, VertexInputFormat::RGB32_SFLOAT)
{
}

VertexComponent::VertexComponent(VertexComponentType type, uint32_t binding, uint32_t location, VertexInputFormat format) :
	VertexInput(binding, location, format),
	type(type),
	m_byteOffset(0)
{
}

size_t VertexComponent::getByteOffset() const
{
	return m_byteOffset;
}

//-----
// VertexFormat
VertexFormat::VertexFormat() :
	m_byteSize(0)
{
}

VertexFormat::VertexFormat(const std::vector<VertexComponent>& components) :
	m_byteSize(0)
{
	m_components.reserve(components.size());

	for (const auto& component : components)
		addComponent(component);
}

Ptr<VertexFormat> VertexFormat::create(DefaultVertexFormat defaultFormat)
{
	switch (defaultFormat)
	{
		case DefaultVertexFormat::XY:
		{
			return std::make_shared<VertexFormat>(std::initializer_list<VertexComponent>{
				VertexComponent(VertexComponentType::Position, 0, 0, VertexInputFormat::RG32_SFLOAT)
			});
		}
		case DefaultVertexFormat::XY_RGB:
		{
			return std::make_shared<VertexFormat>(std::initializer_list<VertexComponent>{
				VertexComponent(VertexComponentType::Position, 0, 0, VertexInputFormat::RG32_SFLOAT),
					VertexComponent(VertexComponentType::Color, 0, 1, VertexInputFormat::RGB32_SFLOAT)
			});
		}
		case DefaultVertexFormat::XY_RGBA:
		{
			return std::make_shared<VertexFormat>(std::initializer_list<VertexComponent>{
				VertexComponent(VertexComponentType::Position, 0, 0, VertexInputFormat::RG32_SFLOAT),
				VertexComponent(VertexComponentType::Color, 0, 1, VertexInputFormat::RGBA32_SFLOAT)
				});
		}
		case DefaultVertexFormat::XY_UV:
		{
			return std::make_shared<VertexFormat>(std::initializer_list<VertexComponent>{
				VertexComponent(VertexComponentType::Position, 0, 0, VertexInputFormat::RG32_SFLOAT),
				VertexComponent(VertexComponentType::TexCoords, 0, 1, VertexInputFormat::RG32_SFLOAT)
				});
		}
		case DefaultVertexFormat::XYZ:
		{
			return std::make_shared<VertexFormat>(std::initializer_list<VertexComponent>{
				VertexComponent(VertexComponentType::Position, 0, 0, VertexInputFormat::RGB32_SFLOAT)
				});
		}
		case DefaultVertexFormat::XYZ_RGB:
		{
			return std::make_shared<VertexFormat>(std::initializer_list<VertexComponent>{
				VertexComponent(VertexComponentType::Position, 0, 0, VertexInputFormat::RGB32_SFLOAT),
					VertexComponent(VertexComponentType::Color, 0, 1, VertexInputFormat::RGB32_SFLOAT)
			});
		}
		case DefaultVertexFormat::XYZ_RGB_Normal:
		{
			return std::make_shared<VertexFormat>(std::initializer_list<VertexComponent>{
				VertexComponent(VertexComponentType::Position, 0, 0, VertexInputFormat::RGB32_SFLOAT),
					VertexComponent(VertexComponentType::Color, 0, 1, VertexInputFormat::RGB32_SFLOAT),
					VertexComponent(VertexComponentType::Normal, 0, 2, VertexInputFormat::RGB32_SFLOAT)
			});
		}
		case DefaultVertexFormat::XYZ_RGB_Normal_Tangent_Bitangent:
		{
			return std::make_shared<VertexFormat>(std::initializer_list<VertexComponent>{
				VertexComponent(VertexComponentType::Position, 0, 0, VertexInputFormat::RGB32_SFLOAT),
					VertexComponent(VertexComponentType::Color, 0, 1, VertexInputFormat::RGB32_SFLOAT),
					VertexComponent(VertexComponentType::Normal, 0, 2, VertexInputFormat::RGB32_SFLOAT),
					VertexComponent(VertexComponentType::Tangent, 0, 3, VertexInputFormat::RGB32_SFLOAT),
					VertexComponent(VertexComponentType::Bitangent, 0, 4, VertexInputFormat::RGB32_SFLOAT)
			});
		}
		case DefaultVertexFormat::XYZ_RGBA:
		{
			return std::make_shared<VertexFormat>(std::initializer_list<VertexComponent>{
				VertexComponent(VertexComponentType::Position, 0, 0, VertexInputFormat::RGB32_SFLOAT),
				VertexComponent(VertexComponentType::Color, 0, 1, VertexInputFormat::RGBA32_SFLOAT)
				});
		}
		case DefaultVertexFormat::XYZ_RGBA_Normal:
		{
			return std::make_shared<VertexFormat>(std::initializer_list<VertexComponent>{
				VertexComponent(VertexComponentType::Position, 0, 0, VertexInputFormat::RGB32_SFLOAT),
				VertexComponent(VertexComponentType::Color, 0, 1, VertexInputFormat::RGBA32_SFLOAT),
				VertexComponent(VertexComponentType::Normal, 0, 2, VertexInputFormat::RGB32_SFLOAT)
				});
		}
		case DefaultVertexFormat::XYZ_RGBA_Normal_Tangent_Bitangent:
		{
			return std::make_shared<VertexFormat>(std::initializer_list<VertexComponent>{
				VertexComponent(VertexComponentType::Position, 0, 0, VertexInputFormat::RGB32_SFLOAT),
				VertexComponent(VertexComponentType::Color, 0, 1, VertexInputFormat::RGBA32_SFLOAT),
				VertexComponent(VertexComponentType::Normal, 0, 2, VertexInputFormat::RGB32_SFLOAT),
				VertexComponent(VertexComponentType::Tangent, 0, 3, VertexInputFormat::RGB32_SFLOAT),
				VertexComponent(VertexComponentType::Bitangent, 0, 4, VertexInputFormat::RGB32_SFLOAT)
				});
		}
		case DefaultVertexFormat::XYZ_UV:
		{
			return std::make_shared<VertexFormat>(std::initializer_list<VertexComponent>{
				VertexComponent(VertexComponentType::Position, 0, 0, VertexInputFormat::RGB32_SFLOAT),
				VertexComponent(VertexComponentType::TexCoords, 0, 1, VertexInputFormat::RG32_SFLOAT)
				});
		}
		case DefaultVertexFormat::XYZ_UV_Normal:
		{
			return std::make_shared<VertexFormat>(std::initializer_list<VertexComponent>{
				VertexComponent(VertexComponentType::Position, 0, 0, VertexInputFormat::RGB32_SFLOAT),
				VertexComponent(VertexComponentType::TexCoords, 0, 1, VertexInputFormat::RG32_SFLOAT),
				VertexComponent(VertexComponentType::Normal, 0, 2, VertexInputFormat::RGB32_SFLOAT)
				});
		}
		case DefaultVertexFormat::XYZ_UV_Normal_Tangent_Bitangent:
		{
			return std::make_shared<VertexFormat>(std::initializer_list<VertexComponent>{
				VertexComponent(VertexComponentType::Position, 0, 0, VertexInputFormat::RGB32_SFLOAT),
				VertexComponent(VertexComponentType::TexCoords, 0, 1, VertexInputFormat::RG32_SFLOAT),
				VertexComponent(VertexComponentType::Normal, 0, 2, VertexInputFormat::RGB32_SFLOAT),
				VertexComponent(VertexComponentType::Tangent, 0, 3, VertexInputFormat::RGB32_SFLOAT),
				VertexComponent(VertexComponentType::Bitangent, 0, 4, VertexInputFormat::RGB32_SFLOAT)
				});
		}
	}

	ATEMA_ERROR("Invalid format");

	return {};
}

void VertexFormat::addComponent(const VertexComponent& component)
{
	m_componentsByType[component.type] = m_components.size();

	auto& newComponent = m_components.emplace_back(component);

	newComponent.m_byteOffset = m_byteSize;

	m_byteSize += newComponent.getByteSize();
}

bool VertexFormat::hasComponent(VertexComponentType type) const
{
	return m_componentsByType.count(type) > 0;
}

const VertexComponent& VertexFormat::getComponent(VertexComponentType type) const noexcept
{
	const auto it = m_componentsByType.find(type);

	ATEMA_ASSERT(it != m_componentsByType.end(), "No component found with the required type");

	return m_components[it->second];
}

const std::vector<VertexComponent>& VertexFormat::getComponents() const noexcept
{
	return m_components;
}

size_t VertexFormat::getByteSize() const
{
	return m_byteSize;
}
