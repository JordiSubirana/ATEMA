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

#ifndef ATEMA_GRAPHICS_BUFFER_ARRAY_HEADER
#define ATEMA_GRAPHICS_BUFFER_ARRAY_HEADER

#include <atema/context/opengl.hpp>
#include <atema/context/resource_gl.hpp>

#include <vector>

namespace at
{
	template <typename T>
	class Buffer : public ResourceGL
	{
		public:	
			enum class update_mode : GLenum
			{
				static_draw		= GL_STATIC_DRAW,
				static_read		= GL_STATIC_READ,
				static_copy		= GL_STATIC_COPY,
				dynamic_draw	= GL_DYNAMIC_DRAW,
				dynamic_read	= GL_DYNAMIC_READ,
				dynamic_copy	= GL_DYNAMIC_COPY,
				stream_draw		= GL_STREAM_DRAW,
				stream_read		= GL_STREAM_READ,
				stream_copy		= GL_STREAM_COPY
			};
			
		public:
			Buffer();
			Buffer(const T *elements, size_t elements_size, update_mode update_mode = update_mode::static_draw);
			Buffer(const Buffer<T>& array);
			virtual ~Buffer() noexcept;
			
			void create(const T *elements, size_t elements_size, update_mode update_mode = update_mode::static_draw);
			void create(const Buffer<T>& array);
			T* createVRAM_map(size_t elements_size, update_mode update_mode = update_mode::static_draw);

			void unmap() const;

			T* get() noexcept;
			const T* get() const noexcept;
			
			T& operator[](size_t index);
			const T& operator[](size_t index) const;
			
			size_t get_size() const;
			
			update_mode get_update_mode() const;
			
			//ObjectGL
			GLuint get_gl_id() const noexcept;
			bool is_valid() const noexcept;
			
			//ResourceGL
			void to_cpu();
			void to_gpu();
			
		protected:
			GLenum m_internal_type;
			
		private:
			void ensure_buffer();
			
			GLuint m_vbo;
			std::vector<T> m_elements;
			update_mode m_update_mode;
			
			bool m_buffer_ok;
			bool m_filled;
	};
}

#include "buffer.tpp"

#endif