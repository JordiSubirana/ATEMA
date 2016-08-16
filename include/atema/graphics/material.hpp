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

#ifndef ATEMA_GRAPHICS_MATERIAL_HEADER
#define ATEMA_GRAPHICS_MATERIAL_HEADER

#include <atema/graphics/config.hpp>
#include <atema/graphics/color.hpp>
#include <atema/graphics/texture.hpp>

namespace at
{
	class ATEMA_GRAPHICS_API Material
	{
		public:
			Material();
			virtual ~Material();
			
			Color ambient_color;
			Color diffuse_color;
			Color specular_color;
			
			float shininess;
			
			Texture ambient_texture;
			Texture diffuse_texture;
			Texture specular_texture;
	};
}

#endif