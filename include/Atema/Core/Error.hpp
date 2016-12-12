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

#ifndef ATEMA_ERROR_HEADER
#define ATEMA_ERROR_HEADER

#include <Atema/Core/Config.hpp>

#include <exception>
#include <stdexcept>

#define ATEMA_ERROR(desc) throw at::Error(desc);

namespace at
{
	class ATEMA_CORE_API Error : public std::runtime_error
	{
		public:
			Error(const char *description) noexcept;
			virtual ~Error() noexcept;
	};
}

#endif