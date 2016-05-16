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

#ifndef ATEMA_GRAPHICS_ARRAY_HEADER
#define ATEMA_GRAPHICS_ARRAY_HEADER

#include <atema/context/opengl.hpp>
#include <atema/graphics/shared_object.hpp>

#include <vector>

namespace at
{
	template <typename T>
	class Array : public SharedObject
	{
		public:	
			enum class update : GLenum
			{
				static_mode		= GL_STATIC_DRAW,
				dynamic_mode	= GL_DYNAMIC_DRAW,
				stream_mode		= GL_STREAM_DRAW
			};
			
			using SharedObject::download;
			using SharedObject::upload;
			
		public:
			Array();
			Array(const T *elements, size_t elements_size, update update_mode = update::static_mode);
			Array(const Array<T>& array);
			virtual ~Array() noexcept;
			
			void create(const T *elements, size_t elements_size, update update_mode = update::static_mode);
			void create(const Array<T>& array);
			
			T* get() noexcept;
			const T* get() const noexcept;
			
			T& operator[](size_t index);
			const T& operator[](size_t index) const;
			
			size_t get_size() const;
			
			update get_update_mode() const;
			
			GLuint get_gl_id() const;
			
			void download();
			void upload();
			
		protected:
			virtual GLenum get_buffer_type() const noexcept;
			GLenum get_internal_type() const noexcept;
			
		private:
			using data = struct arr_data
			{
				GLuint vbo;		
				std::vector<T> elements;		
				size_t elements_size;		
				update update_mode;
				
				arr_data() :
					vbo(0),
					elements(),
					elements_size(0),
					update_mode(update::static_mode)
				{
					
				}
				
				~arr_data() noexcept
				{
					free();
				};
				
				void free() noexcept
				{
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
					glBindBuffer(GL_ARRAY_BUFFER, 0);
					glDeleteBuffers(1, &vbo);
					
					elements.clear();
					elements.resize(0);
					
					elements_size = 0;
					
					vbo = 0;
				}
			};
			
			data m_data;
	};
}

#include "array.tpp"

#endif