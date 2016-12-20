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

#ifndef ATEMA_CORE_REF_HEADER
#define ATEMA_CORE_REF_HEADER

#include <Atema/Core/Config.hpp>
#include <Atema/Core/Error.hpp>

#include <atomic>
#include <type_traits>

namespace at
{
	template <typename T>
	class Ref
	{
		template <typename U>
		friend class Ref;
		
		public:
			template <typename...Args>
			static Ref<T> create(Args&... args);
			
		public:
			Ref();
			template <typename U>
			Ref(U* ptr);
			Ref(T* ptr);
			Ref(const Ref<T>& ref);
			template <typename U>
			Ref(const Ref<U>& ref);
			virtual ~Ref();
			
			unsigned int get_count() const;
			
			T* get();
			const T* get() const;
			
			template <typename U>
			Ref<U> force_static_cast() const;
			
			operator bool() const noexcept;
			
			T* operator->() const noexcept;
			T& operator*() const;
			
			Ref<T>& operator=(const Ref<T>& ref);
			
			template <typename U>
			typename std::enable_if< std::is_base_of<T, U>::value, Ref<T>& >::type operator=(U* ptr)
			{
				decrement();
				
				if (ptr)
				{
					m_count = new (std::nothrow) std::atomic_uint(1);
					
					if (m_count)
					{
						m_ptr = static_cast<T*>(ptr);
					}
					else
					{
						m_count = nullptr;
						m_ptr = nullptr;
						ATEMA_ERROR("Counter allocation failed.")
					}
				}
				else
				{
					m_count = nullptr;
					m_ptr = nullptr;
				}
			}
			
			template <typename U>
			typename std::enable_if< (std::is_base_of<U, T>::value && !std::is_same<T,U>::value), Ref<T>& >::type operator=(U* ptr)
			{
				decrement();
				
				if (ptr)
				{
					m_count = new (std::nothrow) std::atomic_uint(1);
					
					if (m_count)
					{
						m_ptr = dynamic_cast<T*>(ptr);
						if (!m_ptr)
						{
							m_count = nullptr;
							
							ATEMA_ERROR("Types are not compatible.")
						}
					}
					else
					{
						m_count = nullptr;
						m_ptr = nullptr;
						ATEMA_ERROR("Counter allocation failed.")
					}
				}
				else
				{
					m_count = nullptr;
					m_ptr = nullptr;
				}
			}
			
			template <typename U>
			typename std::enable_if< std::is_base_of<T, U>::value, Ref<T>& >::type operator=(const Ref<U>& ref)
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
			
			template <typename U>
			typename std::enable_if< (std::is_base_of<U, T>::value && !std::is_same<T,U>::value), Ref<T>& >::type operator=(const Ref<U>& ref)
			{
				ref.increment();
				
				decrement();
				
				if (ref)
				{
					m_count = ref.m_count;
					m_ptr = dynamic_cast<T*>(ref.m_ptr);
					if (!m_ptr)
					{
						m_count = nullptr;
						
						ref.decrement();
						
						ATEMA_ERROR("Types are not compatible.")
					}
				}
				else
				{
					m_count = nullptr;
					m_ptr = nullptr;
				}
			}
			
		private:
			void increment() const;
			void decrement() const;
			
			mutable T *m_ptr;
			mutable std::atomic_uint *m_count;
	};
}

#include <Atema/Core/Ref.tpp>

#endif