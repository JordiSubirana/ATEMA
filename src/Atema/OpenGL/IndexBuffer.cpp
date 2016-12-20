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

#include <Atema/OpenGL/IndexBuffer.hpp>
#include <Atema/Core/Error.hpp>

#include <glad/glad.h>

namespace at
{
	OpenGLIndexBuffer::OpenGLIndexBuffer() :
		OpenGLBuffer(GL_ELEMENT_ARRAY_BUFFER)
	{
		
	}
	
	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		
	}
	
	void OpenGLIndexBuffer::reset(const Ref<IndexBuffer>& buffer)
	{
		IndexBufferStorage storage;
		storage.data.resize(buffer->get_byte_size());
		
		buffer->download(storage);
		
		reset(storage);
	}
	
	void OpenGLIndexBuffer::reset(const IndexBufferStorage& storage)
	{
		set_data(reinterpret_cast<const void*>(storage.data.data()), sizeof(unsigned int)*storage.data.size(), GL_STATIC_DRAW); //TODO: Add custom usage
	}
	
	void OpenGLIndexBuffer::upload(const IndexBufferStorage& storage)
	{
		reset(storage);
	}
	
	void OpenGLIndexBuffer::download(IndexBufferStorage& storage) const
	{
		storage.data.resize(get_byte_size()/sizeof(unsigned int));
		
		get_data(reinterpret_cast<void*>(storage.data.data()));
	}
}