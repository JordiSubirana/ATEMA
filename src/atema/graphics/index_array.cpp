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
		m_internal_type = GL_ELEMENT_ARRAY_BUFFER;
	}
	
	IndexArray::IndexArray(const unsigned int *elements, size_t elements_size, update_mode update_mode) :
		BufferArray<unsigned int>(elements, elements_size, update_mode)
	{
		m_internal_type = GL_ELEMENT_ARRAY_BUFFER;
	}
	
	IndexArray::IndexArray(const BufferArray<unsigned int>& array) :
		BufferArray<unsigned int>(array)
	{
		m_internal_type = GL_ELEMENT_ARRAY_BUFFER;
	}
	
	IndexArray::IndexArray(const IndexArray& array) :
		BufferArray<unsigned int>(static_cast< BufferArray<unsigned int> >(array))
	{
		m_internal_type = GL_ELEMENT_ARRAY_BUFFER;
	}
	
	IndexArray::~IndexArray() noexcept
	{
		
	}
	
	void IndexArray::create(const IndexArray& array)
	{
		BufferArray<unsigned int>::create(static_cast< BufferArray<unsigned int> >(array));
	}
}