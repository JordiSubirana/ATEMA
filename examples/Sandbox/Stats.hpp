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

#ifndef ATEMA_SANDBOX_STATS_HPP
#define ATEMA_SANDBOX_STATS_HPP

#include <cstdint>
#include <map>
#include <string>

class Stats
{
public:
	using Type = int64_t;
	using Map = std::map<std::string, Type>;

	Stats() = default;
	~Stats() = default;

	static Stats& instance();

	Map::iterator begin() noexcept;
	Map::const_iterator begin() const noexcept;

	Map::iterator end() noexcept;
	Map::const_iterator end() const noexcept;

	size_t size() const noexcept;
	void clear();

	Type& operator[](const std::string& name);
	const Type& operator[](const std::string& name) const;

private:
	Map m_stats;
};

#endif