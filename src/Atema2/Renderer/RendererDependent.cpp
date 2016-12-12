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

#include <Atema/Renderer/RendererDependent.hpp>
#include <Atema/Core/Error.hpp>

namespace at
{
	RendererDependent::RendererDependent() :
		m_renderer(nullptr)
	{
		
	}
	
	RendererDependent::~RendererDependent()
	{
		
	}
	
	Renderer* RendererDependent::get_renderer()
	{
		return (m_renderer);
	}
	
	const Renderer* RendererDependent::get_renderer() const
	{
		return (m_renderer);
	}
	
	void RendererDependent::set_renderer(Renderer *renderer)
	{
		if (!renderer)
			ATEMA_ERROR("A valid Renderer object is needed.")
		
		m_renderer = renderer;
	}
}