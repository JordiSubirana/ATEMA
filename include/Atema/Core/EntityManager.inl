/*
	Copyright 2022 Jordi SUBIRANA

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

#ifndef ATEMA_CORE_ENTITYMANAGER_INL
#define ATEMA_CORE_ENTITYMANAGER_INL

#include <Atema/Core/EntityManager.hpp>
#include <Atema/Core/Error.hpp>
#include <Atema/Core/TypeInfo.hpp>

namespace at
{
	namespace detail
	{
		template <typename T>
		ComponentHandler<T>::~ComponentHandler()
		{
			clear();
		}

		template <typename T>
		void ComponentHandler<T>::clear()
		{
			m_sparseSet.clear();
		}

		template <typename T>
		void ComponentHandler<T>::erase(EntityHandle entity)
		{
			m_sparseSet.erase(entity);
		}

		template <typename T>
		SparseSet<T>& ComponentHandler<T>::getSet()
		{
			return m_sparseSet;
		}

		template <typename T>
		const SparseSet<T>& ComponentHandler<T>::getSet() const
		{
			return m_sparseSet;
		}
	}

	template <typename T>
	T& EntityManager::createComponent(EntityHandle entity)
	{
		return getComponents<T>().emplace(entity);
	}

	template <typename T>
	void EntityManager::removeComponent(EntityHandle entity)
	{
		getComponents<T>().erase(entity);
	}

	template <typename T>
	bool EntityManager::hasComponent(EntityHandle entity) const
	{
		return getComponents<T>().contains[entity];
	}

	template <typename T>
	T& EntityManager::getComponent(EntityHandle entity)
	{
		return getComponents<T>()[entity];
	}

	template <typename T>
	const T& EntityManager::getComponent(EntityHandle entity) const
	{
		return getComponents<T>()[entity];
	}

	template <typename ... Args>
	SparseSetUnion<Args...> EntityManager::getUnion()
	{
		SparseSetUnion<Args...> sparseUnion(getComponents<Args>()...);
		
		return sparseUnion;
	}

	template <typename T>
	SparseSet<T>& EntityManager::getComponents()
	{
		constexpr auto typeID = TypeInfo<T>::id;

		if (!m_components.contains(typeID))
		{
			auto ptr = std::make_shared<detail::ComponentHandler<T>>();
			auto abstractPtr = std::static_pointer_cast<detail::AbstractComponentHandler>(ptr);

			m_components.insert(typeID, abstractPtr);
		}

		return std::static_pointer_cast<detail::ComponentHandler<T>>(m_components[typeID])->getSet();
	}

	template <typename T>
	const SparseSet<T>& EntityManager::getComponents() const
	{
		constexpr auto typeID = TypeInfo<T>::id;

		if (!m_components.contains(typeID))
		{
			ATEMA_ERROR("Requested component set does not exist");
		}
		
		return std::static_pointer_cast<detail::ComponentHandler<T>>(m_components[typeID])->getSet();
	}
}

#endif
