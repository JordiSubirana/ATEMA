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

#include <atema/graphics/renderer.hpp>
#include <atema/core/error.hpp>

namespace at
{
	Renderer::Renderer() :
		m_target(nullptr),
		m_shader(nullptr),
		m_polygon_mode(polygon_mode::surfaces)
	{
		
	}
	
	Renderer::~Renderer() noexcept
	{
		
	}
	
	void Renderer::set_target(RenderTarget *target)
	{
		m_target = target;
	}
	
	RenderTarget* Renderer::get_target()
	{
		return (m_target);
	}
	
	const RenderTarget* Renderer::get_target() const
	{
		return (m_target);
	}
	
	void Renderer::set_shader(Shader *shader)
	{
		m_shader = shader;
	}
	
	Shader* Renderer::get_shader()
	{
		return (m_shader);
	}
	
	const Shader* Renderer::get_shader() const
	{
		return (m_shader);
	}
	
	void Renderer::set_polygon_mode(Renderer::polygon_mode mode)
	{
		m_polygon_mode = mode;
	}
	
	Renderer::polygon_mode Renderer::get_polygon_mode() const noexcept
	{
		return (m_polygon_mode);
	}
	
	void Renderer::draw(Drawable& drawable)
	{
		if (!(m_target && m_target->is_valid()))
			ATEMA_ERROR("No render target available.")
		
		if (!(m_shader && m_shader->is_valid()))
			ATEMA_ERROR("No shader available.")
		
		glPolygonMode(GL_FRONT_AND_BACK, static_cast<GLenum>(m_polygon_mode));
		
		m_target->make_current(true);
		
		m_shader->bind();
		
		drawable.draw(*this);
		
		m_shader->unbind();
	}
}