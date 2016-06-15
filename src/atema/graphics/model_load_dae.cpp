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

#include <tinyxml2/tinyxml2.h>

#include <map>
#include <vector>
#include <cstdlib>
#include <string>

#define ATEMA_XML_FOR(at_variable, at_parent, at_name) \
	for (tinyxml2::XMLElement* at_variable = at_parent->FirstChildElement(at_name); at_variable; at_variable = at_variable->NextSiblingElement(at_name))

namespace at
{
	void Model::load_dae(const char *filename)
	{
		struct DaeSource
		{
			unsigned int stride;
			std::vector<float> data;
		};
		
		struct DaeVertices
		{
			DaeSource *positions;
			DaeSource *normals;
			DaeSource *texcoords;
			
			DaeVertices()
			{
				positions = nullptr;
				normals = nullptr;
				texcoords = nullptr;
			}
		};
		
		std::map< std::string, DaeSource > dae_sources;
		
		tinyxml2::XMLDocument document;
		tinyxml2::XMLElement *parent = nullptr;
		tinyxml2::XMLElement *tmp = nullptr;
		
		if (document.LoadFile(filename) != tinyxml2::XML_SUCCESS)
			ATEMA_ERROR("File loading failed.")
		
		parent = document.FirstChildElement("COLLADA")->FirstChildElement("library_geometries");
		
		//<geometry>
		//parents : <library_geometries>
		//attributes :
		//  id (xs::ID)
		//  name (xs::token)
		//children :
		//  <asset> (0/1) -> unimplemented
		//  one of : (1)
		//    <convex_mesh> -> unimplemented
		//    <mesh>
		//    <spline> -> unimplemented
		//    <brep>
		//  <extra> (0+) -> unimplemented
		ATEMA_XML_FOR(geometry, parent, "geometry")
		{
			dae_sources.clear();
			
			//<mesh>
			//parents : <geometry>
			//children :
			//  <source> (1+)
			//  <vertices> (1)
			//  combinaison of : (each of the following elements is a at::Mesh + at::Material)
			//    <lines> (0+) -> unimplemented
			//    <linestrips> (0+) -> unimplemented
			//    <polygons> (0+) -> unimplemented
			//    <polylist> (0+) -> unimplemented
			//    <triangles> (0+)
			//    <trifans> (0+) -> unimplemented
			//    <tristrips> (0+) -> unimplemented
			//  <extra> (0+) -> unimplemented
			tmp = geometry->FirstChildElement("mesh"); //TODO: convex_mesh & spline
			
			//<source>
			//parents : <animation>, <mesh>, <morph>, <skin>, <spline>, <convex_mesh>, <brep>, <nurbs>, <nurbs_surface>
			//attributes :
			//  id (xs::ID)
			//  name (xs::token)
			//children :
			//  <asset> (0/1) -> unimplemented
			//  one of : (0/1)
			//    <bool_array> -> unimplemented
			//    <float_array>
			//    <IDREF_array> -> unimplemented
			//    <int_array>
			//    <Name_array> -> unimplemented
			//    <IDREF_array> -> unimplemented
			//    <token_array> -> unimplemented
			//  <technique_common> (0/1) -> unimplemented
			//  <technique> (0+) -> unimplemented
			ATEMA_XML_FOR(source, tmp, "source")
			{
				DaeSource dae_source;
				
				unsigned int count;
				unsigned int stride;
				const char *name = nullptr;
				
				tmp = source->FirstChildElement("technique_common")->FirstChildElement("accessor");
				
				count = tmp->UnsignedAttribute("count");
				stride = tmp->UnsignedAttribute("stride");
				
				if (!count || !stride)
					continue; //Bad data
				
				dae_source.data.resize(count*stride);
				
				//<float_array>
				//parents : <source>
				//attributes :
				//  count (uint_type) -> required
				//  id (xs::ID)
				//  name (xs::token)
				//  digits (xs::unsignedByte) -> 6
				//  magnitude (xs::short) -> 38
				tmp = source->FirstChildElement("float_array");
				
				if (tmp)
				{
					//Load float_arrays
					std::string float_data = tmp->GetText();
					std::string str_float;
					size_t tmp_index = 0;
					float value;
					int check_yz = 0;
					
					name = source->Attribute("id");
					
					if (!name || float_data.empty())
						continue;
					
					for (size_t i = 0; i < dae_source.data.size(); i++)
					{
						tmp_index = float_data.find_first_of(' ');
						
						str_float = float_data.substr(0, tmp_index);
						
						float_data = float_data.substr(tmp_index+1, float_data.size());
						
						value = atof(str_float.c_str());
						
						if (stride == 3)
						{
							//Inverse Y and Z axis
							if (check_yz == 2)
							{
								dae_source.data[i] = -dae_source.data[i-1];
								dae_source.data[i-1] = value;
								
								check_yz = 0;
							}
							else
							{
								dae_source.data[i] = value;
								
								check_yz++;
							}
						}
						else
						{
							dae_source.data[i] = value;
						}
					}
					
					dae_sources[std::string(name)] = dae_source;
				}
			} //sources
			
			tmp = geometry->FirstChildElement("mesh")->FirstChildElement("vertices");
			
			if (tmp)
			{
				DaeVertices vertices;
				
				ATEMA_XML_FOR(input, tmp, "input")
				{
					std::string semantic;
					std::string source;
					semantic = input->Attribute("semantic");
					source = input->Attribute("source");
					
					if (semantic.empty() || source.empty())
						continue;
					
					source = source.substr(1, source.size());
					
					auto it = dae_sources.find(source);
										
					if (it == dae_sources.end())
						continue;
					
					if (semantic.compare("POSITION") == 0)
						vertices.positions = &it->second;
					else if (semantic.compare("NORMAL") == 0)
						vertices.normals = &it->second;
					else if (semantic.compare("TEXCOORDS") == 0)
						vertices.texcoords = &it->second;
					else
						continue; //TODO: Other semantics
				}
				
				if (!vertices.positions)
					continue;
				
				tmp = geometry->FirstChildElement("mesh");
				
				//TODO: Other surfaces
				ATEMA_XML_FOR(surface, tmp, "triangles")
				{
					ModelElement element;
					
					std::vector<unsigned int> indices;
					unsigned int count;
					std::string indices_data;
					std::string str_int;
					size_t tmp_index = 0;
					
					//TODO: Check if the surface has "VERTEX" semantic
					tmp = surface->FirstChildElement("p");
					
					count = surface->UnsignedAttribute("count");
					
					indices_data = tmp->GetText();
					
					if (!tmp || !count || indices_data.empty())
						continue;
					
					indices.resize(count*3); //count triangles
					
					for (size_t i = 0; i < indices.size(); i++)
					{
						tmp_index = indices_data.find_first_of(' ');
						
						str_int = indices_data.substr(0, tmp_index);
						
						indices_data = indices_data.substr(tmp_index+1, indices_data.size());
						
						indices[i] = static_cast<unsigned int>(atoi(str_int.c_str()));
					}
					
					element.mesh = std::make_shared< Mesh >();
					element.mesh->create(Mesh::draw_mode::triangles, reinterpret_cast<Vector3f*>(vertices.positions->data.data()), vertices.positions->data.size()/3, indices.data(), indices.size());
					
					if (vertices.normals && (vertices.normals->data.size() == vertices.positions->data.size()))
					{
						element.normals = std::make_shared< Buffer<Vector3f> >();
						element.normals->create(reinterpret_cast<Vector3f*>(vertices.normals->data.data()), vertices.normals->data.size()/3);
					}
					
					if (vertices.texcoords && (vertices.texcoords->data.size() == vertices.positions->data.size()))
					{
						element.texcoords = std::make_shared< Buffer<Vector2f> >();
						element.texcoords->create(reinterpret_cast<Vector2f*>(vertices.texcoords->data.data()), vertices.texcoords->data.size()/2);
					}
					
					m_elements.push_back(element);
				}
			}
			
		} //geometries
		
		// parent = document.FirstChildElement("COLLADA")->FirstChildElement("library_geometries");
	}
}