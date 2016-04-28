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

#include <atema/graphics/array.hpp>

namespace at
{
	class IndexArray : public Array<unsigned int>
	{
		public:
			IndexArray();
			IndexArray(const unsigned int *elements, size_t elements_size, update update_mode = update::static_mode);
			IndexArray(const Array<unsigned int>& array);
			IndexArray(const IndexArray& array);
			virtual ~IndexArray() noexcept;
			
			void create(const IndexArray& array);
			
		protected:
			virtual GLenum get_buffer_type() const noexcept;
	};
}

#endif