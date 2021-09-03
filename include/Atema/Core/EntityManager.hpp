/*
	Copyright 2021 Jordi SUBIRANA

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

#ifndef ATEMA_CORE_ENTITYMANAGER_HPP
#define ATEMA_CORE_ENTITYMANAGER_HPP

#include <Atema/Core/Config.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Core/SparseSet.hpp>
#include <Atema/Core/SparseSetUnion.hpp>

#include <list>

namespace at
{
	using EntityHandle = std::uint32_t;

	namespace detail
	{
		class ATEMA_CORE_API AbstractComponentHandler
		{
		public:
			AbstractComponentHandler();
			virtual ~AbstractComponentHandler();

			virtual void clear() = 0;
			virtual void erase(EntityHandle entity) = 0;
		};
		
		template <typename T>
		class ComponentHandler : public AbstractComponentHandler
		{
		public:
			ComponentHandler() = default;
			virtual ~ComponentHandler();

			void clear() override;
			void erase(EntityHandle entity) override;

			SparseSet<T>& getSet();
			const SparseSet<T>& getSet() const;
		
		private:
			SparseSet<T> m_sparseSet;
		};
	}
	
	class ATEMA_CORE_API EntityManager : public NonCopyable
	{
	public:
		EntityManager();
		virtual ~EntityManager();

		EntityHandle createEntity();
		
		void removeEntity(EntityHandle entity);

		void clear();

		template <typename T>
		T& createComponent(EntityHandle entity);

		template <typename T>
		void removeComponent(EntityHandle entity);

		template <typename T>
		bool hasComponent(EntityHandle entity) const;

		template <typename T>
		T& getComponent(EntityHandle entity);
		template <typename T>
		const T& getComponent(EntityHandle entity) const;

		template <typename...Args>
		SparseSetUnion<Args...> getUnion();
		
	private:
		template <typename T>
		SparseSet<T>& getComponents();
		template <typename T>
		const SparseSet<T>& getComponents() const;

		EntityHandle getNextID();

		std::list<EntityHandle> m_availableIDs;

		EntityHandle m_nextID;
		
		SparseSet<Ptr<detail::AbstractComponentHandler>> m_components;
	};
}

#include <Atema/Core/EntityManager.inl>

#endif