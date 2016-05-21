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

#include <atema/graphics/mesh.hpp>
#include <atema/graphics/renderer.hpp>
#include <iostream>

namespace at
{
	Mesh::Mesh() :
		m_draw_mode(Mesh::draw_mode::points)
	{
		
	}
	
	Mesh::Mesh(draw_mode mesh_draw_mode, const Vector3f *elements, size_t elements_size, unsigned int *indices, size_t indices_size, typename BufferArray<Vector3f>::update_mode elements_update_mode, IndexArray::update_mode indices_update_mode) :
		Mesh()
	{
		create(mesh_draw_mode, elements, elements_size, indices, indices_size, elements_update_mode, indices_update_mode);
	}
	
	Mesh::Mesh(draw_mode mesh_draw_mode, const BufferArray<Vector3f>& elements, const IndexArray& indices) :
		Mesh()
	{
		create(mesh_draw_mode, elements, indices);
	}
	
	Mesh::Mesh(draw_mode mesh_draw_mode, const IndexedArray<Vector3f>& indexed_array) :
		Mesh(mesh_draw_mode, indexed_array.elements, indexed_array.indices)
	{
		
	}
	
	Mesh::Mesh(const Mesh& mesh) :
		Mesh(mesh.m_draw_mode, static_cast<const IndexedArray<Vector3f>&>(mesh))
	{
		
	}
	
	Mesh::~Mesh() noexcept
	{
		
	}
	
	void Mesh::create(draw_mode mesh_draw_mode, const Vector3f *elements, size_t elements_size, unsigned int *indices, size_t indices_size, typename BufferArray<Vector3f>::update_mode elements_update_mode, IndexArray::update_mode indices_update_mode)
	{
		m_draw_mode = mesh_draw_mode;
		IndexedArray<Vector3f>::create(elements, elements_size, indices, indices_size, elements_update_mode, indices_update_mode);
	}
	
	void Mesh::create(draw_mode mesh_draw_mode, const BufferArray<Vector3f>& elements, const IndexArray& indices)
	{
		m_draw_mode = mesh_draw_mode;
		IndexedArray<Vector3f>::create(elements, indices);
	}
	
	void Mesh::create(draw_mode mesh_draw_mode, const IndexedArray<Vector3f>& indexed_array)
	{
		create(mesh_draw_mode, indexed_array.elements, indexed_array.indices);
	}
	
	void Mesh::create(const Mesh& mesh)
	{
		create(mesh.m_draw_mode, static_cast<const IndexedArray<Vector3f>&>(mesh));
	}
	
	void Mesh::set_draw_mode(draw_mode mesh_draw_mode) noexcept
	{
		m_draw_mode = mesh_draw_mode;
	}
	
	Mesh::draw_mode Mesh::get_draw_mode() const noexcept
	{
		return (m_draw_mode);
	}
	
	bool Mesh::is_valid() const noexcept
	{
		return (elements.is_valid());
	}
	
	//PRIVATE
	bool Mesh::ensure_indices()
	{
		if (indices.is_valid())
			return (true);
		
		try
		{
			indices.generate(elements);
			
			return (true);
		}
		catch (const std::exception& e)
		{
			return (false);
		}
		
		return (true);
	}
	
	void Mesh::draw(const Renderer& renderer)
	{
		GLint entry_location;
		
		if (!is_valid())
			return;
		
		entry_location = renderer.get_shader()->get_gl_entry_location();
		
		glBindVertexArray(renderer.get_shader()->get_gl_vao_id());
		
		if (indices.is_valid())
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices.get_gl_id());
		glBindBuffer(GL_ARRAY_BUFFER, elements.get_gl_id());
		
		glEnableVertexAttribArray(entry_location);
		glVertexAttribPointer(entry_location, 3, GL_FLOAT, GL_FALSE, 0, 0);
		
		if (indices.is_valid())
			glDrawElements(static_cast<GLenum>(m_draw_mode), indices.get_size(), GL_UNSIGNED_INT, 0);
		else
			glDrawArrays(static_cast<GLenum>(m_draw_mode), 0, elements.get_size());
		
		glBindVertexArray(0);
		
		glDisableVertexAttribArray(entry_location);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}