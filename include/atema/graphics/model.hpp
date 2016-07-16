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

#ifndef ATEMA_GRAPHICS_MODEL_HEADER
#define ATEMA_GRAPHICS_MODEL_HEADER

#include <atema/graphics/config.hpp>
#include <atema/graphics/mesh.hpp>
#include <atema/graphics/material.hpp>
#include <atema/graphics/drawable.hpp>

#include <vector>
#include <memory>

namespace at
{
	class Model;
	
	template <typename T>
	class ShaderVariable;
	
	class ATEMA_GRAPHICS_API Model : public Drawable
	{
		friend class at::ShaderVariable<Model>;
		
		public:
			Model();
			virtual ~Model();
			
			void create(const char *filename);
			
			bool is_valid() const noexcept;
			
		private:
			void load_obj(const char *filename);
			void load_dae(const char *filename);
			
			void draw(Renderer& renderer);
			
			bool m_valid;
			
			struct ModelElement
			{
				std::shared_ptr< Mesh > mesh;
				std::shared_ptr< Buffer<Vector3f> > normals;
				std::shared_ptr< Buffer<Vector2f> > texcoords;
				std::shared_ptr< Material > material;
			};
			
			std::vector< ModelElement > m_elements;
			
			size_t m_current_element;
	};
}

#endif