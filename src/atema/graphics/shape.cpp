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
#include <atema/math/tools.hpp>
#include <atema/core/error.hpp>

#include <vector>

namespace at
{
	//TRIANGLE
	MeshElement Shape::create_triangle_mesh(const Vector3f& p1, const Vector3f& p2, const Vector3f& p3)
	{
		MeshElement mesh;
		std::vector<Vector3f> points;
		
		points.resize(3);
		
		points[0] = p1;
		points[1] = p2;
		points[2] = p3;
		
		mesh.create(MeshElement::draw_mode::triangles, points.data(), points.size());
		
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
	MeshElement Shape::create_grid_mesh(size_t div_x, size_t div_y, const Vector3f& p1, const Vector3f& p2, const Vector3f& p3)
	{
		MeshElement mesh;
		std::vector<Vector3f> points;
		std::vector<unsigned int> indices;
		
		size_t points_x = div_x+1;
		size_t points_y = div_y+1;
		
		Vector3f dx;
		Vector3f dy;
		
		size_t index;
		
		if (!div_x || !div_y)
			ATEMA_ERROR("Axis divisions must be non zero.")
		
		points.resize(points_x * points_y);
		indices.resize(3*(2*(div_x*div_y)));
		
		dx = (p3-p1)/static_cast<float>(div_x);
		dy = (p2-p1)/static_cast<float>(div_y);
		
		index = 0;
		
		for (size_t y = 0; y < points_y; y++)
		{
			for (size_t x = 0; x < points_x; x++)
			{
				points[index] = p1 + dx*static_cast<float>(x) + dy*static_cast<float>(y);
				index++;
			}
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
		
		mesh.create(MeshElement::draw_mode::triangles, points.data(), points.size(), indices.data(), indices.size());
		
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
	
	//SPHERE
	MeshElement Shape::create_sphere_mesh(float radius, size_t nb_stacks, size_t nb_slices, const Vector3f& origin)
	{
		MeshElement mesh;
		std::vector<Vector3f> points;
		std::vector<unsigned int> indices;
		
		size_t index;
		size_t index_max;
		
		float dslice = ATEMA_DEG_TO_RAD( 360.0f/nb_slices );
		float dstack = ATEMA_DEG_TO_RAD( 180.0f/(nb_stacks-1) );
		
		const float rad_pi = static_cast<float>(ATEMA_PI);
		const float rad_2pi = 2*rad_pi;
		
		float u;
		float t;
		
		points.resize(2 + nb_slices*(nb_stacks-2));
		indices.resize(2*3*nb_slices*(nb_stacks-2));
		
		//POINTS
		//Sphere top
		points[0] = origin;
		points[0].y += radius;
		
		//Sphere middle
		index = 1;
		u = 0.0f;
		t = dstack;
		for (size_t y = 0; y < nb_stacks-2; y++)
		{
			u = 0.0f;
			
			for (size_t x = 0; x < nb_slices; x++)
			{
				points[index] = origin;
				
				points[index].x += static_cast<float>(radius * std::sin(t) * std::cos(u));
				points[index].y += static_cast<float>(radius * std::cos(t));
				points[index].z += static_cast<float>(radius * std::sin(t) * std::sin(u));
				
				u += dslice;
				
				index++;
			}
			
			t += dstack;
		}
		
		//Sphere down
		points[points.size()-1] = origin;
		points[points.size()-1].y -= radius;
		
		//INDICES
		//Sphere top
		index = 1;
		index_max = nb_slices*3;
		for (size_t i = 0; i < index_max; i += 3)
		{
			indices[i+0] = 0; //Common point : sphere top
			indices[i+1] = index++;
			if (i < index_max-3)
				indices[i+2] = index;
			else //Closing the loop !
				indices[i+2] = index - nb_slices;
		}
		
		//Shere middle : offset of 1 because of the top point
		index = index_max;
		for (size_t y = 0; y < nb_stacks-3; y++)
		{
			for (size_t x = 0; x < nb_slices; x++)
			{
				if (x < nb_slices-1)
				{
					indices[index++] = static_cast<unsigned int>(1 + x		+ y*nb_slices);
					indices[index++] = static_cast<unsigned int>(1 + x		+(y+1)*nb_slices);
					indices[index++] = static_cast<unsigned int>(1 + x+1	+ y*nb_slices); //Change 1
					indices[index++] = static_cast<unsigned int>(1 + x+1	+ y*nb_slices); //Change 2
					indices[index++] = static_cast<unsigned int>(1 + x		+(y+1)*nb_slices);
					indices[index++] = static_cast<unsigned int>(1 + x+1	+(y+1)*nb_slices); //Change 3
				}
				else //Closing the loop !
				{
					indices[index++] = static_cast<unsigned int>(1 + x		+ y*nb_slices);
					indices[index++] = static_cast<unsigned int>(1 + x		+(y+1)*nb_slices);
					indices[index++] = static_cast<unsigned int>(1 + x+1	+(y-1)*nb_slices); //Change 1
					indices[index++] = static_cast<unsigned int>(1 + x+1	+(y-1)*nb_slices); //Change 2
					indices[index++] = static_cast<unsigned int>(1 + x		+(y+1)*nb_slices);
					indices[index++] = static_cast<unsigned int>(1 + x+1	+(y+0)*nb_slices); //Change 3
				}
			}
		}
		
		//Sphere down
		index_max = indices.size();
		index = points.size() - 1 - nb_slices;
		for (size_t i = index_max - (nb_slices*3); i < index_max; i += 3)
		{
			indices[i+0] = index++;
			indices[i+1] = points.size()-1; //Common point : sphere down
			if (i < index_max-3)
				indices[i+2] = index;
			else //Closing the loop !
				indices[i+2] = index - nb_slices;
		}
		
		mesh.create(MeshElement::draw_mode::triangles, points.data(), points.size(), indices.data(), indices.size());
		
		return (mesh);
	}
}