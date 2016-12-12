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

#include <Atema/Renderer/VertexBuffer.hpp>
#include <Atema/Renderer/Renderer.hpp>
#include <Atema/Core/Error.hpp>

namespace at
{
	Ref<VertexBuffer> VertexBuffer::create(Renderer *renderer)
	{
		if (!renderer)
		{
			renderer = Renderer::get_current();
			
			if (!renderer)
				ATEMA_ERROR("No Renderer is currently active.")
		}
		
		Ref<VertexBuffer> ref = renderer->create_vertex_buffer();
		
		return (ref);
	}
	
	Ref<VertexBuffer> VertexBuffer::create(const Ref<VertexBuffer>& buffer, Renderer *renderer)
	{
		Ref<VertexBuffer> ref = VertexBuffer::create(renderer);
		ref->reset(buffer);
		
		return (ref);
	}
	
	Ref<VertexBuffer> VertexBuffer::create(void *data, size_t size, const VertexFormat& format, Renderer *renderer)
	{
		Ref<VertexBuffer> ref = VertexBuffer::create(renderer);
		ref->reset(data, size, format);
		
		return (ref);
	}
}