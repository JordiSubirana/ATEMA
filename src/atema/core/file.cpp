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

#include <atema/core/file.hpp>
#include <atema/core/error.hpp>

namespace at
{
	std::string File::get_extension(const std::string& str)
	{
		size_t index = str.find_last_of('.');
		
		return ( str.substr(index+1, str.size()-1-index) );
	}
	
	std::string File::get_path(const std::string& str)
	{
		return ( str.substr(0, str.find_last_of('/')+1) );
	}
	
	bool File::extension_match(const std::string& filename, const std::string& extension)
	{
		if (filename.size() < extension.size()+2)
			return (false);
		
		return (extension.compare(get_extension(filename)) == 0);
	}
}