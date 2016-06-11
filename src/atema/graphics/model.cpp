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

#include <atema/graphics/model.hpp>
#include <atema/core/error.hpp>
#include <atema/core/file.hpp>

#include <string>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace at
{
	//PUBLIC
	Model::Model()
	{
		
	}
	
	Model::~Model()
	{
		
	}
	
	Mesh& Model::get_mesh() noexcept
	{
		return (m_mesh);
	}
	
	void Model::create(const char *filename)
	{
		bool valid_tmp = m_valid;
		
		if (!filename)
			ATEMA_ERROR("No filename given.")
		
		m_valid = false;
		
		if (File::extension_match(filename, "obj"))
		{
			load_obj(filename);
		}
		else
		{
			m_valid = valid_tmp;
			ATEMA_ERROR("Invalid filename or extension.")
		}
	}
	
	bool Model::is_valid() const noexcept
	{
		return (m_valid);
	}
	
	//PRIVATE
	void Model::load_obj(const char *filename)
	{
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		
		std::string directory;

		std::string err;
		bool ret = tinyobj::LoadObj(shapes, materials, err, filename);
		
		std::vector<MeshElement>& mesh_elements = m_mesh.m_elements;
		
		if (!err.empty())
		{
			//Warnings
		}
		
		if (!ret)
			ATEMA_ERROR("Error when loading OBJ file.")
		
		mesh_elements.resize(shapes.size());
		
		for (size_t i = 0; i < mesh_elements.size(); i++)
		{
			//NAME shapes[i].name.c_str()
			
			if ((shapes[i].mesh.positions.size() == 0) || (shapes[i].mesh.positions.size()%3 != 0))
				ATEMA_ERROR("Invalid vertices size.")
			
			mesh_elements[i].create(MeshElement::draw_mode::triangles, reinterpret_cast<Vector3f*>(shapes[i].mesh.positions.data()), shapes[i].mesh.positions.size()/3);
			
			if (shapes[i].mesh.indices.size() > 0)
				mesh_elements[i].indices.create(shapes[i].mesh.indices.data(), shapes[i].mesh.indices.size());				
			
			//MATERIAL IDs shapes[i].mesh.material_ids.size()
		}
		
		m_materials.resize(materials.size());
		
		directory = File::get_path(filename);
		
		for (size_t i = 0; i < m_materials.size(); i++)
		{
			//NAME materials[i].name.c_str()
			
			m_materials[i].ambient_color = Color(materials[i].ambient[0], materials[i].ambient[1], materials[i].ambient[2], 1.0f); //Ka
			m_materials[i].diffuse_color = Color(materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2], 1.0f); //Kd
			m_materials[i].specular_color = Color(materials[i].specular[0], materials[i].specular[1], materials[i].specular[2], 1.0f); //Ks
			// m_materials[i].transmittance_color = Color(materials[i].transmittance[0], materials[i].transmittance[1], materials[i].transmittance[2], 1.0f); //Tr
			// m_materials[i].emission_color = Color(materials[i].emission[0], materials[i].emission[1], materials[i].emission[2], 1.0f); //Ke
			
			m_materials[i].shininess = materials[i].shininess; //Ns
			// m_materials[i].ior = materials[i].ior; //Ni
			// m_materials[i].dissolve = materials[i].dissolve; //dissolve
			// m_materials[i].illum = materials[i].illum; //illum
			
			if (!materials[i].ambient_texname.empty())
				m_materials[i].ambient_texture.create((directory + materials[i].ambient_texname).c_str()); //map_Ka
			if (!materials[i].diffuse_texname.empty())
				m_materials[i].diffuse_texture.create((directory + materials[i].diffuse_texname).c_str()); //map_Kd
			if (!materials[i].specular_texname.empty())
				m_materials[i].specular_texture.create((directory + materials[i].specular_texname).c_str()); //map_Ks
			// m_materials[i].specular_highlight_texture.create(materials[i].specular_highlight_texname.c_str()); //map_Ns
		}
		
		m_valid = true;
	}
}