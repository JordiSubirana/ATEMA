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

#include <Atema/Core/Color.hpp>
#include <Atema/Core/Error.hpp>

namespace at
{
	Color::Color() :
		r(0.0f), g(0.0f), b(0.0f), a(0.0f)
	{
		
	}
	
	Color::~Color()
	{
		
	}
	
	float& Color::operator[](size_t index)
	{
		if (index >= 4)
			ATEMA_ERROR("Index is greater than array size.")
		
		return (reinterpret_cast<float*>(this)[index]);
	}
	
	const float& Color::operator[](size_t index) const
	{
		if (index >= 4)
			ATEMA_ERROR("Index is greater than array size.")
		
		return (reinterpret_cast<const float*>(this)[index]);
	}
}