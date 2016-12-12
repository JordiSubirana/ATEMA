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

#include <Atema/Renderer/Renderer.hpp>
#include <Atema/Renderer/VertexBuffer.hpp>
#include <Atema/Renderer/IndexBuffer.hpp>
#include <Atema/Renderer/Texture.hpp>
#include <Atema/Core/Error.hpp>

namespace at
{
	thread_local Renderer *s_current;
	
	Renderer::Renderer() :
		m_is_current(false)
	{
		set_current();
	}
	
	Renderer::~Renderer()
	{
		if (this == s_current)
			s_current = nullptr;
	}
	
	bool Renderer::is_current()
	{
		return (m_is_current);
	}
	
	Renderer* Renderer::get_current()
	{
		return (s_current);
	}
	
	void Renderer::set_current()
	{
		if (s_current)
			s_current->m_is_current = false;
		
		s_current = this;
		m_is_current = true;
	}
	
	bool Renderer::is_compatible_with(const Renderer& renderer) const
	{
		return (this == &renderer);
	}
}