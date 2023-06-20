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

#ifndef ATEMA_CORE_RESOURCEMANAGER_HPP
#define ATEMA_CORE_RESOURCEMANAGER_HPP

#include <Atema/Core/Config.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Core/Hash.hpp>

#include <unordered_map>
#include <filesystem>
#include <functional>

namespace at
{
	class ATEMA_CORE_API AbstractResourceManager
	{
	public:
		AbstractResourceManager();
		virtual ~AbstractResourceManager();

		// When a resource is not used for a number of iterations, it is removed
		// Default value is 0, meaning unused resources will be removed at each call of clearUnused()
		virtual void setMaxUnusedCounter(uint32_t counter) = 0;

		// Updates the internal counter of each resource depending if it is in use
		// Then remove unused ones, depending on the max unused counter
		virtual void clearUnused() = 0;

		// Forces the deletion of every resource no matter it is in use or not
		// If the user owns a resource at this time (saving a Ptr to the resource) it will not be deleted
		virtual void clear() = 0;
	};

	// Class offering resource management for a given type T, using ID class as an identifier (defaulted to std::filesystem::path)
	// The user must provide at least one resource loader (function creating a Ptr<T> from a const ID&)
	template <typename T, typename ID = std::filesystem::path>
	class ResourceManager : public AbstractResourceManager
	{
	public:
		using Loader = std::function<Ptr<T>(const ID&)>;
		using Hasher = std::function<StdHash(const ID&)>;
		using Deleter = std::function<void(Ptr<T>)>;

		ResourceManager();
		~ResourceManager();

		// Returns true if a resource with the corresponding id exists
		bool contains(const ID& id) const;

		// If the resource exists, returns it
		// Otherwise, the resource is created using create(const ID& id)
		Ptr<T> get(const ID& id);

		// Associates a resource to the corresponding ID and sets the unused counter to 0
		void set(const ID& id, const Ptr<T>& resource);

		// Add a loader to the manager
		// When creating a new resource, the manager will call all loaders until one succeeds to create a resource
		// The loaders are called in insertion order
		void addLoader(const Loader& loader);

		// Sets a custom hash function for ID type
		// Default : DefaultStdHasher::hash(const ID&)
		void setHasher(const Hasher& hasher);

		// When a resource is removed, it is moved to the Deleter
		// The default behaviour is a simple deletion (Ptr<T>::reset())
		void setDeleter(const Deleter& deleter);

		// When a resource is not used for a number of iterations, it is removed
		// Default value is 0, meaning unused resources will be removed at each call of clearUnused()
		void setMaxUnusedCounter(uint32_t counter) override;

		// Updates the internal counter of each resource depending if it is in use
		// Then remove unused ones, depending on the max unused counter
		void clearUnused() override;

		// Forces the deletion of every resource no matter it is in use or not
		// If the user owns a resource at this time (saving a Ptr to the resource) it will not be deleted
		void clear();

		// Forces the deletion of a particular value
		void remove(const T* value);

	private:
		StdHash getHash(const ID& id) const;

		Ptr<T> load(const ID& id);

		struct ResourceData
		{
			Ptr<T> resource;
			uint32_t counter = 0;
		};

		std::unordered_map<std::size_t, ResourceData> m_resources;
		std::unordered_map<const T*, std::vector<size_t>> m_resourceIndices;

		uint32_t m_maxUnusedCounter;

		std::vector<Loader> m_loaders;
		Hasher m_hasher;
		Deleter m_deleter;
	};
}

#include <Atema/Core/ResourceManager.inl>

#endif
