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

#ifndef ATEMA_RENDERER_VERTEX_BUFFER_IMPLEMENTATION
#define ATEMA_RENDERER_VERTEX_BUFFER_IMPLEMENTATION

#include <Atema/Renderer/VertexBuffer.hpp>
#include <Atema/Renderer/Renderer.hpp>
#include <Atema/Core/Error.hpp>

namespace at
{
	template <typename T>
	Ref<VertexBuffer> VertexBuffer::create(const VertexBufferStorage<T>& storage, const VertexFormat& format, Renderer *renderer)
	{
		Ref<VertexBuffer> ref = VertexBuffer::create(renderer);
		ref->reset(storage, format);
		
		return (ref);
	}
	
	template <typename T>
	void VertexBuffer::reset(const VertexBufferStorage<T>& storage, const VertexFormat& format)
	{
		if (format.get_byte_size() != sizeof(T))
			ATEMA_ERROR("Invalid vertex format.")
		
		reset(static_cast<void*>(storage.data.data()), storage.data.size(), format);
	}
	
	template <typename T>
	void VertexBuffer::upload(const VertexBufferStorage<T>& storage)
	{
		VertexFormat format = get_vertex_format();
		
		if (format.get_byte_size() != sizeof(T))
			ATEMA_ERROR("Invalid data format.")
		
		upload(static_cast<void*>(storage.data.data()), storage.data.size(), format);
	}
	
	template <typename T>
	void VertexBuffer::download(VertexBufferStorage<T>& storage) const
	{
		storage.data.resize(get_size()*get_vertex_format().get_byte_size() / sizeof(T));
		
		download(static_cast<void*>(storage.data.data()));
	}
}

#endif