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

#include <atema/graphics/index_array.hpp>

namespace at
{
	//PUBLIC
	IndexArray::IndexArray()
	{
		
	}
	
	IndexArray::IndexArray(const unsigned int *elements, size_t elements_size, update update_mode) :
		Array<unsigned int>(elements, elements_size, update_mode)
	{
		
	}
	
	IndexArray::IndexArray(const Array<unsigned int>& array) :
		Array<unsigned int>(array)
	{
		
	}
	
	IndexArray::IndexArray(const IndexArray& array) :
		Array<unsigned int>(static_cast< Array<unsigned int> >(array))
	{
		
	}
	
	IndexArray::~IndexArray() noexcept
	{
		
	}
	
	void IndexArray::create(const IndexArray& array)
	{
		Array<unsigned int>::create(static_cast< Array<unsigned int> >(array));
	}
	
	//PROTECTED
	GLenum IndexArray::get_buffer_type() const noexcept
	{
		return (GL_ELEMENT_ARRAY_BUFFER);
	}
}