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
#include <atema/graphics/renderer.hpp>
#include <atema/core/error.hpp>
#include <atema/core/file.hpp>

#include <string>

namespace at
{
	//PUBLIC
	Model::Model() :
		m_current_element(0)
	{
		
	}
	
	Model::~Model()
	{
		
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
		else if (File::extension_match(filename, "dae"))
		{
			load_dae(filename);
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
	void Model::draw(Renderer& renderer)
	{
		Shader *shader = renderer.get_shader();
		std::string entry;
		std::string at_entry;
		
		m_current_element = 0;
		
		if (!shader)
			ATEMA_ERROR("No existing shader in renderer parameters.")
		
		entry = shader->get_entry_name();
		
		at_entry = "at_";
		at_entry += entry;
		
		for (auto& element : m_elements)
		{
			try
			{
				if (element.normals)
					shader->set_varying((at_entry + "_normal").c_str(), *element.normals);
			}
			catch (...)
			{
				//Variable not used in the shader
			}
			
			try
			{
				if (element.texcoords)
					shader->set_varying((at_entry + "_tex_coords").c_str(), *element.texcoords);
			}
			catch (...)
			{
				//Variable not used in the shader
			}
			
			try
			{
				if (element.material)
				{
					// shader->set_uniform((entry + "_material").c_str(), *element.material);
					// shader->set_uniform("model_material", *element.material);
					shader->set_uniform(entry.c_str(), *element.material);
				}
			}
			catch (...)
			{
				//Variable not used in the shader
			}
			
			element.mesh->draw(renderer, shader->get_varying_location((at_entry + "_position").c_str()));
			m_current_element++;
		}
	}
}