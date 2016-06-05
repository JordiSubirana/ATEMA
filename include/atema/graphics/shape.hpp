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

#ifndef ATEMA_GRAPHICS_SHAPE_HEADER
#define ATEMA_GRAPHICS_SHAPE_HEADER

#include <atema/graphics/config.hpp>
#include <atema/graphics/mesh.hpp>

namespace at
{
	class ATEMA_GRAPHICS_API Shape
	{
		public:
			Shape() = delete;
			~Shape() = delete;
			
			static Mesh create_triangle_mesh(const Vector3f& p1, const Vector3f& p2, const Vector3f& p3);
			static Buffer<Vector2f> create_triangle_texture_coordinates(const Vector2f& p1, const Vector2f& p2, const Vector2f& p3);
			
			static Mesh create_grid_mesh(size_t div_x, size_t div_y, const Vector3f& p1 = Vector3f(-1.0f, 1.0f, 0.0f), const Vector3f& p2 = Vector3f(-1.0f, -1.0f, 0.0f), const Vector3f& p3 = Vector3f(1.0f, 1.0f, 0.0f));
			static Buffer<Vector2f> create_grid_texture_coordinates(size_t div_x, size_t div_y, const Vector2f& p1 = Vector2f(0.0f, 0.0f), const Vector2f& p2 = Vector2f(0.0f, 1.0f), const Vector2f& p3 = Vector2f(1.0f, 0.0f));
	};
}

#endif