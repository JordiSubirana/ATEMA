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

#include <atema/graphics/buffer_array.hpp>
#include <atema/core/error.hpp>

#include <vector>

namespace at
{
	class IndexArray : public BufferArray<unsigned int>
	{
		public:
			using BufferArray<unsigned int>::create;
			using BufferArray<unsigned int>::get;
			using BufferArray<unsigned int>::get_gl_id;
			using BufferArray<unsigned int>::is_valid;
			using BufferArray<unsigned int>::to_cpu;
			using BufferArray<unsigned int>::to_gpu;
			using BufferArray<unsigned int>::operator[];
			using BufferArray<unsigned int>::get_size;
			using BufferArray<unsigned int>::get_update_mode;
		
		public:
			IndexArray();
			IndexArray(const unsigned int *elements, size_t elements_size, update_mode update_mode = update_mode::static_draw);
			IndexArray(const BufferArray<unsigned int>& array);
			IndexArray(const IndexArray& array);
			virtual ~IndexArray() noexcept;
			
			void create(const IndexArray& array);
			
			template <typename T>
			void generate(const BufferArray<T>& array);
	};
	
	template <typename T>
	void IndexArray::generate(const BufferArray<T>& array)
	{
		//TODO: Handle vector exceptions
		std::vector<unsigned int> tmp;
		
		if (!array.is_valid())
			ATEMA_ERROR("BufferArray is invalid.")
		
		tmp.resize(array.get_size());
		
		for (unsigned int i = 0; i <= tmp.size(); i++)
			tmp[i] = i;
		
		BufferArray<unsigned int>::create(tmp.data(), tmp.size(), get_update_mode());
	}
}

#endif