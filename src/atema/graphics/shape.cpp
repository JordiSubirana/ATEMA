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

#include <atema/graphics/shape.hpp>
#include <atema/core/error.hpp>

#include <vector>

namespace at
{
	//TRIANGLE
	Mesh Shape::create_triangle_mesh(const Vector3f& p1, const Vector3f& p2, const Vector3f& p3)
	{
		Mesh mesh;
		std::vector<Vector3f> points;
		
		points.resize(3);
		
		points[0] = p1;
		points[1] = p2;
		points[2] = p3;
		
		mesh.create(Mesh::draw_mode::triangles, points.data(), points.size());
		
		return (mesh);
	}
	
	Buffer<Vector2f> Shape::create_triangle_texture_coordinates(const Vector2f& p1, const Vector2f& p2, const Vector2f& p3)
	{
		Buffer<Vector2f> array;
		std::vector<Vector2f> points;
		
		points.resize(3);
		
		points[0] = p1;
		points[1] = p2;
		points[2] = p3;
		
		array.create(points.data(), points.size());
		
		return (array);
	}
	
	//GRID
	Mesh Shape::create_grid_mesh(size_t div_x, size_t div_y, const Vector3f& p1, const Vector3f& p2, const Vector3f& p3)
	{
		Mesh mesh;
		std::vector<Vector3f> points;
		std::vector<unsigned int> indices;
		
		size_t points_x = div_x+1;
		size_t points_y = div_y+1;
		
		Vector3f dx;
		Vector3f dy;
		
		Vector3f tmp;
		
		size_t index;
		
		if (!div_x || !div_y)
			ATEMA_ERROR("Axis divisions must be non zero.")
		
		points.resize(points_x * points_y);
		indices.resize(3*(2*(div_x*div_y)));
		
		dx = (p3-p1)/static_cast<float>(div_x);
		dy = (p2-p1)/static_cast<float>(div_y);
		
		tmp = p1;
		
		index = 0;
		
		for (size_t y = 0; y < points_y; y++)
		{
			for (size_t x = 0; x < points_x; x++)
			{
				points[index] = tmp;
				index++;
				
				tmp += dx;
			}
			
			tmp -= (dx*static_cast<float>(points_x));
			tmp += dy;
		}
		
		index = 0;
		
		for (size_t y = 0; y < div_y; y++)
		{
			for (size_t x = 0; x < div_x; x++)
			{
				indices[index++] = static_cast<unsigned int>(x		+ y*points_x);
				indices[index++] = static_cast<unsigned int>(x		+(y+1)*points_x);
				indices[index++] = static_cast<unsigned int>(x+1	+ y*points_x);
				indices[index++] = static_cast<unsigned int>(x+1	+ y*points_x);
				indices[index++] = static_cast<unsigned int>(x		+(y+1)*points_x);
				indices[index++] = static_cast<unsigned int>(x+1	+(y+1)*points_x);
			}
		}
		
		mesh.create(Mesh::draw_mode::triangles, points.data(), points.size(), indices.data(), indices.size());
		
		return (mesh);
	}
	
	Buffer<Vector2f> Shape::create_grid_texture_coordinates(size_t div_x, size_t div_y, const Vector2f& p1, const Vector2f& p2, const Vector2f& p3)
	{
		Buffer<Vector2f> array;
		std::vector<Vector2f> points;
		
		size_t points_x = div_x+1;
		size_t points_y = div_y+1;
		
		Vector2f dx;
		Vector2f dy;
		
		Vector2f tmp;
		
		size_t index;
		
		if (!div_x || !div_y)
			ATEMA_ERROR("Axis divisions must be non zero.")
		
		points.resize(points_x * points_y);
		
		dx = (p3-p1)/static_cast<float>(div_x);
		dy = (p2-p1)/static_cast<float>(div_y);
		
		tmp = p1;
		
		index = 0;
		
		for (size_t y = 0; y < points_y; y++)
		{
			for (size_t x = 0; x < points_x; x++)
			{
				points[index] = tmp;
				index++;
				
				tmp += dx;
			}
			
			tmp -= (dx*static_cast<float>(points_x));
			tmp += dy;
		}
		
		array.create(points.data(), points.size());
		
		return (array);
	}
}