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

#ifndef ATEMA_GRAPHICS_BUFFER_ARRAY_IMPLEMENTATION
#define ATEMA_GRAPHICS_BUFFER_ARRAY_IMPLEMENTATION

#include <atema/core/error.hpp>
#include <atema/graphics/buffer.hpp>



namespace at
{
	//PUBLIC
	template <typename T>
	Buffer<T>::Buffer() :
		m_internal_type(GL_ARRAY_BUFFER),
		m_vbo(0),
		m_elements(),
		m_update_mode(update_mode::static_draw),
		m_buffer_ok(false),
		m_filled(false)
	{
		ensure_buffer();
	}
	
	template <typename T>
	Buffer<T>::Buffer(const T *elements, size_t elements_size, update_mode update_mode) :
		Buffer()
	{
		create(elements, elements_size, update_mode);
	}
	
	template <typename T>
	Buffer<T>::Buffer(const Buffer<T>& array) :
		Buffer()
	{
		create(array);
	}
	
	template <typename T>
	Buffer<T>::~Buffer() noexcept
	{
		glBindBuffer(m_internal_type, 0);
		glDeleteBuffers(1, &m_vbo);
		
		m_elements.resize(0);
		
		m_vbo = 0;
	}
	
	template <typename T>
	void Buffer<T>::create(const T *elements, size_t elements_size, update_mode update_mode)
	{
		try
		{
			m_filled = false;
			m_elements.clear();
			
			if (!elements || !elements_size)
				return;
			
			ensure_buffer();
			
			m_elements.resize(elements_size);
			m_update_mode = update_mode;
			
			glBindBuffer(m_internal_type, m_vbo);
			
			for (size_t i = 0; i < elements_size; i++)
			{
				m_elements[i] = elements[i];
			}
			
			glBufferData(m_internal_type, elements_size*sizeof(T), nullptr, static_cast<GLenum>(update_mode));
			
			if (glGetError() != GL_NO_ERROR)
				ATEMA_ERROR("OpenGL could not allocate VBO.")
			
			glBufferSubData(m_internal_type, 0, elements_size*sizeof(T), elements);
			
			if (glGetError() != GL_NO_ERROR)
				ATEMA_ERROR("OpenGL could not fill VBO.")
			
			glBindBuffer(m_internal_type, 0);
			
			m_filled = true;
		}
		catch (const Error& e)
		{
			m_elements.resize(0);
			
			throw;
		}
	}


	template <typename T>
	T* Buffer<T>::createVRAM_map(size_t elements_size, update_mode update_mode)
	{
		try
		{
			m_filled = false;
			m_elements.clear();

			if (!elements_size)
				return nullptr ;

			ensure_buffer();

			m_update_mode = update_mode;

			glBindBuffer(m_internal_type, m_vbo);

			glBufferData(m_internal_type, elements_size*sizeof(T), nullptr, static_cast<GLenum>(update_mode));

			if (glGetError() != GL_NO_ERROR)
				ATEMA_ERROR("OpenGL could not allocate VBO.")

			glBufferSubData(m_internal_type, 0, elements_size*sizeof(T), nullptr);

			if (glGetError() != GL_NO_ERROR)
				ATEMA_ERROR("OpenGL could not fill VBO.")


			m_filled = true;

			return static_cast<T*>(glMapBuffer(m_internal_type, GL_READ_WRITE));

		}
		catch (const Error& e)
		{
			m_elements.resize(0);

			throw;
		}
	}

	template <typename T>
	void Buffer<T>::unmap() const
	{
		glUnmapBuffer(m_internal_type);
		glBindBuffer(m_internal_type, 0);
	}

	template <typename T>
	void Buffer<T>::create(const Buffer<T>& array)
	{
		create(array.get(), array.get_size(), array.get_update_mode());
	}
	
	template <typename T>
	T* Buffer<T>::get() noexcept
	{
		if (get_size() == 0)
			return (nullptr);
		
		return (m_elements.data());
	}
	
	template <typename T>
	const T* Buffer<T>::get() const noexcept
	{
		if (get_size() == 0)
			return (nullptr);
		
		return (m_elements.data());
	}
	
	template <typename T>
	T& Buffer<T>::operator[](size_t index)
	{
		if (index >= get_size())
			ATEMA_ERROR("Index is greater than array size.")
		
		return (m_elements[index]);
	}
	
	template <typename T>
	const T& Buffer<T>::operator[](size_t index) const
	{
		if (index >= get_size())
			ATEMA_ERROR("Index is greater than array size.")
		
		return (m_elements[index]);
	}
	
	template <typename T>
	size_t Buffer<T>::get_size() const
	{
		return (m_elements.size());
	}
	
	template <typename T>
	typename Buffer<T>::update_mode Buffer<T>::get_update_mode() const
	{
		return (m_update_mode);
	}
	
	template <typename T>
	GLuint Buffer<T>::get_gl_id() const noexcept
	{
		return (m_vbo);
	}
	
	template <typename T>
	bool Buffer<T>::is_valid() const noexcept
	{
		return (m_buffer_ok && m_filled);
	}
	
	template <typename T>
	void Buffer<T>::to_cpu()
	{
		try
		{
			if (!is_valid())
				return;
			
			glBindBuffer(m_internal_type, m_vbo);
			
			glGetBufferSubData(m_internal_type, 0, m_elements.size()*sizeof(T), get());
			
			glBindBuffer(m_internal_type, 0);
		}
		catch (const Error& e)
		{
			throw;
		}
	}
	
	template <typename T>
	void Buffer<T>::to_gpu()
	{
		try
		{
			if (!is_valid())
				return;
			
			glBindBuffer(m_internal_type, m_vbo);
			
			glBufferSubData(m_internal_type, 0, m_elements.size()*sizeof(T), get());
			
			glBindBuffer(m_internal_type, 0);
		}
		catch (const Error& e)
		{
			throw;
		}
	}
	
	//PRIVATE
	template <typename T>
	void Buffer<T>::ensure_buffer()
	{
		if (m_buffer_ok)
			return;
		
		try
		{
			glGenBuffers(1, &m_vbo);
			glBindBuffer(m_internal_type, m_vbo);
			
			if ((glIsBuffer(m_vbo) == GL_FALSE) || (glGetError() != GL_NO_ERROR))
				ATEMA_ERROR("OpenGL could not create VBO.")
			
			glBindBuffer(m_internal_type, 0);
			
			m_buffer_ok = true;
		}
		catch (const Error& e)
		{
			glBindBuffer(m_internal_type, 0);
			glDeleteBuffers(1, &m_vbo);
			
			m_vbo = 0;
			
			throw;
		}
	}
}

#endif