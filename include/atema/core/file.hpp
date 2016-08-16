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

#ifndef ATEMA_CORE_FILE_HEADER
#define ATEMA_CORE_FILE_HEADER

#include <atema/core/config.hpp>

#include <string>

namespace at
{
	class ATEMA_CORE_API File
	{
		public:
			File() = delete;
			File(const File& file) = delete;
			virtual ~File() = delete;
			
			static std::string get_extension(const std::string& str);
			static std::string get_path(const std::string& str);
			
			static bool extension_match(const std::string& filename, const std::string& extension);
	};
}

#endif