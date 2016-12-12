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

#ifndef ATEMA_RENDERER_VERTEX_IMPLEMENTATION
#define ATEMA_RENDERER_VERTEX_IMPLEMENTATION

#include <Atema/Renderer/Vertex.hpp>

namespace at
{
	//--------------------
	//Vertex Component
	//--------------------
	template <typename T>
	VertexComponent VertexComponent::get(VertexComponent::Usage usage)
	{
		VertexComponent component;
		
		component.usage = usage;
		component.type = get_type<T>();
		
		return (component);
	}
	
	template <typename T>
	VertexComponent::Type VertexComponent::get_type()
	{
		return (VertexComponent::Type::Unknown);
	}
	
	//--------------------
	//Vertex Format
	//--------------------
	template <typename T>
	const VertexFormat& VertexFormat::get()
	{
		return (VertexFormat{});
	}
}

#endif