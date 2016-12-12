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

#ifndef ATEMA_CORE_DURATION_HEADER
#define ATEMA_CORE_DURATION_HEADER

#include <Atema/Core/Config.hpp>

#include <chrono>

namespace at
{
    class Duration;
	
    template <typename T>
    Duration second(T) noexcept;
	
    template <typename T>
    Duration msecond(T) noexcept;
	
    template <typename T>
    Duration usecond(T) noexcept;
	
    class ATEMA_CORE_API Duration
	{
        template <typename T>
		friend Duration second(T) noexcept;
		
		template <typename T>
		friend Duration msecond(T) noexcept;
		
		template <typename T>
		friend Duration usecond(T) noexcept;
		
		public:
			float s() const noexcept;
			float ms() const noexcept;
			unsigned long long us() const noexcept;
			
			std::chrono::duration<unsigned long long, std::micro> to_std_duration() const noexcept ;
			
		private:
			unsigned long long _d; // us
    };
	
    template <typename T>
    Duration second(T t) noexcept
	{
        T factor = 1e6;
        Duration d;
        d._d = static_cast<unsigned long long>(t*factor);
        return (d);
    }
	
    template <typename T>
    Duration msecond(T t) noexcept
	{
        T factor = 1e3;
        Duration d;
        d._d = static_cast<unsigned long long>(t*factor);
        return (d);
    }
	
    template <typename T>
    Duration usecond(T t) noexcept
	{
        Duration d;
        d._d = static_cast<unsigned long long>(t);
        return (d);
    }
	
    void sleep(Duration const& d);
}

#endif