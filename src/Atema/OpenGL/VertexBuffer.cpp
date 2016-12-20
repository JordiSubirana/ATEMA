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

#include <Atema/OpenGL/VertexBuffer.hpp>
#include <Atema/Core/Error.hpp>

#include <glad/glad.h>

namespace at
{
	OpenGLVertexBuffer::OpenGLVertexBuffer() :
		OpenGLBuffer(GL_ARRAY_BUFFER)
	{
		
	}
	
	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		
	}
	
	void OpenGLVertexBuffer::reset(const Ref<VertexBuffer>& buffer)
	{
		VertexBufferStorage<char> storage;
		buffer->download(storage);
		
		VertexBuffer::reset(storage, buffer->get_vertex_format());
	}
	
	void OpenGLVertexBuffer::reset(const void *data, size_t size, const VertexFormat& format)
	{
		m_format = format;
		
		set_data(data, size*format.get_byte_size(), GL_STATIC_DRAW); //TODO: Add custom usage
	}
	
	const VertexFormat& OpenGLVertexBuffer::get_vertex_format() const
	{
		return (m_format);
	}
	
	//PRIVATE
	void OpenGLVertexBuffer::upload(const void *data, size_t size, const VertexFormat& format)
	{
		reset(data, size*format.get_byte_size(), format);
	}
	
	void OpenGLVertexBuffer::download(void *data) const
	{
		get_data(data);
	}
}