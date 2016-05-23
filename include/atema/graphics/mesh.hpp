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

#ifndef ATEMA_GRAPHICS_MESH_HEADER
#define ATEMA_GRAPHICS_MESH_HEADER

#include <atema/graphics/config.hpp>
#include <atema/graphics/indexed_array.hpp>
#include <atema/math/vector.hpp>
#include <atema/context/opengl.hpp>
#include <atema/graphics/drawable.hpp>

namespace at
{
	class Renderer;
	
	class ATEMA_GRAPHICS_API Mesh : public IndexedArray<Vector3f>, public Drawable
	{
		public:
			using IndexedArray<Vector3f>::create;
			
		public:
			enum class draw_mode : GLenum
			{
				points = GL_POINTS,
				line_strip = GL_LINE_STRIP,
				line_loop = GL_LINE_LOOP,
				lines = GL_LINES,
				line_strip_adjacency = GL_LINE_STRIP_ADJACENCY,
				lines_adjacency = GL_LINES_ADJACENCY,
				triangle_strip = GL_TRIANGLE_STRIP,
				triangle_fan = GL_TRIANGLE_FAN,
				triangles = GL_TRIANGLES,
				triangle_strip_adjacency = GL_TRIANGLE_STRIP_ADJACENCY,
				triangles_adjacency = GL_TRIANGLES_ADJACENCY,
				patches = GL_PATCHES
			};
			
		public:
			Mesh();
			Mesh(draw_mode mesh_draw_mode, const Vector3f *elements, size_t elements_size, unsigned int *indices = nullptr, size_t indices_size = 0, typename Buffer<Vector3f>::update_mode elements_update_mode = Buffer<Vector3f>::update_mode::static_draw, IndexArray::update_mode indices_update_mode = IndexArray::update_mode::static_draw);
			Mesh(draw_mode mesh_draw_mode, const Buffer<Vector3f>& elements, const IndexArray& indices);
			Mesh(draw_mode mesh_draw_mode, const IndexedArray<Vector3f>& indexed_array);
			Mesh(const Mesh& mesh);
			virtual ~Mesh() noexcept;
			
			void create(draw_mode mesh_draw_mode, const Vector3f *elements, size_t elements_size, unsigned int *indices = nullptr, size_t indices_size = 0, typename Buffer<Vector3f>::update_mode elements_update_mode = Buffer<Vector3f>::update_mode::static_draw, IndexArray::update_mode indices_update_mode = IndexArray::update_mode::static_draw);
			void create(draw_mode mesh_draw_mode, const Buffer<Vector3f>& elements, const IndexArray& indices);
			void create(draw_mode mesh_draw_mode, const IndexedArray<Vector3f>& indexed_array);
			void create(const Mesh& mesh);
			
			void set_draw_mode(draw_mode mesh_draw_mode) noexcept;
			draw_mode get_draw_mode() const noexcept;
			
			//ObjectGL
			bool is_valid() const noexcept;
			
		private:
			bool ensure_indices();
			void draw(const Renderer& renderer);
			
			draw_mode m_draw_mode;
	};
}

#endif