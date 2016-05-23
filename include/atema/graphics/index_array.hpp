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

#ifndef ATEMA_GRAPHICS_INDEX_ARRAY_HEADER
#define ATEMA_GRAPHICS_INDEX_ARRAY_HEADER

#include "buffer.hpp"
#include <atema/core/error.hpp>

#include <vector>

namespace at
{
	class IndexArray : public Buffer<unsigned int>
	{
		public:
			using Buffer<unsigned int>::create;
			using Buffer<unsigned int>::get;
			using Buffer<unsigned int>::get_gl_id;
			using Buffer<unsigned int>::is_valid;
			using Buffer<unsigned int>::to_cpu;
			using Buffer<unsigned int>::to_gpu;
			using Buffer<unsigned int>::operator[];
			using Buffer<unsigned int>::get_size;
			using Buffer<unsigned int>::get_update_mode;
		
		public:
			IndexArray();
			IndexArray(const unsigned int *elements, size_t elements_size, update_mode update_mode = update_mode::static_draw);
			IndexArray(const Buffer<unsigned int>& array);
			IndexArray(const IndexArray& array);
			virtual ~IndexArray() noexcept;
			
			void create(const IndexArray& array);
			
			template <typename T>
			void generate(const Buffer<T>& array);
	};
	
	template <typename T>
	void IndexArray::generate(const Buffer<T>& array)
	{
		//TODO: Handle vector exceptions
		std::vector<unsigned int> tmp;
		
		if (!array.is_valid())
			ATEMA_ERROR("Buffer is invalid.")
		
		tmp.resize(array.get_size());
		
		for (unsigned int i = 0; i <= tmp.size(); i++)
			tmp[i] = i;
		
		Buffer<unsigned int>::create(tmp.data(), tmp.size(), get_update_mode());
	}
}

#endif