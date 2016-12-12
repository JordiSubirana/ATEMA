// ----------------------------------------------------------------------
// Copyright (C) 2016 Jordi SUBIRANA
//
// This file is part of ATEMA.
//
// ATEMA is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ATEMA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ATEMA.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------

#include <Atema/Renderer/Vertex.hpp>
#include <Atema/Core/Error.hpp>

namespace at
{
	namespace
	{
		unsigned int byte_sizes[] =
		{
			0,
			sizeof(int),			2*sizeof(int),			3*sizeof(int),			4*sizeof(int),
			sizeof(unsigned int),	2*sizeof(unsigned int),	3*sizeof(unsigned int),	4*sizeof(unsigned int),
			sizeof(float),			2*sizeof(float),		3*sizeof(float),		4*sizeof(float),
			sizeof(double),			2*sizeof(double),		3*sizeof(double),		4*sizeof(double)
		};
	}
	
	//--------------------
	//Vertex Component
	//--------------------
	VertexComponent::VertexComponent() :
		type(VertexComponent::Type::Unknown),
		usage(VertexComponent::Usage::Custom),
		byte_offset(0)
	{
		
	}
	
	unsigned int VertexComponent::type_byte_size(VertexComponent::Type type)
	{
		return (byte_sizes[static_cast<unsigned int>(type)]);
	}
	
	//--------------------
	//Vertex Format
	//--------------------
	size_t VertexFormat::get_size() const
	{
		return (m_components.size());
	}
	
	size_t VertexFormat::get_byte_size() const
	{
		return (m_byte_size);
	}
	
	VertexComponent VertexFormat::operator[](size_t index)
	{
		if (index >= m_components.size())
			ATEMA_ERROR("Index is greater than array size.")
		
		return (m_components[index]);
	}
	
	//Private constructor
	VertexFormat::VertexFormat(std::initializer_list<VertexComponent> components)
	{
		m_components = components;
		
		m_byte_size = 0;
		
		for (auto& component : m_components)
		{
			component.byte_offset = m_byte_size;
			m_byte_size += VertexComponent::type_byte_size(component.type);
		}
	}
}