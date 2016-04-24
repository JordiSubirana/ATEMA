#ifndef ATEMA_GRAPHICS_ARRAY_IMPL
#define ATEMA_GRAPHICS_ARRAY_IMPL

#include <atema/core/error.hpp>

namespace at
{
	template <typename T>
	array<T>::array()
	{
		
	}
	
	template <typename T>
	array<T>::array(const T *elements, size_t elements_size, update update_mode)
	{
		create(elements, elements_size, update_mode);
	}
	
	template <typename T>
	array<T>::array(const array<T>& array)
	{
		create(array);
	}
	
	template <typename T>
	array<T>::~array() noexcept
	{
		
	}
	
	template <typename T>
	void array<T>::create(const T *elements, size_t elements_size, update update_mode)
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
			
			//Flush gl error
			glGetError();
			
			//-----
			//Creation
			glGenBuffers(1, &(tmp.vbo));
			
			glBindBuffer(GL_ARRAY_BUFFER, tmp.vbo);
			
			if ((glIsBuffer(tmp.vbo) == GL_FALSE) || (glGetError() != GL_NO_ERROR))
				ATEMA_ERROR("OpenGL could not create VBO.")
			
			glBufferData(GL_ARRAY_BUFFER, elements_size*sizeof(T), nullptr, update_mode);
			
			if (glGetError() != GL_NO_ERROR)
				ATEMA_ERROR("OpenGL could not allocate VBO.")
			
			glBufferSubData(GL_ARRAY_BUFFER, 0, elements_size*sizeof(T), elements);
			
			if (glGetError() != GL_NO_ERROR)
				ATEMA_ERROR("OpenGL could not fill VBO.")
			
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			//-----
			
			m_data.free(); //Destroy previous VBO
			
			m_data.vbo = data.vbo;
			m_data.elements.swap(data.elements);
			m_data.elements_size = data.elements_size;
			m_data.update_mode = data.update_mode;
			
			data.vbo = 0; //Ensure that VBO will not be destroyed
			
		}
		catch (const error& e)
		{
			throw;
		}
	}
	
	template <typename T>
	void array<T>::create(const array<T>& array)
	{
		create(array.get(), array.get_size(), array.get_update_mode());
	}
	
	template <typename T>
	T* array<T>::get() noexcept
	{
		if (get_size() == 0)
			return (nullptr);
		
		return (m_data.elements.data());
	}
	
	template <typename T>
	const T* array<T>::get() const noexcept
	{
		if (get_size() == 0)
			return (nullptr);
		
		return (m_data.elements.data());
	}
	
	template <typename T>
	T& array<T>::operator[](size_t index)
	{
		return (m_data.elements[index]);
	}
	
	template <typename T>
	const T& array<T>::operator[](size_t index) const
	{
		return (m_data.elements[index]);
	}
	
	template <typename T>
	size_t array<T>::get_size() const
	{
		return (m_data.elements.size());
	}
	
	template <typename T>
	typename array<T>::update array<T>::get_update_mode() const
	{
		return (m_data.update_mode);
	}
	
	template <typename T>
	GLuint array<T>::get_gl_id() const
	{
		return (m_data.vbo);
	}
	
	template <typename T>
	void array<T>::download()
	{
		try
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_data.vbo);
			
			glGetBufferSubData(GL_ARRAY_BUFFER, 0, m_data.elements_size*sizeof(T), get());
			
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		catch (const error& e)
		{
			throw;
		}
	}
	
	template <typename T>
	void array<T>::upload()
	{
		try
		{
			glBindBuffer(GL_ARRAY_BUFFER, tmp.vbo);
			
			glBufferSubData(GL_ARRAY_BUFFER, 0, tmp.elements_size*sizeof(T), get());
			
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		catch (const error& e)
		{
			throw;
		}
	}
}

#endif