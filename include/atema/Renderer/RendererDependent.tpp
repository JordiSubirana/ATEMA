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

#ifndef ATEMA_RENDERER_RENDERER_DEPENDENT_IMPLEMENTATION
#define ATEMA_RENDERER_RENDERER_DEPENDENT_IMPLEMENTATION

#include <Atema/Renderer/RendererDependent.hpp>

#include <type_traits>

namespace at
{
	template <typename D, typename B>
	Ref<D> RendererDependent::cast(const Ref<B>& ref) const
	{
		static_assert(std::is_base_of<RendererDependent, B>::value, "Cast is only valid for derived of RendererDependent.");
		static_assert(std::is_base_of<B, D>::value, "The requested target type is not compatible with the base type.");
		
		if ((m_renderer == ref->m_renderer) || (m_renderer->is_compatible_with(*(ref->m_renderer))))
		{
			return (ref.template force_static_cast<D>());
		}
		else
		{
			return (ref); //Let Ref class try dynamic_cast
		}
	}
}

#endif