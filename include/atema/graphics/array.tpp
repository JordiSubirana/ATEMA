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

#ifndef ATEMA_GRAPHICS_ARRAY_IMPL
#define ATEMA_GRAPHICS_ARRAY_IMPL

#include <atema/core/error.hpp>

namespace at
{
	//PUBLIC
	template <typename T>
	Array<T>::Array()
	{
		
	}
	
	template <typename T>
	Array<T>::Array(const T *elements, size_t elements_size, update update_mode)
	{
		create(elements, elements_size, update_mode);
	}
	
	template <typename T>
	Array<T>::Array(const Array<T>& array)
	{
		create(array);
	}
	
	template <typename T>
	Array<T>::~Array() noexcept
	{
		
	}
	
	template <typename T>
	void Array<T>::create(const T *elements, size_t elements_size, update update_mode)
	{
		data tmp;
		
		try
		{
			if (!elements || !elements_size)
				ATEMA_ERROR("No elements given.")
			
			tmp.vbo = 0;
			tmp.elements.resize(elements_size);
			tmp.elements_size = elements_size;
			tmp.update_mode = update_mode;
			
			for (size_t i = 0; i < elements_size; i++)
			{
				tmp.elements[i] = elements[i];
			}
			
			//Flush gl Error
			glGetError();
			
			//-----
			//Creation
			glGenBuffers(1, &(tmp.vbo));
			
			glBindBuffer(get_internal_type(), tmp.vbo);
			
			if ((glIsBuffer(tmp.vbo) == GL_FALSE) || (glGetError() != GL_NO_ERROR))
				ATEMA_ERROR("OpenGL could not create VBO.")
			
			glBufferData(get_internal_type(), elements_size*sizeof(T), nullptr, static_cast<GLenum>(update_mode));
			
			if (glGetError() != GL_NO_ERROR)
				ATEMA_ERROR("OpenGL could not allocate VBO.")
			
			glBufferSubData(get_internal_type(), 0, elements_size*sizeof(T), elements);
			
			if (glGetError() != GL_NO_ERROR)
				ATEMA_ERROR("OpenGL could not fill VBO.")
			
			glBindBuffer(get_internal_type(), 0);
			//-----
			
			m_data.free(); //Destroy previous VBO
			
			m_data.vbo = tmp.vbo;
			m_data.elements.swap(tmp.elements);
			m_data.elements_size = tmp.elements_size;
			m_data.update_mode = tmp.update_mode;
			
			tmp.vbo = 0; //Ensure that VBO will not be destroyed
			
		}
		catch (const Error& e)
		{
			throw;
		}
	}
	
	template <typename T>
	void Array<T>::create(const Array<T>& array)
	{
		create(array.get(), array.get_size(), array.get_update_mode());
	}
	
	template <typename T>
	T* Array<T>::get() noexcept
	{
		if (get_size() == 0)
			return (nullptr);
		
		return (m_data.elements.data());
	}
	
	template <typename T>
	const T* Array<T>::get() const noexcept
	{
		if (get_size() == 0)
			return (nullptr);
		
		return (m_data.elements.data());
	}
	
	template <typename T>
	T& Array<T>::operator[](size_t index)
	{
		return (m_data.elements[index]);
	}
	
	template <typename T>
	const T& Array<T>::operator[](size_t index) const
	{
		return (m_data.elements[index]);
	}
	
	template <typename T>
	size_t Array<T>::get_size() const
	{
		return (m_data.elements.size());
	}
	
	template <typename T>
	typename Array<T>::update Array<T>::get_update_mode() const
	{
		return (m_data.update_mode);
	}
	
	template <typename T>
	GLuint Array<T>::get_gl_id() const
	{
		return (m_data.vbo);
	}
	
	template <typename T>
	void Array<T>::download()
	{
		try
		{
			glBindBuffer(get_internal_type(), m_data.vbo);
			
			glGetBufferSubData(get_internal_type(), 0, m_data.elements_size*sizeof(T), get());
			
			glBindBuffer(get_internal_type(), 0);
		}
		catch (const Error& e)
		{
			throw;
		}
	}
	
	template <typename T>
	void Array<T>::upload()
	{
		try
		{
			glBindBuffer(get_internal_type(), m_data.vbo);
			
			glBufferSubData(get_internal_type(), 0, m_data.elements_size*sizeof(T), get());
			
			glBindBuffer(get_internal_type(), 0);
		}
		catch (const Error& e)
		{
			throw;
		}
	}
	
	//PROTECTED
	template <typename T>
	GLenum Array<T>::get_buffer_type() const noexcept
	{
		return (GL_ARRAY_BUFFER);
	}
	
	template <typename T>
	GLenum Array<T>::get_internal_type() const noexcept
	{
		return (this->get_buffer_type());
	}
}

#endif