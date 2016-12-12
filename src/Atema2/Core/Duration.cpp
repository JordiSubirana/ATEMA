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

#include <Atema/Core/Duration.hpp>
#include <thread>

namespace at
{
	float Duration::s() const noexcept
	{
		return _d*1e-6f;
	}
	
	float Duration::ms() const noexcept
	{
		return _d*1e-3f;
	}
	
	unsigned long long Duration::us() const noexcept
	{
		return _d;
	}
	
	void sleep(const Duration &d)
	{
		std::this_thread::sleep_for(d.to_std_duration());
	}
	
	std::chrono::duration<unsigned long long, std::micro> Duration::to_std_duration() const noexcept
	{
		std::chrono::duration<unsigned long long, std::micro> std_d(_d);
		return (std_d);
	}
}