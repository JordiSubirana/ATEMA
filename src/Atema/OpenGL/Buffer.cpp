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

#include <Atema/OpenGL/Buffer.hpp>
#include <Atema/Core/Error.hpp>

#include <glad/glad.h>

namespace at
{
	OpenGLBuffer::OpenGLBuffer(int usage) :
		m_id(0),
		m_usage(usage),
		m_update_mode(GL_STATIC_DRAW),
		m_byte_size(0),
		m_mapped_data(nullptr)
	{
		glGenBuffers(1, &m_id);
		glBindBuffer(usage, m_id);
		
		if (glIsBuffer(m_id) == GL_FALSE)
			ATEMA_ERROR("OpenGL could not create the buffer.")
	}
	
	OpenGLBuffer::~OpenGLBuffer()
	{
		glBindBuffer(m_usage, 0);
		glDeleteBuffers(1, &m_id);
	}
	
	bool OpenGLBuffer::is_mapped() const
	{
		return (m_mapped_data);
	}
	
	void* OpenGLBuffer::map()
	{
		if (!m_mapped_data)
		{
			glBindBuffer(m_usage, m_id);
			
			m_mapped_data = glMapBuffer(m_usage, GL_READ_WRITE);
			
			if (!m_mapped_data)
				ATEMA_ERROR("OpenGL could not map the buffer.")
		}
		
		return (m_mapped_data);
	}
	
	void OpenGLBuffer::unmap() const
	{
		if (m_mapped_data)
		{
			m_mapped_data = nullptr;
			
			glBindBuffer(m_usage, m_id);
			
			if (glUnmapBuffer(m_usage) == GL_FALSE)
				ATEMA_ERROR("OpenGL could not unmap the buffer.")
		}
	}
	
	size_t OpenGLBuffer::get_byte_size() const noexcept
	{
		return (m_byte_size);
	}
	
	void OpenGLBuffer::set_data(const void *data, size_t size, int update_mode)
	{
		unmap();
		
		glBindBuffer(m_usage, m_id);
		
		if ((size != m_byte_size) || (update_mode != m_update_mode))
		{
			m_update_mode = update_mode;
			m_byte_size = size;
			
			glBufferData(m_usage, size, nullptr, m_update_mode);
			
			if (glGetError() != GL_NO_ERROR)
				ATEMA_ERROR("OpenGL could not allocate the buffer.")
		}
		
		glBufferSubData(m_usage, 0, m_byte_size, data);
	}
	
	void OpenGLBuffer::get_data(void *data) const
	{
		unmap();
		
		glBindBuffer(m_usage, m_id);
		glGetBufferSubData(m_usage, 0, m_byte_size, data);
	}
}