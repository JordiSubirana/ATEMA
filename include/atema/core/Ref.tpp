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

#ifndef ATEMA_CORE_REF_IMPLEMENTATION
#define ATEMA_CORE_REF_IMPLEMENTATION

#include <Atema/Core/Ref.hpp>
#include <Atema/Core/Error.hpp>

namespace at
{
	template <typename T>
	template <typename...Args>
	Ref<T> Ref<T>::create(Args&... args)
	{
		Ref<T> ref;
		T* var = nullptr;
		
		try
		{
			T* var = new T(args...);
			
			ref = var;
		}
		catch (...)
		{
			if (var)
				delete var;
			
			ATEMA_ERROR("Failed to create Ref object.")
		}
		
		return (ref);
	}
	
	template <typename T>
	Ref<T>::Ref()
	{
		m_ptr = nullptr;
		m_count = nullptr;
	}
	
	template <typename T>
	template <typename U>
	Ref<T>::Ref(U* ptr) :
		Ref()
	{
		(*this) = ptr;
	}
	
	template <typename T>
	Ref<T>::Ref(T* ptr) :
		Ref()
	{
		(*this) = ptr;
	}
	
	template <typename T>
	template <typename U>
	Ref<T>::Ref(const Ref<U>& ref) :
		Ref()
	{
		(*this) = ref;
	}
	
	template <typename T>
	Ref<T>::Ref(const Ref<T>& ref) :
		Ref()
	{
		(*this) = ref;
	}
	
	template <typename T>
	Ref<T>::~Ref()
	{
		decrement();
	}
	
	template <typename T>
	unsigned int Ref<T>::get_count() const
	{
		if (m_count)
			return (static_cast<unsigned int>(*m_count));
		
		return (0);
	}
	
	template <typename T>
	T* Ref<T>::get()
	{
		return (m_ptr);
	}
	
	template <typename T>
	const T* Ref<T>::get() const
	{
		return (m_ptr);
	}
	
	template <typename T>
	template <typename U>
	Ref<U> Ref<T>::force_static_cast() const
	{
		Ref<U> ref;
		
		increment();
		
		ref.m_count = m_count;
		ref.m_ptr = static_cast<U*>(m_ptr);
		
		return (ref);
	}
	
	template <typename T>
	Ref<T>::operator bool() const noexcept
	{
		return (m_ptr);
	}
	
	template <typename T>
	T* Ref<T>::operator->() const noexcept
	{
		return (m_ptr);
	}
	
	template <typename T>
	T& Ref<T>::operator*() const
	{
		if (!m_ptr)
			ATEMA_ERROR("Ref holds a null pointer.")
		
		return (*m_ptr);
	}
	
	template <typename T>
	Ref<T>& Ref<T>::operator=(const Ref<T>& ref)
	{
		ref.increment();
		
		decrement();
		
		if (ref)
		{
			m_count = ref.m_count;
			m_ptr = static_cast<T*>(ref.m_ptr);
		}
		else
		{
			m_count = nullptr;
			m_ptr = nullptr;
		}
	}
	
	template <typename T>
	void Ref<T>::increment() const
	{
		if (m_count)
			(*m_count)++;
	}
	
	template <typename T>
	void Ref<T>::decrement() const
	{
		if (m_count)
		{
			if (--(*m_count) == 0)
			{
				delete m_count;
				
				if (m_ptr)
					delete m_ptr;
				
				m_count = nullptr;
				m_ptr = nullptr;
			}
		}
	}
}

#endif