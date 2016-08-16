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
#include <vector>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace at
{
	void Model::load_obj(const char *filename)
	{
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		
		std::string directory;

		std::string err;
		bool ret = tinyobj::LoadObj(shapes, materials, err, filename);
		
		if (!err.empty())
		{
			//Warnings
		}
		
		if (!ret)
			ATEMA_ERROR("Error when loading OBJ file.")
		
		m_elements.resize(shapes.size());
		
		for (size_t i = 0; i < m_elements.size(); i++)
		{
			//NAME shapes[i].name.c_str()
			auto& mesh = shapes[i].mesh;
			auto& positions = mesh.positions;
			auto& indices = mesh.indices;
			auto& normals = mesh.normals;
			auto& texcoords = mesh.texcoords;
			size_t mat_id = 0;
			
			if ((positions.size() == 0) || (positions.size()%3 != 0))
				ATEMA_ERROR("Invalid vertices size.")
			
			m_elements[i].mesh = std::make_shared< Mesh >();			
			m_elements[i].mesh->create(Mesh::draw_mode::triangles, reinterpret_cast< Vector3f* >(positions.data()), positions.size()/3);
			
			if (indices.size() > 0)
				m_elements[i].mesh->indices.create(indices.data(), indices.size());
			
			if ( (normals.size()) && (normals.size() == positions.size()) )
			{
				m_elements[i].normals = std::make_shared< Buffer<Vector3f> >();
				m_elements[i].normals->create(reinterpret_cast<Vector3f*>(normals.data()), normals.size()/3);
			}
			
			if ( (texcoords.size()) && (texcoords.size()/2 == positions.size()/3) )
			{
				m_elements[i].texcoords = std::make_shared< Buffer<Vector2f> >();
				m_elements[i].texcoords->create(reinterpret_cast<Vector2f*>(texcoords.data()), texcoords.size()/2);
			}
			
			if (mesh.material_ids.empty())
				continue;
			
			mat_id = mesh.material_ids[0];
			
			if (materials.size() <= mat_id)
				continue;
			
			directory = File::get_path(filename);
			
			//NAME materials[i].name.c_str()
			
			m_elements[i].material = std::make_shared< Material >();
			
			m_elements[i].material->ambient_color = Color(materials[mat_id].ambient[0], materials[mat_id].ambient[1], materials[mat_id].ambient[2], 1.0f); //Ka
			m_elements[i].material->diffuse_color = Color(materials[mat_id].diffuse[0], materials[mat_id].diffuse[1], materials[mat_id].diffuse[2], 1.0f); //Kd
			m_elements[i].material->specular_color = Color(materials[mat_id].specular[0], materials[mat_id].specular[1], materials[mat_id].specular[2], 1.0f); //Ks
			// m_elements[i].material->transmittance_color = Color(materials[mat_id].transmittance[0], materials[mat_id].transmittance[1], materials[mat_id].transmittance[2], 1.0f); //Tr
			// m_elements[i].material->emission_color = Color(materials[mat_id].emission[0], materials[mat_id].emission[1], materials[mat_id].emission[2], 1.0f); //Ke
			
			m_elements[i].material->shininess = materials[mat_id].shininess; //Ns
			// m_elements[i].material->ior = materials[mat_id].ior; //Ni
			// m_elements[i].material->dissolve = materials[mat_id].dissolve; //dissolve
			// m_elements[i].material->illum = materials[mat_id].illum; //illum
			
			if (!materials[mat_id].ambient_texname.empty())
			{
				m_elements[i].material->ambient_texture.create((directory + materials[mat_id].ambient_texname).c_str()); //map_Ka
			}
			if (!materials[mat_id].diffuse_texname.empty())
			{
				m_elements[i].material->diffuse_texture.create((directory + materials[mat_id].diffuse_texname).c_str()); //map_Kd
			}
			if (!materials[mat_id].specular_texname.empty())
			{
				m_elements[i].material->specular_texture.create((directory + materials[mat_id].specular_texname).c_str()); //map_Ks
			}
			// m_elements[i].material->specular_highlight_texture.create(materials[mat_id].specular_highlight_texname.c_str()); //map_Ns
		}
		
		m_valid = true;
	}
}