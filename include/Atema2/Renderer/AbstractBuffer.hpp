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

#ifndef ATEMA_RENDERER_ABSTRACT_BUFFER_HEADER
#define ATEMA_RENDERER_ABSTRACT_BUFFER_HEADER

#include <Atema/Renderer/Config.hpp>
#include <Atema/Core/Ref.hpp>

#include <vector>

namespace at
{
	class ATEMA_RENDERER_API AbstractBuffer
	{
		public:
			AbstractBuffer() = default;
			virtual ~AbstractBuffer() = 0;
			
			virtual bool is_mapped() const = 0;
			
			virtual void* map() = 0;
			virtual void unmap() const = 0;
			
			template <typename T>
			T* map();
	};
}

#include <Atema/Renderer/AbstractBuffer.tpp>

#endif