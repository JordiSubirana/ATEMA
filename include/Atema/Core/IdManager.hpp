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

#ifndef ATEMA_CORE_IDMANAGER_HPP
#define ATEMA_CORE_IDMANAGER_HPP

#include <Atema/Core/Config.hpp>

#include <numeric>
#include <set>

namespace at
{
	template <typename T>
	class IdManager
	{
	public:
		static_assert(std::numeric_limits<T>::is_integer, "T must be an integer type");

		IdManager();
		IdManager(const IdManager& other) = default;
		IdManager(IdManager&& other) noexcept = default;
		~IdManager() = default;

		// Get a new ID
		T get();
		// Release an existing ID to make it available again
		void release(T id);

		IdManager& operator=(const IdManager& other) = default;
		IdManager& operator=(IdManager&& other) noexcept = default;

	private:
		T m_nextId;
		std::set<T> m_availableIds;
	};
}

#include <Atema/Core/IdManager.inl>

#endif