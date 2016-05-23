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

#ifndef ATEMA_GRAPHICS_INDEXED_ARRAY_HEADER
#define ATEMA_GRAPHICS_INDEXED_ARRAY_HEADER

#include "buffer.hpp"
#include <atema/graphics/index_array.hpp>

namespace at
{
	template <typename T>
	class IndexedArray : public ResourceGL
	{
		public:
			Buffer<T> elements;
			IndexArray indices;
			
		public:
			IndexedArray();
			IndexedArray(const T *elements, size_t elements_size, const unsigned int *indices = nullptr, size_t indices_size = 0, typename Buffer<T>::update_mode elements_update_mode = Buffer<T>::update_mode::static_draw, IndexArray::update_mode indices_update_mode = IndexArray::update_mode::static_draw);
			IndexedArray(const Buffer<T>& elements, const IndexArray& indices);
			IndexedArray(const IndexedArray<T>& array);
			virtual ~IndexedArray() noexcept;
			
			void create(const T *elements, size_t elements_size, const unsigned int *indices = nullptr, size_t indices_size = 0, typename Buffer<T>::update_mode elements_update_mode = Buffer<T>::update_mode::static_draw, IndexArray::update_mode indices_update_mode = IndexArray::update_mode::static_draw);
			void create(const Buffer<T>& elements, const IndexArray& indices);
			void create(const IndexedArray<T>& array);
			
			T& operator[](size_t index);
			const T& operator[](size_t index) const;
			
			size_t get_size() const;
			
			//ObjectGL
			GLuint get_gl_id() const noexcept;
			bool is_valid() const noexcept;
			
			//ResourceGL
			void to_cpu();
			void to_gpu();
	};
}

#include <atema/graphics/indexed_array.tpp>

#endif