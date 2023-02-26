/*
	Copyright 2023 Jordi SUBIRANA

	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
	Software, and to permit persons to whom the Software is furnished to do so, subject
	to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
	PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
	CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
	OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef ATEMA_CORE_IDMANAGER_INL
#define ATEMA_CORE_IDMANAGER_INL

#include <Atema/Core/IdManager.hpp>

namespace at
{
	template <typename T>
	IdManager<T>::IdManager() :
		m_nextId(0)
	{

	}

	template <typename T>
	T IdManager<T>::get()
	{
		// Recycle an ID if possible
		if (!m_availableIds.empty())
			return m_availableIds.extract(m_availableIds.begin()).value();
		
		return m_nextId++;
	}
	
	template <typename T>
	void IdManager<T>::release(T id)
	{
		// Ensure the ID was previously allocated (m_nextId is the max ID possible)
		if (id < m_nextId)
			m_availableIds.emplace(id);
	}
}

#endif