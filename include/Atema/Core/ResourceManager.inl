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

#ifndef ATEMA_CORE_RESOURCEMANAGER_INL
#define ATEMA_CORE_RESOURCEMANAGER_INL

#include <Atema/Core/ResourceManager.hpp>
#include <Atema/Core/Error.hpp>

namespace at
{
	template <typename T, typename ID>
	ResourceManager<T, ID>::ResourceManager() :
		m_maxUnusedCounter(0)
	{
		StdHash(*defaultHasher)(const ID&) = DefaultStdHasher::hash;

		m_hasher = defaultHasher;

		m_deleter = [](Ptr<T> resource)
		{
			resource.reset();
		};
	}

	template <typename T, typename ID>
	ResourceManager<T, ID>::~ResourceManager()
	{
	}

	template <typename T, typename ID>
	bool ResourceManager<T, ID>::contains(const ID& id) const
	{
		return m_resources.count(getHash(id)) > 0;
	}

	template <typename T, typename ID>
	Ptr<T> ResourceManager<T, ID>::get(const ID& id)
	{
		const auto it = m_resources.find(getHash(id));

		if (it != m_resources.end())
			return it->second.resource;

		return load(id);
	}

	template <typename T, typename ID>
	void ResourceManager<T, ID>::set(const ID& id, const Ptr<T>& resource)
	{
		m_resources[getHash(id)] = {resource, 0};
	}

	template <typename T, typename ID>
	void ResourceManager<T, ID>::addLoader(const Loader& loader)
	{
		if (!loader)
			ATEMA_ERROR("Invalid loader function");

		m_loaders.emplace_back(loader);
	}

	template <typename T, typename ID>
	void ResourceManager<T, ID>::setHasher(const Hasher& hasher)
	{
		if (!hasher)
			ATEMA_ERROR("Invalid hasher function");

		m_hasher = hasher;
	}

	template <typename T, typename ID>
	void ResourceManager<T, ID>::setDeleter(const Deleter& deleter)
	{
		if (!deleter)
			ATEMA_ERROR("Invalid deleter function");

		m_deleter = deleter;
	}

	template <typename T, typename ID>
	void ResourceManager<T, ID>::setMaxUnusedCounter(uint32_t counter)
	{
		m_maxUnusedCounter = counter;
	}

	template <typename T, typename ID>
	void ResourceManager<T, ID>::clearUnused()
	{
		for (auto it = m_resources.begin(); it != m_resources.end();)
		{
			// Is the resource used
			if (it->second.resource.use_count() > 1)
			{
				it->second.counter = 0;

				it++;
			}
			// Otherwise we increment the counter, then we delete the resource if needed
			else
			{
				it->second.counter++;

				if (it->second.counter > m_maxUnusedCounter)
				{
					m_deleter(std::move(it->second.resource));

					it = m_resources.erase(it);
				}
				else
				{
					it++;
				}
			}
		}
	}

	template <typename T, typename ID>
	void ResourceManager<T, ID>::clear()
	{
		for (auto& resourceKV : m_resources)
			m_deleter(std::move(resourceKV.second.resource));

		m_resources.clear();
	}

	template <typename T, typename ID>
	StdHash ResourceManager<T, ID>::getHash(const ID& id) const
	{
		return m_hasher(id);
	}

	template <typename T, typename ID>
	Ptr<T> ResourceManager<T, ID>::load(const ID& id)
	{
		Ptr<T> resource;

		for (const auto& loader : m_loaders)
		{
			resource = loader(id);

			if (resource)
				break;
		}

		if (!resource)
			ATEMA_ERROR("No loader was able to load the resource");

		set(id, resource);

		return resource;
	}
}

#endif
